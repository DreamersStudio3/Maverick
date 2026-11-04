#include "AI/AITask/MVAIPatrolTask.h"

#include "AI/Controller/MVAIController.h"
#include "Character/NPC/Enemy/MVEnemy.h"
#include "Components/SplineComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "StateTreeExecutionContext.h"

namespace MaverickPatrolTaskPrivate
{
void ApplyMovementSettings(AMVEnemy& Enemy, const float Speed, const float TurnRate, const bool bMovementActive, const bool bRotationActive)
{
	UCharacterMovementComponent* Movement = Enemy.GetCharacterMovement();
	if (!Movement) return;
	if (bMovementActive) Movement->MaxWalkSpeed = Speed;
	if (bRotationActive)
	{
		Movement->RotationRate = FRotator(0.0f, TurnRate, 0.0f);
		Movement->bOrientRotationToMovement = true;
		Movement->bUseControllerDesiredRotation = false;
		Enemy.bUseControllerRotationYaw = false;
	}
}

void RestoreMovement(FMVAIPatrolTaskInstanceData& Data)
{
	if (!Data.MovementSettingsHandle.IsValid()) return;
	if (AMVEnemy* Enemy = Cast<AMVEnemy>(Data.Owner)) Enemy->OnLocomotionSettingsUpdated.Remove(Data.MovementSettingsHandle);
	Data.MovementSettingsHandle.Reset();
	if (IsValid(Data.Movement))
	{
		Data.Movement->MaxWalkSpeed = Data.PreviousMaxWalkSpeed;
		Data.Movement->RotationRate = Data.PreviousRotationRate;
		Data.Movement->bOrientRotationToMovement = Data.bPreviousOrientRotationToMovement;
		Data.Movement->bUseControllerDesiredRotation = Data.bPreviousUseControllerDesiredRotation;
	}
	if (IsValid(Data.Owner))
	{
		Data.Owner->bUseControllerRotationYaw = Data.bPreviousUseControllerRotationYaw;
		Data.Owner->ConsumeMovementInputVector();
	}
}

bool HasTarget(const AAIController* Controller)
{
	const AMVAIController* MVController = Cast<AMVAIController>(Controller);
	return IsValid(MVController) && IsValid(MVController->TargetActor);
}

APawn* ResolveOwner(FStateTreeExecutionContext& Context, APawn* BoundOwner)
{
	if (IsValid(BoundOwner)) return BoundOwner;
	const AAIController* Controller = Cast<AAIController>(Context.GetOwner());
	if (Controller) return Controller->GetPawn();
	return Cast<APawn>(Context.GetOwner());
}

bool UpdateMovement(FMVAIPatrolTaskInstanceData& Data)
{
	if (!IsValid(Data.Owner) || !IsValid(Data.Controller) || !IsValid(Data.Spline)
		|| !IsValid(Data.Movement) || Data.Controller->GetPawn() != Data.Owner) return false;
	const float Length = Data.Spline->GetSplineLength();
	if (Length < 10.0f) return false;
	const FVector Location = Data.Owner->GetActorLocation();
	const float Distance = Data.Spline->GetDistanceAlongSplineAtLocation(Data.Owner->GetNavAgentLocation(), ESplineCoordinateSpace::World);
	if (!Data.Spline->IsClosedLoop())
	{
		const float EndDistance = Data.Direction > 0.0f ? Length : 0.0f;
		const FVector End = Data.Spline->GetLocationAtDistanceAlongSpline(EndDistance, ESplineCoordinateSpace::World);
		if (FVector::Dist2D(Location, End) <= FMath::Clamp(Data.AcceptanceRadius, 0.0f, Length * 0.25f)) Data.Direction *= -1.0f;
	}
	const float LookAhead = FMath::Clamp(Data.LookAheadDistance, 1.0f, Length * 0.25f);
	float TargetDistance = Distance + Data.Direction * LookAhead;
	if (Data.Spline->IsClosedLoop()) TargetDistance = FMath::Fmod(TargetDistance + Length, Length);
	else TargetDistance = FMath::Clamp(TargetDistance, 0.0f, Length);
	const FVector Destination = Data.Spline->GetLocationAtDistanceAlongSpline(TargetDistance, ESplineCoordinateSpace::World);
	Data.Owner->AddMovementInput((Destination - Location).GetSafeNormal2D());
	return true;
}
}

