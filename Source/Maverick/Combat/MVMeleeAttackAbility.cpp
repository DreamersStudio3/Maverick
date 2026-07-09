#include "Combat/MVMeleeAttackAbility.h"

#include "Character/MVCharacterBase.h"
#include "Collision/MVCollisionChannels.h"
#include "Combat/MVHitResolverSubsystem.h"
#include "Components/ActorComponent.h"
#include "Components/MeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Struct/MVHitTypes.h"

DEFINE_LOG_CATEGORY_STATIC(LogMVMeleeAttackAbility, Log, All);

UMVMeleeAttackAbility::UMVMeleeAttackAbility()
{
	TargetObjectChannels.Add(ECC_Pawn);
	TargetObjectChannels.Add(MVCollisionChannels::AttackTarget);
}

void UMVMeleeAttackAbility::SetWeaponTraceSockets(
	const FName StartSocketName,
	const FName EndSocketName)
{
	TraceStartSocketName = StartSocketName;
	TraceEndSocketName = EndSocketName;
}

void UMVMeleeAttackAbility::StartAbility_Implementation()
{
	const bool bWasAbilityActive = bAbilityActive;
	Super::StartAbility_Implementation();

	if (bWasAbilityActive || !bAbilityActive)
	{
		return;
	}

	BeginMeleeTrace();
}

void UMVMeleeAttackAbility::EndAbility_Implementation()
{
	StopMeleeTrace();
	Super::EndAbility_Implementation();
}

void UMVMeleeAttackAbility::BeginDestroy()
{
	StopMeleeTrace();
	Super::BeginDestroy();
}

void UMVMeleeAttackAbility::BeginMeleeTrace()
{
	UWorld* World = GetAbilityWorld();
	if (!World)
	{
		return;
	}

	PreviousSamples.Reset();
	HitVictimKeys.Reset();
	bLoggedMissingTraceSegments = false;

	TickMeleeTrace();

	const float EffectiveTraceInterval = FMath::Max(0.001f, TraceInterval);
	World->GetTimerManager().SetTimer(
		TraceTimerHandle,
		this,
		&UMVMeleeAttackAbility::TickMeleeTrace,
		EffectiveTraceInterval,
		true);
}

void UMVMeleeAttackAbility::StopMeleeTrace()
{
	if (UWorld* World = GetAbilityWorld())
	{
		World->GetTimerManager().ClearTimer(TraceTimerHandle);
	}

	PreviousSamples.Reset();
	HitVictimKeys.Reset();
}

void UMVMeleeAttackAbility::TickMeleeTrace()
{
	if (!bAbilityActive)
	{
		StopMeleeTrace();
		return;
	}

	AMVCharacterBase* OwnerCharacter = GetOwnerCharacter();
	if (!OwnerCharacter)
	{
		return;
	}

	TArray<UObject*> AbilityDataProviders;
	CollectAttackAbilityDataProviders(AbilityDataProviders);
	if (AbilityDataProviders.IsEmpty())
	{
		if (!bLoggedMissingTraceSegments)
		{
			UE_LOG(
				LogMVMeleeAttackAbility,
				Warning,
				TEXT("Melee attack trace has no provider. Owner=%s."),
				*GetNameSafe(OwnerCharacter));
			bLoggedMissingTraceSegments = true;
		}
		return;
	}

	bool bAnyTraceSegment = false;
	for (UObject* AbilityDataProvider : AbilityDataProviders)
	{
		if (!AbilityDataProvider)
		{
			continue;
		}

		TArray<FMVMeleeWeaponData> MeleeWeaponDataArray;
		if (!CollectMeleeWeaponDataFromProvider(AbilityDataProvider, MeleeWeaponDataArray))
		{
			continue;
		}

		for (const FMVMeleeWeaponData& MeleeWeaponData : MeleeWeaponDataArray)
		{
			if (MeleeWeaponData.bSecondaryWeapon && !bIncludeSecondaryWeapon)
			{
				continue;
			}

			FMVMeleeAttackTraceSample CurrentSample;
			CurrentSample.SourceComponent = MeleeWeaponData.WeaponMesh.Get();
			CurrentSample.StartLocation = MeleeWeaponData.StartLocation;
			CurrentSample.EndLocation = MeleeWeaponData.EndLocation;
			CurrentSample.bSecondaryWeapon = MeleeWeaponData.bSecondaryWeapon;
			TraceWeaponSegment(CurrentSample);
			bAnyTraceSegment = true;
		}
	}

	if (!bAnyTraceSegment && !bLoggedMissingTraceSegments)
	{
		UE_LOG(
			LogMVMeleeAttackAbility,
			Warning,
			TEXT("Melee attack trace providers returned no socket segments. Owner=%s StartSocket=%s EndSocket=%s."),
			*GetNameSafe(OwnerCharacter),
			*TraceStartSocketName.ToString(),
			*TraceEndSocketName.ToString());
		bLoggedMissingTraceSegments = true;
	}
}

