#include "Combat/Boss/Duelist/MVDuelistDash.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/RootMotionSource.h"
#include "Engine/World.h"

bool MVDuelistDash::HasPath(const ACharacter* Pawn, const AActor* Target, const FVector& End)
{
	const FVector Start = Pawn->GetActorLocation();
	const float Travel = FVector::Dist2D(Start, End);
	const UCharacterMovementComponent* Movement = Pawn->GetCharacterMovement();
	const UCapsuleComponent* Capsule = Pawn->GetCapsuleComponent();
	FCollisionQueryParams Query(SCENE_QUERY_STAT(DuelistApproach), false, Pawn);
	const ECollisionChannel Channel = Capsule->GetCollisionObjectType();
	const FCollisionResponseParams Response(Capsule->GetCollisionResponseToChannels());
	if (Pawn->GetWorld()->SweepTestByChannel(Start, End, FQuat::Identity, Channel,
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
		if (!Pawn->GetWorld()->LineTraceSingleByChannel(Hit, Foot + Step, Foot - Step, Channel, Query, Response)
			|| !Movement->IsWalkable(Hit)) return false;
	}
	return true;
}

uint16 MVDuelistDash::Start(ACharacter* Pawn, const FVector& End, float Duration, FName Name)
{
	auto Source = MakeShared<FRootMotionSource_MoveToForce>();
	Source->InstanceName = Name;
	Source->Priority = 500;
	Source->StartLocation = Pawn->GetActorLocation();
	Source->TargetLocation = End;
	Source->Duration = Duration;
	Source->bRestrictSpeedToExpected = true;
	Source->FinishVelocityParams.Mode = ERootMotionFinishVelocityMode::SetVelocity;
	Source->FinishVelocityParams.SetVelocity = FVector::ZeroVector;
	return Pawn->GetCharacterMovement()->ApplyRootMotionSource(Source);
}
