#include "MVGlobalSensingTask.h"

#include "AIController.h"
#include "Components/MVActionComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "StateTreeExecutionContext.h"

namespace
{
constexpr int32 GlobalSensingCombatAreaDebugSegments = 96;
constexpr float GlobalSensingCombatAreaDebugThickness = 2.0f;

const TCHAR* GlobalSensingCombatAreaText(const EMVBossCombatArea Area)
{
	switch (Area)
	{
	case EMVBossCombatArea::OutsideArea:
		return TEXT("Outside");
	case EMVBossCombatArea::DefensiveArea:
		return TEXT("Defensive");
	case EMVBossCombatArea::OffensiveArea:
		return TEXT("Offensive");
	default:
		return TEXT("Unknown");
	}
}

void DrawGlobalSensingCombatAreaCircle(
	const UWorld* World,
	const FVector& Center,
	const float Radius,
	const FColor& Color,
	const float Duration)
{
	if (!World || Radius <= 0.0f)
	{
		return;
	}

	const float AngleStep = 2.0f * UE_PI / static_cast<float>(GlobalSensingCombatAreaDebugSegments);
	FVector PreviousPoint = Center + FVector(Radius, 0.0f, 0.0f);

	for (int32 Index = 1; Index <= GlobalSensingCombatAreaDebugSegments; ++Index)
	{
		const float Angle = AngleStep * static_cast<float>(Index);
		const FVector CurrentPoint = Center + FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0.0f);
		DrawDebugLine(World, PreviousPoint, CurrentPoint, Color, false, Duration, 0, GlobalSensingCombatAreaDebugThickness);
		PreviousPoint = CurrentPoint;
	}
}

bool GlobalSensingTraceClear(const AActor& Owner, const FVector& Direction, const float TraceDistance)
{
	if (TraceDistance <= 0.0f)
	{
		return true;
	}

	UWorld* World = Owner.GetWorld();
	const FVector TraceDirection = Direction.GetSafeNormal2D();
	if (!World || TraceDirection.IsNearlyZero())
	{
		return false;
	}

	FHitResult HitResult;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(MVGlobalSensingTask), false);
	QueryParams.AddIgnoredActor(&Owner);

	const FVector Start = Owner.GetActorLocation();
	const FVector End = Start + TraceDirection * TraceDistance;
	return !World->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams);
}

bool GlobalSensingLineOfSightClear(const AActor& Owner, const AActor& Target)
{
	UWorld* World = Owner.GetWorld();
	if (!World)
	{
		return false;
	}

	FHitResult HitResult;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(MVGlobalSensingLineOfSight), false);
	QueryParams.AddIgnoredActor(&Owner);

	const bool bHit = World->LineTraceSingleByChannel(
		HitResult,
		Owner.GetActorLocation(),
		Target.GetActorLocation(),
		ECC_Visibility,
		QueryParams);
	return !bHit || HitResult.GetActor() == &Target;
}

UMVActionCooldownComponent* GlobalSensingEnsureCooldownComponent(APawn& Owner)
{
	if (UMVActionCooldownComponent* ExistingComponent = Owner.FindComponentByClass<UMVActionCooldownComponent>())
	{
		return ExistingComponent;
	}

	UMVActionCooldownComponent* NewComponent = NewObject<UMVActionCooldownComponent>(&Owner, TEXT("ActionCooldownComponent"));
	if (!NewComponent)
	{
		return nullptr;
	}

	Owner.AddInstanceComponent(NewComponent);
	NewComponent->RegisterComponent();
	return NewComponent;
}

void UpdateGlobalSensingCooldownContext(FMVGlobalSensingTaskInstanceData& InstanceData, const float DeltaTime)
{
	InstanceData.ReadyActionIds.Reset();
	InstanceData.bAttackCadenceReady = true;

	if (!InstanceData.Owner)
	{
		return;
	}

	if (!InstanceData.CooldownComponent)
	{
		InstanceData.CooldownComponent = GlobalSensingEnsureCooldownComponent(*InstanceData.Owner);
		if (InstanceData.CooldownComponent)
		{
			InstanceData.CooldownComponent->ConfigureCooldowns(InstanceData.ActionCooldowns);
		}
	}

	if (!InstanceData.CooldownComponent)
	{
		return;
	}

	InstanceData.CooldownComponent->TickCooldowns(DeltaTime);
	InstanceData.CooldownComponent->GetReadyActionIds(InstanceData.ReadyActionIds);
	InstanceData.bAttackCadenceReady = InstanceData.AttackCadenceActionId.IsNone()
		|| InstanceData.CooldownComponent->IsCooldownReady(InstanceData.AttackCadenceActionId);
}