void UMVMeleeAttackAbility::TraceWeaponSegment(
	const FMVMeleeAttackTraceSample& CurrentSample)
{
	const float CapsuleRadius = ResolveTraceCapsuleRadius();
	const FMVMeleeAttackTraceSample* PreviousSample = FindPreviousSample(CurrentSample.SourceComponent.Get());
	if (PreviousSample)
	{
		PerformSubstepSweep(*PreviousSample, CurrentSample, CapsuleRadius);
	}
	else
	{
		PerformInitialOverlap(CurrentSample, CapsuleRadius);
	}

	CachePreviousSample(CurrentSample);
}

void UMVMeleeAttackAbility::PerformInitialOverlap(
	const FMVMeleeAttackTraceSample& CurrentSample,
	const float CapsuleRadius)
{
	UWorld* World = GetAbilityWorld();
	AMVCharacterBase* Attacker = GetOwnerCharacter();
	if (!World || !Attacker)
	{
		return;
	}

	FMVMeleeAttackCapsule Capsule;
	if (!BuildTraceCapsule(CurrentSample, CapsuleRadius, Capsule))
	{
		return;
	}

	FCollisionObjectQueryParams ObjectQueryParams;
	BuildObjectQueryParams(ObjectQueryParams);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(MVMeleeAttackAbilityInitialOverlap), false, Attacker);
	QueryParams.AddIgnoredActor(Attacker);

	TArray<FOverlapResult> OverlapResults;
	const bool bHasOverlap = World->OverlapMultiByObjectType(
		OverlapResults,
		Capsule.Center,
		Capsule.Rotation,
		ObjectQueryParams,
		FCollisionShape::MakeCapsule(Capsule.Radius, Capsule.HalfHeight),
		QueryParams);

	DrawDebugTraceCapsule(Capsule, CurrentSample, bHasOverlap);

	if (!bHasOverlap)
	{
		return;
	}

	for (const FOverlapResult& OverlapResult : OverlapResults)
	{
		AActor* HitActor = OverlapResult.GetActor();
		if (!HitActor && OverlapResult.GetComponent())
		{
			HitActor = OverlapResult.GetComponent()->GetOwner();
		}
		ProcessHitActor(HitActor, Capsule.Center);
	}
}

