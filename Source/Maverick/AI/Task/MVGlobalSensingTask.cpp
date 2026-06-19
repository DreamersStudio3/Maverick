
#include "MVGlobalSensingTask.h"
#include "AIController.h"
#include "DrawDebugHelpers.h"
#include "StateTreeExecutionContext.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialExpressionLocalPosition.h"

namespace
{
	constexpr int32 CombatAreaDebugSegments = 96;
	constexpr float CombatAreaDebugThickness = 2.0f;

	const TCHAR* LexToString(const EMVBossCombatArea Area)
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

	void DrawCombatAreaCircle(const UWorld* World, const FVector& Center, const float Radius, const FColor& Color,
		const float Duration)
	{
		if (!World || Radius <= 0.0f)
		{
			return;
		}

		const float AngleStep = 2.0f * UE_PI / static_cast<float>(CombatAreaDebugSegments);
		FVector PreviousPoint = Center + FVector(Radius, 0.0f, 0.0f);

		for (int32 Index = 1; Index <= CombatAreaDebugSegments; ++Index)
		{
			const float Angle = AngleStep * static_cast<float>(Index);
			const FVector CurrentPoint = Center + FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0.0f);
			DrawDebugLine(World, PreviousPoint, CurrentPoint, Color, false, Duration, 0, CombatAreaDebugThickness);
			PreviousPoint = CurrentPoint;
		}
	}

	void DrawCombatAreaDebug(const FMVGlobalSensingTaskInstanceData& InstanceData, const FVector& OwnerLocation,
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

		//DrawCombatAreaCircle(World, DrawOrigin, InstanceData.OutsideArea, FColor::Cyan, Duration);
		DrawCombatAreaCircle(World, DrawOrigin, InstanceData.DefensiveArea, FColor::Yellow, Duration);
		DrawCombatAreaCircle(World, DrawOrigin, InstanceData.OffensiveArea, FColor::Red, Duration);

		DrawDebugLine(World, DrawOrigin, DrawTarget, FColor::White, false, Duration, 0, 1.5f);
		DrawDebugSphere(World, DrawTarget, 20.0f, 12, FColor::White, false, Duration, 0, 1.5f);

		const FString AreaText = FString::Printf(TEXT("CombatArea: %s  Distance: %.0f"),
			LexToString(InstanceData.CurrentArea), InstanceData.DistanceToTarget);
		DrawDebugString(World, DrawOrigin + FVector(0.0f, 0.0f, 60.0f), AreaText, InstanceData.Owner.Get(),
			FColor::White, Duration, true);
	}
}

EStateTreeRunStatus FMVGlobalSensingTask::EnterState(FStateTreeExecutionContext& Context,
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

	UWorld* World = InstanceData.Owner->GetWorld();
	if (!World)
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.Target = UGameplayStatics::GetPlayerPawn(World, 0);

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FMVGlobalSensingTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (!InstanceData.Target || !InstanceData.Owner)
	{
		return EStateTreeRunStatus::Failed;
	}

	const FVector TargetLocation = InstanceData.Target->GetActorLocation();
	const FVector OwnerLocation = InstanceData.Owner->GetActorLocation();
	
	//타겟과 거리 구하는 로직
	InstanceData.DistanceToTarget = FVector::Dist(OwnerLocation, TargetLocation);
	
	
	//타겟 방향 구하는 로직
	FVector TargetDirection = TargetLocation - OwnerLocation;
	TargetDirection.Z = 0.0f;

	if (TargetDirection.IsNearlyZero())
	{
		return EStateTreeRunStatus::Failed;
	}

	TargetDirection.Normalize();
	
	//내적 
	float DotProduct = FVector::DotProduct(InstanceData.Owner->GetActorForwardVector(), TargetDirection);
	InstanceData.AngleToTarget = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(DotProduct, -1.0f, 1.0f)));
	
	//외적
	FVector CrossProduct = FVector::CrossProduct(InstanceData.Owner->GetActorForwardVector(), TargetDirection);
	
	//앞/뒤/좌/우 판정
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
	
	//Area 판정
	//OutSideArea
	if (InstanceData.DistanceToTarget > InstanceData.DefensiveArea )
	{
		InstanceData.CurrentArea = EMVBossCombatArea::OutsideArea;
	}
	//DefensiveArea
	if ((InstanceData.DefensiveArea > InstanceData.DistanceToTarget) 
		&& (InstanceData.DistanceToTarget > InstanceData.OffensiveArea))
	{
		InstanceData.CurrentArea = EMVBossCombatArea::DefensiveArea;
	}
	//OffensiveArea
	if (InstanceData.OffensiveArea > InstanceData.DistanceToTarget)
	{
		InstanceData.CurrentArea = EMVBossCombatArea::OffensiveArea;
	}

	DrawCombatAreaDebug(InstanceData, OwnerLocation, TargetLocation);
	
	//UE_LOG(LogTemp,Log,TEXT("CurrentArea: %hhd"), InstanceData.CurrentArea)
	
	return EStateTreeRunStatus::Running;
}

void FMVGlobalSensingTask::ExitState(FStateTreeExecutionContext& Context,
                                        const FStateTreeTransitionResult& Transition) const
{
	FStateTreeTaskCommonBase::ExitState(Context, Transition);
}