void DrawGlobalSensingCombatAreaDebug(
	const FMVGlobalSensingTaskInstanceData& InstanceData,
	const FVector& OwnerLocation,
	const FVector& TargetLocation)
{
	if (!InstanceData.bDrawCombatAreaDebug || !InstanceData.Owner)
	{
		return;
	}

	const UWorld* World = InstanceData.Owner->GetWorld();
	if (!World)
	{
		return;
	}

	const float Duration = InstanceData.CombatAreaDebugDuration;
	const FVector DrawOrigin = OwnerLocation + FVector(0.0f, 0.0f, InstanceData.CombatAreaDebugHeightOffset);
	const FVector DrawTarget = TargetLocation + FVector(0.0f, 0.0f, InstanceData.CombatAreaDebugHeightOffset);

	DrawGlobalSensingCombatAreaCircle(World, DrawOrigin, InstanceData.DefensiveArea, FColor::Yellow, Duration);
	DrawGlobalSensingCombatAreaCircle(World, DrawOrigin, InstanceData.OffensiveArea, FColor::Red, Duration);

	DrawDebugLine(World, DrawOrigin, DrawTarget, FColor::White, false, Duration, 0, 1.5f);
	DrawDebugSphere(World, DrawTarget, 20.0f, 12, FColor::White, false, Duration, 0, 1.5f);

	const FString AreaText = FString::Printf(
		TEXT("CombatArea: %s  Distance: %.0f"),
		GlobalSensingCombatAreaText(InstanceData.CurrentArea),
		InstanceData.DistanceToTarget);
	DrawDebugString(World, DrawOrigin + FVector(0.0f, 0.0f, 60.0f), AreaText, InstanceData.Owner.Get(), FColor::White, Duration, true);
}

void UpdateGlobalSensingCombatContext(FMVGlobalSensingTaskInstanceData& InstanceData)
{
	FMVAICombatContext& CombatContext = InstanceData.CombatContext;
	CombatContext.bHasTarget = InstanceData.bHasTarget;
	CombatContext.DistanceToTarget = InstanceData.DistanceToTarget;
	CombatContext.AngleToTarget = InstanceData.AngleToTarget;
	CombatContext.CurrentArea = InstanceData.CurrentArea;
	CombatContext.bHasLineOfSight = InstanceData.bHasLineOfSight;
	CombatContext.bActionRunning = InstanceData.bActionRunning;
	CombatContext.ReadyActionIds = InstanceData.ReadyActionIds;
	CombatContext.LastAttackTag = InstanceData.LastAttackTag;
	CombatContext.bAttackCadenceReady = InstanceData.bAttackCadenceReady;
	CombatContext.CurrentPhase = InstanceData.CurrentPhase;
	CombatContext.bCounterWindow = InstanceData.bCounterWindow;
	CombatContext.bSprintPathClear = InstanceData.bSprintPathClear;
	CombatContext.bAirborneChargePathClear = InstanceData.bAirborneChargePathClear;
	CombatContext.bTargetCanBeAirborne = InstanceData.bTargetCanBeAirborne;
	CombatContext.bTargetIsAirborne = InstanceData.bTargetIsAirborne;
	CombatContext.bShouldUseAirborneCharge = InstanceData.bShouldUseAirborneCharge;
	CombatContext.bNeedAttackAngle = InstanceData.bNeedAttackAngle;
	CombatContext.bNeedClearAttackPath = InstanceData.bNeedClearAttackPath;
	CombatContext.bStrafePathClear = InstanceData.bStrafePathClear;
	CombatContext.bIsDead = InstanceData.bIsDead;
}
}