void UMVMeleeAttackAbility::PerformSubstepSweep(
	const FMVMeleeAttackTraceSample& PreviousSample,
	const FMVMeleeAttackTraceSample& CurrentSample,
	const float CapsuleRadius)
{
	UWorld* World = GetAbilityWorld();
	AMVCharacterBase* Attacker = GetOwnerCharacter();
	if (!World || !Attacker)
	{
		return;
	}

	const int32 SubstepCount = ResolveSubstepCount(PreviousSample, CurrentSample);
	FMVMeleeAttackTraceSample LastStepSample = PreviousSample;
	for (int32 StepIndex = 1; StepIndex <= SubstepCount; ++StepIndex)
	{
		const float Alpha = static_cast<float>(StepIndex) / static_cast<float>(SubstepCount);

		FMVMeleeAttackTraceSample StepSample;
		StepSample.SourceComponent = CurrentSample.SourceComponent;
		StepSample.StartLocation = FMath::Lerp(PreviousSample.StartLocation, CurrentSample.StartLocation, Alpha);
		StepSample.EndLocation = FMath::Lerp(PreviousSample.EndLocation, CurrentSample.EndLocation, Alpha);
		StepSample.bSecondaryWeapon = CurrentSample.bSecondaryWeapon;

		FMVMeleeAttackCapsule PreviousCapsule;
		FMVMeleeAttackCapsule CurrentCapsule;
		if (!BuildTraceCapsule(LastStepSample, CapsuleRadius, PreviousCapsule)
			|| !BuildTraceCapsule(StepSample, CapsuleRadius, CurrentCapsule))
		{
			LastStepSample = StepSample;
			continue;
		}

		FCollisionObjectQueryParams ObjectQueryParams;
		BuildObjectQueryParams(ObjectQueryParams);

		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(MVMeleeAttackAbilitySubstepSweep), false, Attacker);
		QueryParams.AddIgnoredActor(Attacker);

		TArray<FHitResult> HitResults;
		bool bHasHit = false;
		if (PreviousCapsule.Center.Equals(CurrentCapsule.Center, 0.1f))
		{
			TArray<FOverlapResult> OverlapResults;
			bHasHit = World->OverlapMultiByObjectType(
				OverlapResults,
				CurrentCapsule.Center,
				CurrentCapsule.Rotation,
				ObjectQueryParams,
				FCollisionShape::MakeCapsule(CurrentCapsule.Radius, CurrentCapsule.HalfHeight),
				QueryParams);

			for (const FOverlapResult& OverlapResult : OverlapResults)
			{
				AActor* HitActor = OverlapResult.GetActor();
				if (!HitActor && OverlapResult.GetComponent())
				{
					HitActor = OverlapResult.GetComponent()->GetOwner();
				}
				ProcessHitActor(HitActor, CurrentCapsule.Center);
			}
		}
		else
		{
			bHasHit = World->SweepMultiByObjectType(
				HitResults,
				PreviousCapsule.Center,
				CurrentCapsule.Center,
				CurrentCapsule.Rotation,
				ObjectQueryParams,
				FCollisionShape::MakeCapsule(CurrentCapsule.Radius, CurrentCapsule.HalfHeight),
				QueryParams);

			for (const FHitResult& HitResult : HitResults)
			{
				FVector HitLocation = CurrentCapsule.Center;
				if (!HitResult.ImpactPoint.IsNearlyZero())
				{
					HitLocation = FVector(HitResult.ImpactPoint);
				}
				ProcessHitActor(HitResult.GetActor(), HitLocation);
			}
		}

		DrawDebugTraceCapsule(CurrentCapsule, StepSample, bHasHit, &PreviousCapsule.Center);
		LastStepSample = StepSample;
	}
}

void UMVMeleeAttackAbility::ProcessHitActor(AActor* HitActor, const FVector& HitLocation)
{
	AMVCharacterBase* Attacker = GetOwnerCharacter();
	AMVCharacterBase* Victim = ResolveHitCharacter(HitActor);
	if (!Attacker || !Victim || Victim == Attacker)
	{
		return;
	}

	const TObjectKey<AMVCharacterBase> VictimKey(Victim);
	if (bHitEachVictimOnce && HitVictimKeys.Contains(VictimKey))
	{
		return;
	}

	if (bHitEachVictimOnce)
	{
		HitVictimKeys.Add(VictimKey);
	}

	UMVHitResolverSubsystem* HitResolver = UMVHitResolverSubsystem::Get(Attacker);
	if (!HitResolver)
	{
		UE_LOG(
			LogMVMeleeAttackAbility,
			Warning,
			TEXT("Melee attack hit skipped because HitResolverSubsystem is unavailable. Attacker=%s Victim=%s."),
			*GetNameSafe(Attacker),
			*GetNameSafe(Victim));
		return;
	}

	FMVHitResolveRequest Request;
	Request.Attacker = Attacker;
	Request.Victim = Victim;
	Request.DamageMultiplier = AbilityData.DamageMultiplier;
	Request.GroggyDamageMultiplier = AbilityData.GroggyDamageMultiplier;
	Request.HitReactionType = HitReactionType;
	Request.HitLocation = HitLocation;
	Request.HitDirection = (Victim->GetActorLocation() - Attacker->GetActorLocation()).GetSafeNormal();

	FMVResolvedHitData ResolvedHitData;
	HitResolver->ResolveAttackHit(Request, ResolvedHitData);
}

void UMVMeleeAttackAbility::CachePreviousSample(const FMVMeleeAttackTraceSample& Sample)
{
	for (FMVMeleeAttackTraceSample& PreviousSample : PreviousSamples)
	{
		if (PreviousSample.SourceComponent.Get() == Sample.SourceComponent.Get())
		{
			PreviousSample = Sample;
			return;
		}
	}

	PreviousSamples.Add(Sample);
}

