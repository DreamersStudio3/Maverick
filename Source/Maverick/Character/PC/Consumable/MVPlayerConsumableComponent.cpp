#include "Character/PC/Consumable/MVPlayerConsumableComponent.h"

#include "Character/PC/MVPlayerCharacter.h"
#include "Components/MVActionComponent.h"
#include "Components/MVInputManagerComponent.h"
#include "Components/MVStatComponent.h"
#include "Engine/DataTable.h"
#include "Tables/MVActionTableTypes.h"
#include "Tags/MVGameplayTags.h"

UMVPlayerConsumableComponent::UMVPlayerConsumableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	HealingPotionData.ItemTag = MVGameplayTags::Item_HealingPotion_Basic;
	HealingPotionData.DisplayName = NSLOCTEXT("MaverickConsumable", "BasicHealingPotionDisplayName", "Healing Potion");
	HealingPotionData.HealAmount = 30.0f;
	HealingPotionData.DefaultCount = 3;
	HealingPotionData.MaxCarryCount = 3;
	HealingPotionHotKeyText = NSLOCTEXT("MaverickConsumable", "HealingPotionHotKey", "Use");
}

void UMVPlayerConsumableComponent::BeginPlay()
{
	Super::BeginPlay();

	Initialize(Cast<AMVPlayerCharacter>(GetOwner()));
}

void UMVPlayerConsumableComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindOwnerEvents();
	OwnerCharacter = nullptr;

	Super::EndPlay(EndPlayReason);
}

void UMVPlayerConsumableComponent::Initialize(AMVPlayerCharacter* InOwnerCharacter)
{
	UnbindOwnerEvents();
	OwnerCharacter = InOwnerCharacter;
	InitializeHealingPotionState();
	BindOwnerEvents();
	BroadcastHealingPotionStateChanged();
}

bool UMVPlayerConsumableComponent::TryUseHealingPotion()
{
	if (!CanUseHealingPotion())
	{
		BroadcastHealingPotionStateChanged();
		return false;
	}

	if (HealingPotionData.UseActionRow.DataTable && !HealingPotionData.UseActionRow.RowName.IsNone())
	{
		UMVActionComponent* ActionComponent = OwnerCharacter ? OwnerCharacter->ActionComponent : nullptr;
		const bool bShouldTransition = CanTransitionActiveActionForHealingPotion();
		const bool bStarted = ActionComponent
			&& (bShouldTransition
				? ActionComponent->TryTransitionActionFromRowHandle(
					HealingPotionData.UseActionRow,
					HealingPotionData.UseActionStartSection)
				: ActionComponent->TryStartActionFromRowHandle(
					HealingPotionData.UseActionRow,
					HealingPotionData.UseActionStartSection));
		if (!bStarted)
		{
			BroadcastHealingPotionStateChanged();
			return false;
		}

		BroadcastHealingPotionStateChanged();
		return true;
	}

	return ApplyHealingPotionEffect();
}

bool UMVPlayerConsumableComponent::ApplyHealingPotionEffect()
{
	UMVStatComponent* StatComponent = OwnerCharacter ? OwnerCharacter->StatComponent : nullptr;
	if (!StatComponent || StatComponent->IsDead())
	{
		return false;
	}

	if (HealingPotionState.CurrentCount <= 0
		|| HealingPotionState.HealAmount <= 0.0f)
	{
		BroadcastHealingPotionStateChanged();
		return false;
	}

	StatComponent->RecoverHP(HealingPotionState.HealAmount);
	SetHealingPotionCount(HealingPotionState.CurrentCount - 1);
	return true;
}

void UMVPlayerConsumableComponent::RestoreConsumableCountsForWorldReset()
{
	// 월드 리셋은 모든 인벤토리성 아이템이 아니라, 카운트 복구 대상 소비 아이템만 이 진입점에 모아 처리한다.
	RestoreHealingPotionCountToDefault();
}

void UMVPlayerConsumableComponent::RestoreHealingPotionCountToDefault()
{
	const int32 MaxCarryCount = FMath::Max(0, HealingPotionData.MaxCarryCount);
	const int32 DefaultCount = FMath::Clamp(HealingPotionData.DefaultCount, 0, MaxCarryCount);
	HealingPotionState.CurrentCount = DefaultCount;
	HealingPotionState.MaxCarryCount = MaxCarryCount;
	bHealingPotionUseActionRunning = false;
	BroadcastHealingPotionStateChanged();
}

