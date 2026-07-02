#include "Animation/NotifyStates/MVAnimNotifyState_RecoveryEscapeWindow.h"

#include "Components/MVInputManagerComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

namespace
{
UMVInputManagerComponent* FindRecoveryEscapeInputManagerComponent(const USkeletalMeshComponent* MeshComp)
{
	AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;
	return Owner ? Owner->FindComponentByClass<UMVInputManagerComponent>() : nullptr;
}
}

void UMVAnimNotifyState_RecoveryEscapeWindow::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (UMVInputManagerComponent* InputManager = FindRecoveryEscapeInputManagerComponent(MeshComp))
	{
		InputManager->BeginRecoveryEscapeWindow();
	}
}

void UMVAnimNotifyState_RecoveryEscapeWindow::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (UMVInputManagerComponent* InputManager = FindRecoveryEscapeInputManagerComponent(MeshComp))
	{
		InputManager->EndRecoveryEscapeWindow();
	}
}

FString UMVAnimNotifyState_RecoveryEscapeWindow::GetNotifyName_Implementation() const
{
	return TEXT("Recovery Escape Window");
}