const UMVMeleeAttackAbility::FMVMeleeAttackTraceSample* UMVMeleeAttackAbility::FindPreviousSample(
	const UMeshComponent* SourceComponent) const
{
	if (!SourceComponent)
	{
		return nullptr;
	}

	for (const FMVMeleeAttackTraceSample& PreviousSample : PreviousSamples)
	{
		if (PreviousSample.SourceComponent.Get() == SourceComponent)
		{
			return &PreviousSample;
		}
	}

	return nullptr;
}

void UMVMeleeAttackAbility::BuildObjectQueryParams(
	FCollisionObjectQueryParams& OutObjectQueryParams) const
{
	OutObjectQueryParams = FCollisionObjectQueryParams();
	bool bIncludesAttackTargetChannel = false;
	auto AddMeleeAttackObjectChannel = [&OutObjectQueryParams, &bIncludesAttackTargetChannel](
		const ECollisionChannel Channel)
	{
		OutObjectQueryParams.AddObjectTypesToQuery(Channel);
		if (Channel == MVCollisionChannels::AttackTarget)
		{
			bIncludesAttackTargetChannel = true;
		}
	};

	if (TargetObjectChannels.IsEmpty())
	{
		AddMeleeAttackObjectChannel(ECC_Pawn);
		AddMeleeAttackObjectChannel(MVCollisionChannels::AttackTarget);
		return;
	}

	for (const TEnumAsByte<ECollisionChannel> Channel : TargetObjectChannels)
	{
		AddMeleeAttackObjectChannel(Channel.GetValue());
	}

	if (!bIncludesAttackTargetChannel)
	{
		AddMeleeAttackObjectChannel(MVCollisionChannels::AttackTarget);
	}
}

bool UMVMeleeAttackAbility::CollectMeleeWeaponDataFromProvider(
	UObject* AbilityDataProvider,
	TArray<FMVMeleeWeaponData>& OutMeleeWeaponData) const
{
	OutMeleeWeaponData.Reset();
	if (!AbilityDataProvider)
	{
		return false;
	}

	const bool bHasMeleeWeaponData = IMVAttackAbilityDataInterface::Execute_GetMeleeWeaponData(
		AbilityDataProvider,
		TraceStartSocketName,
		TraceEndSocketName,
		OutMeleeWeaponData);
	if (bHasMeleeWeaponData && !OutMeleeWeaponData.IsEmpty())
	{
		return true;
	}

	if (!bIncludeSecondaryWeapon)
	{
		return false;
	}

	return IMVAttackAbilityDataInterface::Execute_GetMeleeDualWeaponData(
		AbilityDataProvider,
		TraceStartSocketName,
		TraceEndSocketName,
		OutMeleeWeaponData)
		&& !OutMeleeWeaponData.IsEmpty();
}

void UMVMeleeAttackAbility::CollectAttackAbilityDataProviders(TArray<UObject*>& OutProviders)
{
	OutProviders.Reset();

	TSet<TObjectKey<UObject>> ProviderKeys;
	if (UObject* ExplicitProvider = ExplicitAttackAbilityDataProvider.GetObject())
	{
		AddAttackAbilityDataProvider(ExplicitProvider, OutProviders, ProviderKeys);
		if (!OutProviders.IsEmpty())
		{
			return;
		}
	}

	AMVCharacterBase* OwnerCharacter = GetOwnerCharacter();
	if (!OwnerCharacter)
	{
		return;
	}

	AddAttackAbilityDataProvider(OwnerCharacter, OutProviders, ProviderKeys);

	TArray<UActorComponent*> OwnerComponents;
	OwnerCharacter->GetComponents(OwnerComponents);
	for (UActorComponent* OwnerComponentCandidate : OwnerComponents)
	{
		AddAttackAbilityDataProvider(OwnerComponentCandidate, OutProviders, ProviderKeys);
	}

	TArray<AActor*> AttachedActors;
	OwnerCharacter->GetAttachedActors(AttachedActors);
	for (AActor* AttachedActor : AttachedActors)
	{
		AddAttackAbilityDataProvider(AttachedActor, OutProviders, ProviderKeys);
		if (!AttachedActor)
		{
			continue;
		}

		TArray<UActorComponent*> AttachedActorComponents;
		AttachedActor->GetComponents(AttachedActorComponents);
		for (UActorComponent* AttachedActorComponent : AttachedActorComponents)
		{
			AddAttackAbilityDataProvider(AttachedActorComponent, OutProviders, ProviderKeys);
		}
	}
}

