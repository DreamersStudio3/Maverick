#include "Combat/Boss/Duelist/MVDuelistApproachComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/MVActionComponent.h"
#include "Components/MVStatComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/RootMotionSource.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

UMVDuelistApproachComponent::UMVDuelistApproachComponent()
{
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Material(TEXT("/Engine/EngineMaterials/EmissiveMeshMaterial.EmissiveMeshMaterial"));
	DashMaterial = Material.Object;
}

bool UMVDuelistApproachComponent::CanContinue(const AActor* Target) const
{
	const ACharacter* Pawn = Cast<ACharacter>(GetOwner());
	if (!Pawn || !IsValid(Target) || Target->IsActorBeingDestroyed() || !Pawn->GetCharacterMovement()->IsMovingOnGround()) return false;
	const UMVStatComponent* Stat = Pawn->FindComponentByClass<UMVStatComponent>();
	const UMVActionComponent* Action = Pawn->FindComponentByClass<UMVActionComponent>();
	return Stat && !Stat->IsDead() && !Stat->IsGroggy() && Action && !Action->IsActionRunning();
}

bool UMVDuelistApproachComponent::FindDestination(const AActor* Target, FVector& End) const
{
	if (!CanContinue(Target) || MotionId || GetWorld()->GetTimeSeconds() < NextReadyTime
		|| StopRange <= 0 || StopRange >= AttackRange || Duration <= 0) return false;
	const ACharacter* Pawn = CastChecked<ACharacter>(GetOwner());
	const FVector Start = Pawn->GetActorLocation();
	const FVector Delta = Target->GetActorLocation() - Start;
	const float Travel = Delta.Size2D() - StopRange;
	const UCharacterMovementComponent* Movement = Pawn->GetCharacterMovement();
	if (Delta.Size2D() <= AttackRange || Travel > MaxTravel || FMath::Abs(Delta.Z) > Movement->MaxStepHeight) return false;
	End = Start + Delta.GetSafeNormal2D() * Travel;
	const UCapsuleComponent* Capsule = Pawn->GetCapsuleComponent();
	FCollisionQueryParams Query(SCENE_QUERY_STAT(DuelistApproach), false, Pawn);
	const ECollisionChannel Channel = Capsule->GetCollisionObjectType();
	const FCollisionResponseParams Response(Capsule->GetCollisionResponseToChannels());
	if (GetWorld()->SweepTestByChannel(Start, End, FQuat::Identity, Channel,
		Capsule->GetCollisionShape(-2.f), Query, Response)) return false;

	// 짧은 간격으로 발밑을 확인해 도착점 사이의 절벽도 제외한다.
	Query.AddIgnoredActor(Target);
	const float HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
	const int32 Steps = FMath::Max(1, FMath::CeilToInt(Travel / FMath::Max(10.f, Capsule->GetScaledCapsuleRadius())));
	for (int32 Index = 1; Index <= Steps; ++Index)
	{
		const FVector Foot = FMath::Lerp(Start, End, float(Index) / Steps) - FVector::UpVector * HalfHeight;
		FHitResult Hit;
		const FVector Step = FVector::UpVector * (Movement->MaxStepHeight + 5.f);
		if (!GetWorld()->LineTraceSingleByChannel(Hit, Foot + Step, Foot - Step, Channel, Query, Response)
			|| !Movement->IsWalkable(Hit)) return false;
	}
	return true;
}

bool UMVDuelistApproachComponent::StartApproach(AActor* Target)
{
	FVector End;
	if (!FindDestination(Target, End)) return false;
	ACharacter* Pawn = CastChecked<ACharacter>(GetOwner());
	auto Source = MakeShared<FRootMotionSource_MoveToForce>();
	Source->InstanceName = TEXT("DuelistApproach");
	Source->Priority = 500;
	Source->StartLocation = Pawn->GetActorLocation();
	Source->TargetLocation = End;
	Source->Duration = Duration;
	Source->bRestrictSpeedToExpected = true;
	Source->FinishVelocityParams.Mode = ERootMotionFinishVelocityMode::SetVelocity;
	Source->FinishVelocityParams.SetVelocity = FVector::ZeroVector;
	MotionId = Pawn->GetCharacterMovement()->ApplyRootMotionSource(Source);
	if (!MotionId) return false;
	if (USkeletalMeshComponent* Mesh = Pawn->GetMesh(); Mesh && DashMaterial)
	{
		if (!DashFlash) DashFlash = UMaterialInstanceDynamic::Create(DashMaterial, this);
		DashFlash->SetVectorParameterValue(TEXT("Color"), DashColor);
		DashFlash->SetVectorParameterValue(TEXT("EmissiveColor"), DashColor);
		for (int32 Slot = 0; Slot < Mesh->GetNumMaterials(); ++Slot)
		{
			PreviousMaterials.Add(Mesh->GetMaterial(Slot));
			Mesh->SetMaterial(Slot, DashFlash);
		}
	}

	NextReadyTime = GetWorld()->GetTimeSeconds() + Cooldown;
	return true;
}

bool UMVDuelistApproachComponent::TurnTowards(const AActor* Target, float DeltaSeconds) const
{
	AActor* Owner = GetOwner();
	if (!Owner || !IsValid(Target)) return true;
	const FVector Direction = (Target->GetActorLocation() - Owner->GetActorLocation()).GetSafeNormal2D();
	if (Direction.IsNearlyZero()) return true;
	FRotator Rotation = Owner->GetActorRotation();
	const float DesiredYaw = Direction.Rotation().Yaw;
	Rotation.Yaw = FMath::FixedTurn(Rotation.Yaw, DesiredYaw, FMath::Max(1.f, TurnRate) * DeltaSeconds);
	Owner->SetActorRotation(Rotation);
	return FMath::Abs(FMath::FindDeltaAngleDegrees(Rotation.Yaw, DesiredYaw)) <= 2.f;
}

bool UMVDuelistApproachComponent::IsMoving() const
{
	const ACharacter* Pawn = Cast<ACharacter>(GetOwner());
	return MotionId && Pawn && Pawn->GetCharacterMovement()->GetRootMotionSourceByID(MotionId).IsValid();
}

void UMVDuelistApproachComponent::StopApproach()
{
	if (MotionId)
	{
		if (ACharacter* Pawn = Cast<ACharacter>(GetOwner()))
			Pawn->GetCharacterMovement()->RemoveRootMotionSourceByID(MotionId);
		if (ACharacter* Owner = Cast<ACharacter>(GetOwner()))
		{
			USkeletalMeshComponent* Mesh = Owner->GetMesh();
			for (int32 Slot = 0; Mesh && Slot < PreviousMaterials.Num() && Slot < Mesh->GetNumMaterials(); ++Slot)
			{
				if (Mesh->GetMaterial(Slot) == DashFlash) Mesh->SetMaterial(Slot, PreviousMaterials[Slot]);
			}
		}
		PreviousMaterials.Reset();
		MotionId = 0;
	}
}

void UMVDuelistApproachComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	StopApproach();
	Super::EndPlay(Reason);
}
