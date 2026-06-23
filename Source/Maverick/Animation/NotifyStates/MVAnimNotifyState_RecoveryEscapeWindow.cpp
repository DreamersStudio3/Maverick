#include "Animation/NotifyStates/MVAnimNotifyState_RecoveryEscapeWindow.h"

#include "Components/MVActionComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

namespace
{
UMVActionComponent* FindRecoveryEscapeActionComponent(const USkeletalMeshComponent* MeshComp)
{
	AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;
	return Owner ? Owner->FindComponentByClass<UMVActionComponent>() : nullptr;
}
}

void UMVAnimNotifyState_RecoveryEscapeWindow::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (UMVActionComponent* ActionComponent = FindRecoveryEscapeActionComponent(MeshComp))
	{
		ActionComponent->BeginRecoveryEscapeWindow();
	}
}

void UMVAnimNotifyState_RecoveryEscapeWindow::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (UMVActionComponent* ActionComponent = FindRecoveryEscapeActionComponent(MeshComp))
	{
		ActionComponent->EndRecoveryEscapeWindow();
	}
}

FString UMVAnimNotifyState_RecoveryEscapeWindow::GetNotifyName_Implementation() const
{
	return TEXT("Recovery Escape Window");
}