void UMVMeleeAttackAbility::AddAttackAbilityDataProvider(
	UObject* Candidate,
	TArray<UObject*>& InOutProviders,
	TSet<TObjectKey<UObject>>& InOutProviderKeys) const
{
	if (!Candidate || !Candidate->GetClass()->ImplementsInterface(UMVAttackAbilityDataInterface::StaticClass()))
	{
		return;
	}

	const TObjectKey<UObject> CandidateKey(Candidate);
	if (InOutProviderKeys.Contains(CandidateKey))
	{
		return;
	}

	InOutProviderKeys.Add(CandidateKey);
	InOutProviders.Add(Candidate);
}

void UMVMeleeAttackAbility::DrawDebugTraceCapsule(
	const FMVMeleeAttackCapsule& Capsule,
	const FMVMeleeAttackTraceSample& Sample,
	const bool bHit,
	const FVector* PreviousCenter) const
{
	if (!bDrawDebugTrace)
	{
		return;
	}

	UWorld* World = GetAbilityWorld();
	if (!World)
	{
		return;
	}

	const FColor CapsuleColor = bHit ? DebugHitColor : DebugTraceColor;
	DrawDebugCapsule(
		World,
		Capsule.Center,
		Capsule.HalfHeight,
		Capsule.Radius,
		Capsule.Rotation,
		CapsuleColor,
		false,
		DebugDrawDuration,
		0,
		DebugLineThickness);

	DrawDebugLine(
		World,
		Sample.StartLocation,
		Sample.EndLocation,
		DebugSocketLineColor,
		false,
		DebugDrawDuration,
		0,
		DebugLineThickness);

	if (PreviousCenter)
	{
		DrawDebugLine(
			World,
			*PreviousCenter,
			Capsule.Center,
			DebugTraceColor,
			false,
			DebugDrawDuration,
			0,
			DebugLineThickness);
	}
}

float UMVMeleeAttackAbility::ResolveTraceCapsuleRadius() const
{
	return FMath::Max(1.0f, TraceCapsuleRadius);
}

int32 UMVMeleeAttackAbility::ResolveSubstepCount(
	const FMVMeleeAttackTraceSample& PreviousSample,
	const FMVMeleeAttackTraceSample& CurrentSample) const
{
	const float MaxSocketTravelDistance = FMath::Max(
		FVector::Distance(PreviousSample.StartLocation, CurrentSample.StartLocation),
		FVector::Distance(PreviousSample.EndLocation, CurrentSample.EndLocation));
	const float EffectiveStepDistance = FMath::Max(1.0f, MaxTraceStepDistance);
	return FMath::Max(1, FMath::CeilToInt(MaxSocketTravelDistance / EffectiveStepDistance));
}

UWorld* UMVMeleeAttackAbility::GetAbilityWorld() const
{
	const UActorComponent* Owner = OwnerComponent.Get();
	return Owner ? Owner->GetWorld() : nullptr;
}

bool UMVMeleeAttackAbility::BuildTraceCapsule(
	const FMVMeleeAttackTraceSample& Sample,
	const float CapsuleRadius,
	FMVMeleeAttackCapsule& OutCapsule)
{
	const float ResolvedRadius = FMath::Max(1.0f, CapsuleRadius);
	const FVector Segment = Sample.EndLocation - Sample.StartLocation;
	const float SegmentLength = Segment.Size();
	const FVector Direction = SegmentLength > KINDA_SMALL_NUMBER
		? Segment / SegmentLength
		: FVector::UpVector;

	OutCapsule.Center = (Sample.StartLocation + Sample.EndLocation) * 0.5f;
	OutCapsule.Rotation = FQuat::FindBetweenNormals(FVector::UpVector, Direction);
	OutCapsule.Radius = ResolvedRadius;
	OutCapsule.HalfHeight = FMath::Max(ResolvedRadius, SegmentLength * 0.5f + ResolvedRadius);
	return true;
}

AMVCharacterBase* UMVMeleeAttackAbility::ResolveHitCharacter(AActor* HitActor)
{
	AActor* CurrentActor = HitActor;
	while (CurrentActor)
	{
		if (AMVCharacterBase* Character = Cast<AMVCharacterBase>(CurrentActor))
		{
			return Character;
		}

		AActor* OwnerActor = CurrentActor->GetOwner();
		if (OwnerActor == CurrentActor)
		{
			break;
		}
		CurrentActor = OwnerActor;
	}

	return nullptr;
}