void UMVPlayerConsumableComponent::SetHealingPotionCount(const int32 NewCount)
{
	HealingPotionState.MaxCarryCount = FMath::Max(0, HealingPotionState.MaxCarryCount);
	HealingPotionState.CurrentCount = FMath::Clamp(NewCount, 0, HealingPotionState.MaxCarryCount);
	BroadcastHealingPotionStateChanged();
}

FMVHealingPotionRuntimeState UMVPlayerConsumableComponent::GetHealingPotionState() const
{
	FMVHealingPotionRuntimeState State = HealingPotionState;
	State.bCanUse = CanUseHealingPotion();
	return State;
}

FMVQuickSlotViewData UMVPlayerConsumableComponent::BuildHealingPotionQuickSlotViewData() const
{
	const FMVHealingPotionRuntimeState State = GetHealingPotionState();

	FMVQuickSlotViewData ViewData;
	ViewData.Icon = HealingPotionData.Icon.IsNull() ? nullptr : HealingPotionData.Icon.LoadSynchronous();
	ViewData.HotKeyText = HealingPotionHotKeyText;
	ViewData.Count = State.CurrentCount;
	ViewData.bLocked = State.CurrentCount <= 0;
	return ViewData;
}

bool UMVPlayerConsumableComponent::IsHealingPotionUseActionRunning() const
{
	return bHealingPotionUseActionRunning;
}

bool UMVPlayerConsumableComponent::TryHandleActionInput(
	const FGameplayTag ActionInputTag,
	const FVector2D /*ControllerSpaceInput*/,
	const bool /*bHasMovementInput*/)
{
	if (ActionInputTag.MatchesTagExact(MVGameplayTags::Action_Input_UseConsumable))
	{
		return TryUseHealingPotion();
	}

	return false;
}

void UMVPlayerConsumableComponent::HandleHPChanged(float /*CurrentValue*/, float /*MaxValue*/)
{
	BroadcastHealingPotionStateChanged();
}

void UMVPlayerConsumableComponent::HandleActionStarted(const FName ActionTableName, const FName ActionRowName)
{
	bHealingPotionUseActionRunning = IsHealingPotionUseAction(ActionTableName, ActionRowName);
	BroadcastHealingPotionStateChanged();
}

void UMVPlayerConsumableComponent::HandleActionEnded(
	const FName ActionTableName,
	const FName ActionRowName,
	bool /*bInterrupted*/)
{
	if (bHealingPotionUseActionRunning || IsHealingPotionUseAction(ActionTableName, ActionRowName))
	{
		bHealingPotionUseActionRunning = false;
		BroadcastHealingPotionStateChanged();
	}
}

void UMVPlayerConsumableComponent::BindOwnerEvents()
{
	if (!OwnerCharacter)
	{
		return;
	}

	if (UMVInputManagerComponent* InputManager = OwnerCharacter->InputManagerComponent)
	{
		InputManager->RegisterActionInputHandler(this, MVActionInputHandlerPriorities::Consumable);
	}

	if (UMVActionComponent* ActionComponent = OwnerCharacter->ActionComponent)
	{
		ActionComponent->OnActionStarted.RemoveDynamic(
			this,
			&UMVPlayerConsumableComponent::HandleActionStarted);
		ActionComponent->OnActionStarted.AddUniqueDynamic(
			this,
			&UMVPlayerConsumableComponent::HandleActionStarted);
		ActionComponent->OnActionEnded.RemoveDynamic(
			this,
			&UMVPlayerConsumableComponent::HandleActionEnded);
		ActionComponent->OnActionEnded.AddUniqueDynamic(
			this,
			&UMVPlayerConsumableComponent::HandleActionEnded);
	}

	if (UMVStatComponent* StatComponent = OwnerCharacter->StatComponent)
	{
		StatComponent->OnHPChanged.RemoveDynamic(
			this,
			&UMVPlayerConsumableComponent::HandleHPChanged);
		StatComponent->OnHPChanged.AddUniqueDynamic(
			this,
			&UMVPlayerConsumableComponent::HandleHPChanged);
	}
}