FMVAIPatrolTask::FMVAIPatrolTask() { bShouldCallTick = true; }

EStateTreeRunStatus FMVAIPatrolTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& Data = Context.GetInstanceData<FInstanceDataType>(*this);
	Data.Direction = 1.0f;
	Data.Owner = MaverickPatrolTaskPrivate::ResolveOwner(Context, Data.Owner);
	Data.Controller = IsValid(Data.Owner) ? Cast<AAIController>(Data.Owner->GetController()) : nullptr;
	Data.Spline = nullptr;
	Data.Movement = nullptr;
	if (MaverickPatrolTaskPrivate::HasTarget(Data.Controller)) return EStateTreeRunStatus::Succeeded;
	AMVEnemy* Enemy = Cast<AMVEnemy>(Data.Owner);
	if (!IsValid(Enemy) || !IsValid(Data.Controller) || !IsValid(Enemy->PatrolPath)) return EStateTreeRunStatus::Failed;
	Data.Spline = Enemy->PatrolPath->FindComponentByClass<USplineComponent>();
	Data.Movement = Enemy->GetCharacterMovement();
	if (!IsValid(Data.Spline) || Data.Spline->GetSplineLength() < 10.0f || !IsValid(Data.Movement)) return EStateTreeRunStatus::Failed;
	Data.Controller->StopMovement();
	Data.PreviousMaxWalkSpeed = Data.Movement->MaxWalkSpeed;
	Data.PreviousRotationRate = Data.Movement->RotationRate;
	Data.bPreviousUseControllerRotationYaw = Enemy->bUseControllerRotationYaw;
	Data.bPreviousOrientRotationToMovement = Data.Movement->bOrientRotationToMovement;
	Data.bPreviousUseControllerDesiredRotation = Data.Movement->bUseControllerDesiredRotation;
	const float Speed = FMath::Max(1.0f, Data.PatrolSpeed);
	const float TurnRate = FMath::Max(1.0f, Data.TurnRate);
	Data.MovementSettingsHandle = Enemy->OnLocomotionSettingsUpdated.AddWeakLambda(Enemy,
		[Enemy, Speed, TurnRate](const bool bMovementActive, const bool bRotationActive)
		{
			MaverickPatrolTaskPrivate::ApplyMovementSettings(*Enemy, Speed, TurnRate, bMovementActive, bRotationActive);
		});
	MaverickPatrolTaskPrivate::ApplyMovementSettings(*Enemy, Speed, TurnRate, Enemy->IsCharacterMovementActive(), true);
	MaverickPatrolTaskPrivate::UpdateMovement(Data);
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FMVAIPatrolTask::Tick(FStateTreeExecutionContext& Context, float DeltaTime) const
{
	FInstanceDataType& Data = Context.GetInstanceData<FInstanceDataType>(*this);
	const bool bHasTarget = MaverickPatrolTaskPrivate::HasTarget(Data.Controller);
	if (bHasTarget || !MaverickPatrolTaskPrivate::UpdateMovement(Data))
	{
		MaverickPatrolTaskPrivate::RestoreMovement(Data);
		return bHasTarget ? EStateTreeRunStatus::Succeeded : EStateTreeRunStatus::Failed;
	}
	return EStateTreeRunStatus::Running;
}

void FMVAIPatrolTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& Data = Context.GetInstanceData<FInstanceDataType>(*this);
	MaverickPatrolTaskPrivate::RestoreMovement(Data);
	Data.Spline = nullptr;
	Data.Movement = nullptr;
	Data.Controller = nullptr;
	FStateTreeTaskCommonBase::ExitState(Context, Transition);
}
