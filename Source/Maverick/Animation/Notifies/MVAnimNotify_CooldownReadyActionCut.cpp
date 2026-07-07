#include "Animation/Notifies/MVAnimNotify_CooldownReadyActionCut.h"

#include "AI/MVActionCooldownComponent.h"
#include "Components/MVActionComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

namespace
{
AActor* FindCooldownReadyActionCutOwner(const USkeletalMeshComponent* MeshComp)
{
	return MeshComp ? MeshComp->GetOwner() : nullptr;
}
}

void UMVAnimNotify_CooldownReadyActionCut::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (CooldownActionId.IsNone())
	{
		return;
	}

	AActor* Owner = FindCooldownReadyActionCutOwner(MeshComp);
	if (!Owner)
	{
		return;
	}

	const UMVActionCooldownComponent* CooldownComponent = Owner->FindComponentByClass<UMVActionCooldownComponent>();
	if (!CooldownComponent || !CooldownComponent->IsCooldownReady(CooldownActionId))
	{
		return;
	}

	UMVActionComponent* ActionComponent = Owner->FindComponentByClass<UMVActionComponent>();
	if (!ActionComponent || !ActionComponent->IsActionRunning())
	{
		return;
	}

	ActionComponent->CancelActiveAction(BlendOutTime);
}

FString UMVAnimNotify_CooldownReadyActionCut::GetNotifyName_Implementation() const
{
	return CooldownActionId.IsNone()
		? TEXT("Cooldown Ready Action Cut")
		: FString::Printf(TEXT("Cooldown Ready Action Cut: %s"), *CooldownActionId.ToString());
}
