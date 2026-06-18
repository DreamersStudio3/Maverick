#include "Animation/NotifyStates/MVAnimNotifyState_DodgeLaunch.h"

#include "Components/MVDodgeComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

namespace
{
UMVDodgeComponent* FindDodgeComponent(const USkeletalMeshComponent* MeshComp)
{
	const AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;
	return Owner ? Owner->FindComponentByClass<UMVDodgeComponent>() : nullptr;
}
}

void UMVAnimNotifyState_DodgeLaunch::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (UMVDodgeComponent* DodgeComponent = FindDodgeComponent(MeshComp))
	{
		DodgeComponent->BeginDodgeLaunchWindow(
			TotalDuration,
			DistanceCurve,
			DistanceScale,
			bApplyVerticalLaunchOnBegin);
	}
}

void UMVAnimNotifyState_DodgeLaunch::NotifyTick(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float FrameDeltaTime,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	if (UMVDodgeComponent* DodgeComponent = FindDodgeComponent(MeshComp))
	{
		DodgeComponent->TickDodgeLaunchWindow(FrameDeltaTime);
	}
}

void UMVAnimNotifyState_DodgeLaunch::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (UMVDodgeComponent* DodgeComponent = FindDodgeComponent(MeshComp))
	{
		DodgeComponent->EndDodgeLaunchWindow(bClearHorizontalVelocityOnEnd);
	}
}

FString UMVAnimNotifyState_DodgeLaunch::GetNotifyName_Implementation() const
{
	return TEXT("Dodge Launch");
}