void UMVPlayerConsumableComponent::UnbindOwnerEvents()
{
	if (!OwnerCharacter)
	{
		return;
	}

	if (UMVInputManagerComponent* InputManager = OwnerCharacter->InputManagerComponent)
	{
		InputManager->UnregisterActionInputHandler(this);
	}

	if (UMVActionComponent* ActionComponent = OwnerCharacter->ActionComponent)
	{
		ActionComponent->OnActionStarted.RemoveDynamic(
			this,
			&UMVPlayerConsumableComponent::HandleActionStarted);
		ActionComponent->OnActionEnded.RemoveDynamic(
			this,
			&UMVPlayerConsumableComponent::HandleActionEnded);
	}

	if (UMVStatComponent* StatComponent = OwnerCharacter->StatComponent)
	{
		StatComponent->OnHPChanged.RemoveDynamic(
			this,
			&UMVPlayerConsumableComponent::HandleHPChanged);
	}
}

void UMVPlayerConsumableComponent::InitializeHealingPotionState()
{
	const int32 MaxCarryCount = FMath::Max(0, HealingPotionData.MaxCarryCount);

	HealingPotionState.ItemTag = HealingPotionData.ItemTag;
	HealingPotionState.HealAmount = FMath::Max(0.0f, HealingPotionData.HealAmount);
	HealingPotionState.MaxCarryCount = MaxCarryCount;
	HealingPotionState.CurrentCount = FMath::Clamp(HealingPotionData.DefaultCount, 0, MaxCarryCount);
	HealingPotionState.bCanUse = false;
	bHealingPotionUseActionRunning = false;
}

bool UMVPlayerConsumableComponent::CanUseHealingPotion() const
{
	const UMVStatComponent* StatComponent = OwnerCharacter ? OwnerCharacter->StatComponent : nullptr;
	const UMVActionComponent* ActionComponent = OwnerCharacter ? OwnerCharacter->ActionComponent : nullptr;
	if (!StatComponent || StatComponent->IsDead())
	{
		return false;
	}

	if (ActionComponent && ActionComponent->IsActionRunning())
	{
		if (!HasHealingPotionUseAction() || !CanTransitionActiveActionForHealingPotion())
		{
			return false;
		}
	}

	return HealingPotionState.CurrentCount > 0
		&& HealingPotionState.HealAmount > 0.0f;
}

bool UMVPlayerConsumableComponent::HasHealingPotionUseAction() const
{
	return HealingPotionData.UseActionRow.DataTable && !HealingPotionData.UseActionRow.RowName.IsNone();
}

bool UMVPlayerConsumableComponent::CanTransitionActiveActionForHealingPotion() const
{
	const UMVActionComponent* ActionComponent = OwnerCharacter ? OwnerCharacter->ActionComponent : nullptr;
	if (!ActionComponent || !ActionComponent->IsActionRunning() || !ActionComponent->CanInterruptActiveAction())
	{
		return false;
	}

	const UMVInputManagerComponent* InputManager = OwnerCharacter ? OwnerCharacter->InputManagerComponent : nullptr;
	return InputManager && InputManager->IsRecoveryEscapeWindowOpen();
}

bool UMVPlayerConsumableComponent::IsHealingPotionUseAction(
	const FName ActionTableName,
	const FName ActionRowName) const
{
	return HasHealingPotionUseAction()
		&& ActionTableName == ResolveHealingPotionUseActionTableName()
		&& ActionRowName == HealingPotionData.UseActionRow.RowName;
}

FName UMVPlayerConsumableComponent::ResolveHealingPotionUseActionTableName() const
{
	const UDataTable* DataTable = HealingPotionData.UseActionRow.DataTable;
	if (!DataTable)
	{
		return NAME_None;
	}

	FString TableName = DataTable->GetName();
	TableName.RemoveFromStart(TEXT("DT_"));
	return FName(*TableName);
}

void UMVPlayerConsumableComponent::BroadcastHealingPotionStateChanged()
{
	HealingPotionState.bCanUse = CanUseHealingPotion();
	OnHealingPotionStateChanged.Broadcast(HealingPotionState);
}