EStateTreeRunStatus FMVGlobalSensingTask::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	
	if (const AAIController* AIController = Cast<AAIController>(Context.GetOwner()))
	{
		InstanceData.Owner = AIController->GetPawn();
	}
	else
	{
		InstanceData.Owner = Cast<APawn>(Context.GetOwner());
	}

	if (!InstanceData.Owner)
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.CooldownComponent = GlobalSensingEnsureCooldownComponent(*InstanceData.Owner);
	if (InstanceData.CooldownComponent)
	{
		InstanceData.CooldownComponent->ConfigureCooldowns(InstanceData.ActionCooldowns);
	}
	UpdateGlobalSensingCooldownContext(InstanceData, 0.0f);

	UWorld* World = InstanceData.Owner->GetWorld();
	if (!World)
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.Target = UGameplayStatics::GetPlayerPawn(World, 0);
	InstanceData.bHasTarget = InstanceData.Target != nullptr;
	UpdateGlobalSensingCombatContext(InstanceData);

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FMVGlobalSensingTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (!InstanceData.Target || !InstanceData.Owner)
	{
		UpdateGlobalSensingCooldownContext(InstanceData, DeltaTime);
		InstanceData.bHasTarget = false;
		InstanceData.bActionRunning = false;
		UpdateGlobalSensingCombatContext(InstanceData);
		return EStateTreeRunStatus::Failed;
	}

	UpdateGlobalSensingCooldownContext(InstanceData, DeltaTime);

	const FVector TargetLocation = InstanceData.Target->GetActorLocation();
	const FVector OwnerLocation = InstanceData.Owner->GetActorLocation();
	InstanceData.DistanceToTarget = FVector::Dist(OwnerLocation, TargetLocation);
	
	FVector TargetDirection = TargetLocation - OwnerLocation;
	TargetDirection.Z = 0.0f;

	if (TargetDirection.IsNearlyZero())
	{
		return EStateTreeRunStatus::Failed;
	}

	TargetDirection.Normalize();
	InstanceData.bHasTarget = true;
	InstanceData.bHasLineOfSight = GlobalSensingLineOfSightClear(*InstanceData.Owner, *InstanceData.Target);
	InstanceData.bActionRunning = false;
	if (const UMVActionComponent* ActionComponent = InstanceData.Owner->FindComponentByClass<UMVActionComponent>())
	{
		InstanceData.bActionRunning = ActionComponent->IsActionRunning();
	}
	
	const float DotProduct = FVector::DotProduct(InstanceData.Owner->GetActorForwardVector(), TargetDirection);
	InstanceData.AngleToTarget = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(DotProduct, -1.0f, 1.0f)));
	
	const FVector CrossProduct = FVector::CrossProduct(InstanceData.Owner->GetActorForwardVector(), TargetDirection);
	
	if (DotProduct > 0.5f)
	{
		InstanceData.AttackDirection = EMVAttackDirection::Forward;
	}
	if (DotProduct < 0.5f)
	{
		InstanceData.AttackDirection = EMVAttackDirection::Backward;
	}
	if (CrossProduct.Z > 0.0f && FMath::Abs(DotProduct) <= 0.5f)
	{
		InstanceData.AttackDirection = EMVAttackDirection::Left;
	}
	if (CrossProduct.Z < 0.0f && FMath::Abs(DotProduct) <= 0.5f)
	{
		InstanceData.AttackDirection = EMVAttackDirection::Right;
	}
	
	if (InstanceData.DistanceToTarget > InstanceData.DefensiveArea)
	{
		InstanceData.CurrentArea = EMVBossCombatArea::OutsideArea;
	}
	if (InstanceData.DefensiveArea > InstanceData.DistanceToTarget
		&& InstanceData.DistanceToTarget > InstanceData.OffensiveArea)
	{
		InstanceData.CurrentArea = EMVBossCombatArea::DefensiveArea;
	}
	if (InstanceData.OffensiveArea > InstanceData.DistanceToTarget)
	{
		InstanceData.CurrentArea = EMVBossCombatArea::OffensiveArea;
	}

	InstanceData.bSprintPathClear = GlobalSensingTraceClear(
		*InstanceData.Owner,
		TargetDirection,
		InstanceData.ForwardPathTraceDistance);
	InstanceData.bAirborneChargePathClear = InstanceData.bSprintPathClear;

	const FVector LeftStrafeDirection = FVector::CrossProduct(FVector::UpVector, TargetDirection);
	const FVector RightStrafeDirection = FVector::CrossProduct(TargetDirection, FVector::UpVector);
	InstanceData.bStrafePathClear =
		GlobalSensingTraceClear(*InstanceData.Owner, LeftStrafeDirection, InstanceData.StrafePathTraceDistance)
		|| GlobalSensingTraceClear(*InstanceData.Owner, RightStrafeDirection, InstanceData.StrafePathTraceDistance);

	InstanceData.bNeedAttackAngle = FMath::Abs(InstanceData.AngleToTarget) > InstanceData.AttackAngleTolerance;
	InstanceData.bNeedClearAttackPath = !InstanceData.bHasLineOfSight;

	UpdateGlobalSensingCombatContext(InstanceData);
	DrawGlobalSensingCombatAreaDebug(InstanceData, OwnerLocation, TargetLocation);
	
	return EStateTreeRunStatus::Running;
}

void FMVGlobalSensingTask::ExitState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FStateTreeTaskCommonBase::ExitState(Context, Transition);
}
