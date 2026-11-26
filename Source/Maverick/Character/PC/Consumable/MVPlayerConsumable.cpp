#include "Character/PC/Consumable/MVPlayerConsumable.h"

#include "Character/PC/MVPlayerCharacter.h"
#include "Components/MVActionComponent.h"
#include "Components/MVInputManagerComponent.h"
#include "Components/MVInventoryComponent.h"
#include "Components/MVStatComponent.h"
#include "Engine/DataTable.h"
#include "Tables/MVActionTableTypes.h"
#include "Tags/MVGameplayTags.h"

namespace
{
constexpr const TCHAR* MVPlayerConsumableDefaultPotionDataTablePath =
	TEXT("/Game/Table/Props/DT_Items_Consumable.DT_Items_Consumable");
constexpr const TCHAR* MVPlayerConsumableDefaultPotionActionTablePath =
	TEXT("/Game/Table/Props/DT_Props_P1.DT_Props_P1");
constexpr const TCHAR* MVPlayerConsumableDefaultPotionIconPath =
	TEXT("/Game/ArtAssets/Props/Consumables/HealingPotion/Icon/T_Icon_Props_Consumable_HealingPotion_512.T_Icon_Props_Consumable_HealingPotion_512");
const FName MVPlayerConsumableDefaultPotionDataRowName(TEXT("HealingPotion"));
const FName MVPlayerConsumableDefaultPotionActionRowName(TEXT("Items_HealingPotion"));
const FName MVPlayerConsumableDefaultPotionFailedActionRowName(TEXT("Items_Failed"));

FMVHealingPotionTableRow MakeMVPlayerConsumableDefaultHealingPotionData()
{
	FMVHealingPotionTableRow HealingPotionData;
	HealingPotionData.ItemTag = MVGameplayTags::Item_HealingPotion_Basic;
	HealingPotionData.DisplayName = NSLOCTEXT("MaverickConsumable", "BasicHealingPotionDisplayName", "Healing Potion");
	HealingPotionData.HealAmount = 30.0f;
	HealingPotionData.DefaultCount = 3;
	HealingPotionData.MaxCarryCount = 3;
	return HealingPotionData;
}

void ApplyMVPlayerConsumableHealingPotionDataRowDefaults(FDataTableRowHandle& HealingPotionDataRow)
{
	if (HealingPotionDataRow.DataTable && !HealingPotionDataRow.RowName.IsNone())
	{
		return;
	}

	if (UDataTable* DataTable = LoadObject<UDataTable>(
		nullptr,
		MVPlayerConsumableDefaultPotionDataTablePath))
	{
		HealingPotionDataRow.DataTable = DataTable;
		HealingPotionDataRow.RowName = MVPlayerConsumableDefaultPotionDataRowName;
	}
}

void ApplyMVPlayerConsumableHealingPotionDefaults(FMVHealingPotionTableRow& HealingPotionData)
{
	UDataTable* DefaultActionTable = nullptr;
	const auto ResolveDefaultActionTable = [&DefaultActionTable]() -> UDataTable*
	{
		if (!DefaultActionTable)
		{
			DefaultActionTable = LoadObject<UDataTable>(
				nullptr,
				MVPlayerConsumableDefaultPotionActionTablePath);
		}

		return DefaultActionTable;
	};

	if (!HealingPotionData.UseActionRow.DataTable || HealingPotionData.UseActionRow.RowName.IsNone())
	{
		if (UDataTable* ActionTable = ResolveDefaultActionTable())
		{
			HealingPotionData.UseActionRow.DataTable = ActionTable;
			HealingPotionData.UseActionRow.RowName = MVPlayerConsumableDefaultPotionActionRowName;
		}
	}

	if (!HealingPotionData.FailedActionRow.DataTable || HealingPotionData.FailedActionRow.RowName.IsNone())
	{
		if (UDataTable* ActionTable = ResolveDefaultActionTable())
		{
			HealingPotionData.FailedActionRow.DataTable = ActionTable;
			HealingPotionData.FailedActionRow.RowName = MVPlayerConsumableDefaultPotionFailedActionRowName;
		}
	}

	if (HealingPotionData.Icon.IsNull())
	{
		HealingPotionData.Icon = TSoftObjectPtr<UTexture2D>(
			FSoftObjectPath(MVPlayerConsumableDefaultPotionIconPath));
	}
}
}

UMVPlayerConsumable::UMVPlayerConsumable()
{
	ApplyMVPlayerConsumableHealingPotionDataRowDefaults(HealingPotionDataRow);
	HealingPotionData = MakeMVPlayerConsumableDefaultHealingPotionData();
	HealingPotionHotKeyText = FText::AsNumber(1);
	ApplyMVPlayerConsumableHealingPotionDefaults(HealingPotionData);
}

UWorld* UMVPlayerConsumable::GetWorld() const
{
	if (const AMVPlayerCharacter* PlayerCharacter = OwnerCharacter.Get())
	{
		return PlayerCharacter->GetWorld();
	}

	return GetOuter() ? GetOuter()->GetWorld() : nullptr;
}

void UMVPlayerConsumable::Initialize(AMVPlayerCharacter* InOwnerCharacter)
{
	UnbindOwnerEvents();
	OwnerCharacter = InOwnerCharacter;
	HealingPotionData = ResolveHealingPotionData();
	bHealingPotionUseActionRunning = false;
	bHealingPotionEffectApplied = false;
	BindOwnerEvents();
	BroadcastHealingPotionStateChanged();
}

void UMVPlayerConsumable::Deinitialize()
{
	UnbindOwnerEvents();
	OwnerCharacter = nullptr;
	bHealingPotionUseActionRunning = false;
	bHealingPotionEffectApplied = false;
}

bool UMVPlayerConsumable::TryUseHealingPotion()
{
	if (!CanUseHealingPotion())
	{
		const bool bPlayedFailedAction = TryPlayHealingPotionFailedAction();
		BroadcastHealingPotionStateChanged();
		return bPlayedFailedAction;
	}

	if (HasHealingPotionUseAction())
	{
		if (!TryStartHealingPotionAction(
			HealingPotionData.UseActionRow,
			HealingPotionData.UseActionStartSection))
		{
			BroadcastHealingPotionStateChanged();
			return false;
		}

		BroadcastHealingPotionStateChanged();
		return true;
	}

	return ApplyHealingPotionEffect();
}

bool UMVPlayerConsumable::ApplyHealingPotionEffect()
{
	UMVStatComponent* StatComponent = OwnerCharacter ? OwnerCharacter->StatComponent : nullptr;
	if (!StatComponent || StatComponent->IsDead())
	{
		return false;
	}

	if (HealingPotionData.HealAmount <= 0.0f
		|| (HasHealingPotionUseAction() && (!IsHealingPotionUseActionRunning() || bHealingPotionEffectApplied)))
	{
		return false;
	}

	UMVInventoryComponent* Inventory = OwnerCharacter->InventoryComponent;
	bHealingPotionEffectApplied = true;
	if (!Inventory || !Inventory->TryRemoveItem(HealingPotionData.ItemTag))
	{
		bHealingPotionEffectApplied = false;
		return false;
	}

	StatComponent->RecoverHP(HealingPotionData.HealAmount);
	return true;
}

void UMVPlayerConsumable::RestoreConsumableCountsForWorldReset()
{
	// 월드 리셋은 모든 인벤토리성 아이템이 아니라, 카운트 복구 대상 소비 아이템만 이 진입점에 모아 처리한다.
	RestoreHealingPotionCountToDefault();
}

void UMVPlayerConsumable::RestoreHealingPotionCountToDefault()
{
	bHealingPotionUseActionRunning = false;
	bHealingPotionEffectApplied = false;
	SetHealingPotionCount(HealingPotionData.DefaultCount);
}

void UMVPlayerConsumable::SetHealingPotionCount(const int32 NewCount)
{
	UMVInventoryComponent* Inventory = OwnerCharacter ? OwnerCharacter->InventoryComponent.Get() : nullptr;
	const int32 Quantity = FMath::Clamp(NewCount, 0, FMath::Max(0, HealingPotionData.MaxCarryCount));
	if (!Inventory || !Inventory->SetPotionCount(HealingPotionData.ItemTag, Quantity))
	{
		BroadcastHealingPotionStateChanged();
	}
}

FMVHealingPotionRuntimeState UMVPlayerConsumable::GetHealingPotionState() const
{
	FMVHealingPotionRuntimeState State;
	State.ItemTag = HealingPotionData.ItemTag;
	State.HealAmount = FMath::Max(0.0f, HealingPotionData.HealAmount);
	State.MaxCarryCount = FMath::Max(0, HealingPotionData.MaxCarryCount);
	State.CurrentCount = GetHealingPotionCount();
	State.bCanUse = CanUseHealingPotion();
	return State;
}

FMVQuickSlotViewData UMVPlayerConsumable::BuildHealingPotionQuickSlotViewData() const
{
	const FMVHealingPotionRuntimeState State = GetHealingPotionState();

	FMVQuickSlotViewData ViewData;
	ViewData.Icon = HealingPotionData.Icon.IsNull() ? nullptr : HealingPotionData.Icon.LoadSynchronous();
	ViewData.HotKeyText = HealingPotionHotKeyText;
	ViewData.Count = State.CurrentCount;
	ViewData.bLocked = State.CurrentCount <= 0;
	return ViewData;
}

bool UMVPlayerConsumable::IsHealingPotionUseActionRunning() const
{
	const UMVActionComponent* ActionComponent = OwnerCharacter ? OwnerCharacter->ActionComponent : nullptr;
	return bHealingPotionUseActionRunning
		|| (ActionComponent
			&& ActionComponent->IsActionRunning()
			&& IsHealingPotionUseAction(
				ActionComponent->GetActiveActionTableName(),
				ActionComponent->GetActiveActionRowName()));
}

bool UMVPlayerConsumable::TryHandleActionInput(
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

void UMVPlayerConsumable::HandleHPChanged(float /*CurrentValue*/, float /*MaxValue*/)
{
	BroadcastHealingPotionStateChanged();
}

void UMVPlayerConsumable::HandleActionStarted(const FName ActionTableName, const FName ActionRowName)
{
	bHealingPotionUseActionRunning = IsHealingPotionUseAction(ActionTableName, ActionRowName);
	bHealingPotionEffectApplied = false;
	BroadcastHealingPotionStateChanged();
}

void UMVPlayerConsumable::HandleActionEnded(
	const FName ActionTableName,
	const FName ActionRowName,
	bool /*bInterrupted*/)
{
	if (bHealingPotionUseActionRunning || IsHealingPotionUseAction(ActionTableName, ActionRowName))
	{
		bHealingPotionUseActionRunning = false;
		bHealingPotionEffectApplied = false;
		BroadcastHealingPotionStateChanged();
	}
}

void UMVPlayerConsumable::HandleInventoryChanged(const FGameplayTag ItemTag)
{
	if (!ItemTag.IsValid() && IsHealingPotionUseActionRunning())
	{
		bHealingPotionEffectApplied = true;
	}
	if (!ItemTag.IsValid() || ItemTag == HealingPotionData.ItemTag)
	{
		BroadcastHealingPotionStateChanged();
	}
}

void UMVPlayerConsumable::BindOwnerEvents()
{
	if (!OwnerCharacter)
	{
		return;
	}

	if (UMVInventoryComponent* Inventory = OwnerCharacter->InventoryComponent)
	{
		Inventory->OnInventoryChanged.AddUniqueDynamic(this, &ThisClass::HandleInventoryChanged);
	}

	if (UMVInputManagerComponent* InputManager = OwnerCharacter->InputManagerComponent)
	{
		InputManager->RegisterActionInputHandler(this, MVActionInputHandlerPriorities::Consumable);
	}

	if (UMVActionComponent* ActionComponent = OwnerCharacter->ActionComponent)
	{
		ActionComponent->OnActionStarted.RemoveDynamic(
			this,
			&UMVPlayerConsumable::HandleActionStarted);
		ActionComponent->OnActionStarted.AddUniqueDynamic(
			this,
			&UMVPlayerConsumable::HandleActionStarted);
		ActionComponent->OnActionEnded.RemoveDynamic(
			this,
			&UMVPlayerConsumable::HandleActionEnded);
		ActionComponent->OnActionEnded.AddUniqueDynamic(
			this,
			&UMVPlayerConsumable::HandleActionEnded);
	}

	if (UMVStatComponent* StatComponent = OwnerCharacter->StatComponent)
	{
		StatComponent->OnHPChanged.RemoveDynamic(
			this,
			&UMVPlayerConsumable::HandleHPChanged);
		StatComponent->OnHPChanged.AddUniqueDynamic(
			this,
			&UMVPlayerConsumable::HandleHPChanged);
	}
}

void UMVPlayerConsumable::UnbindOwnerEvents()
{
	if (!OwnerCharacter)
	{
		return;
	}

	if (UMVInventoryComponent* Inventory = OwnerCharacter->InventoryComponent)
	{
		Inventory->OnInventoryChanged.RemoveDynamic(this, &ThisClass::HandleInventoryChanged);
	}

	if (UMVInputManagerComponent* InputManager = OwnerCharacter->InputManagerComponent)
	{
		InputManager->UnregisterActionInputHandler(this);
	}

	if (UMVActionComponent* ActionComponent = OwnerCharacter->ActionComponent)
	{
		ActionComponent->OnActionStarted.RemoveDynamic(
			this,
			&UMVPlayerConsumable::HandleActionStarted);
		ActionComponent->OnActionEnded.RemoveDynamic(
			this,
			&UMVPlayerConsumable::HandleActionEnded);
	}

	if (UMVStatComponent* StatComponent = OwnerCharacter->StatComponent)
	{
		StatComponent->OnHPChanged.RemoveDynamic(
			this,
			&UMVPlayerConsumable::HandleHPChanged);
	}
}

void UMVPlayerConsumable::GetDefaultHealingPotionStack(FGameplayTag& ItemTag, int32& Count) const
{
	const FMVHealingPotionTableRow Row = ResolveHealingPotionData();
	ItemTag = Row.ItemTag;
	Count = FMath::Clamp(Row.DefaultCount, 0, FMath::Max(0, Row.MaxCarryCount));
}

int32 UMVPlayerConsumable::GetHealingPotionCount() const
{
	const UMVInventoryComponent* Inventory = OwnerCharacter ? OwnerCharacter->InventoryComponent.Get() : nullptr;
	return Inventory ? Inventory->GetItemCount(HealingPotionData.ItemTag) : 0;
}

FMVHealingPotionTableRow UMVPlayerConsumable::ResolveHealingPotionData() const
{
	FDataTableRowHandle Handle = HealingPotionDataRow;
	ApplyMVPlayerConsumableHealingPotionDataRowDefaults(Handle);
	const FMVHealingPotionTableRow* Row = nullptr;
	const UDataTable* DataTable = Handle.DataTable;
	if (DataTable && !Handle.RowName.IsNone())
	{
		Row = Handle.GetRow<FMVHealingPotionTableRow>(TEXT("MVPlayerConsumable"));
	}

	FMVHealingPotionTableRow Data = Row ? *Row : MakeMVPlayerConsumableDefaultHealingPotionData();
	ApplyMVPlayerConsumableHealingPotionDefaults(Data);
	return Data;
}

bool UMVPlayerConsumable::CanUseHealingPotion() const
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

	return GetHealingPotionCount() > 0 && HealingPotionData.HealAmount > 0.0f;
}

bool UMVPlayerConsumable::HasHealingPotionUseAction() const
{
	return HealingPotionData.UseActionRow.DataTable && !HealingPotionData.UseActionRow.RowName.IsNone();
}

bool UMVPlayerConsumable::HasHealingPotionFailedAction() const
{
	return HealingPotionData.FailedActionRow.DataTable && !HealingPotionData.FailedActionRow.RowName.IsNone();
}

bool UMVPlayerConsumable::CanPlayHealingPotionFailedAction() const
{
	const UMVStatComponent* StatComponent = OwnerCharacter ? OwnerCharacter->StatComponent : nullptr;
	if (!StatComponent || StatComponent->IsDead())
	{
		return false;
	}

	if (GetHealingPotionCount() > 0 || !HasHealingPotionFailedAction())
	{
		return false;
	}

	const UMVActionComponent* ActionComponent = OwnerCharacter ? OwnerCharacter->ActionComponent : nullptr;
	if (!ActionComponent || !ActionComponent->IsActionRunning())
	{
		return true;
	}

	const FName ActiveActionTableName = ActionComponent->GetActiveActionTableName();
	const FName ActiveActionRowName = ActionComponent->GetActiveActionRowName();
	if (IsHealingPotionUseAction(ActiveActionTableName, ActiveActionRowName)
		|| IsHealingPotionFailedAction(ActiveActionTableName, ActiveActionRowName))
	{
		return false;
	}

	return CanTransitionActiveActionForHealingPotion();
}

bool UMVPlayerConsumable::CanTransitionActiveActionForHealingPotion() const
{
	const UMVActionComponent* ActionComponent = OwnerCharacter ? OwnerCharacter->ActionComponent : nullptr;
	if (!ActionComponent || !ActionComponent->IsActionRunning() || !ActionComponent->CanInterruptActiveAction())
	{
		return false;
	}

	const UMVInputManagerComponent* InputManager = OwnerCharacter ? OwnerCharacter->InputManagerComponent : nullptr;
	return InputManager && InputManager->IsRecoveryEscapeWindowOpen();
}

bool UMVPlayerConsumable::TryStartHealingPotionAction(
	const FDataTableRowHandle& ActionRow,
	const FName StartSection)
{
	UMVActionComponent* ActionComponent = OwnerCharacter ? OwnerCharacter->ActionComponent : nullptr;
	if (!ActionComponent || !ActionRow.DataTable || ActionRow.RowName.IsNone())
	{
		return false;
	}

	const bool bShouldTransition = CanTransitionActiveActionForHealingPotion();
	return bShouldTransition
		? ActionComponent->TryTransitionActionFromRowHandle(ActionRow, StartSection)
		: ActionComponent->TryStartActionFromRowHandle(ActionRow, StartSection);
}

bool UMVPlayerConsumable::TryPlayHealingPotionFailedAction()
{
	return CanPlayHealingPotionFailedAction()
		&& TryStartHealingPotionAction(
			HealingPotionData.FailedActionRow,
			HealingPotionData.FailedActionStartSection);
}

bool UMVPlayerConsumable::IsHealingPotionUseAction(
	const FName ActionTableName,
	const FName ActionRowName) const
{
	return HasHealingPotionUseAction()
		&& ActionTableName == ResolveHealingPotionActionTableName(HealingPotionData.UseActionRow)
		&& ActionRowName == HealingPotionData.UseActionRow.RowName;
}

bool UMVPlayerConsumable::IsHealingPotionFailedAction(
	const FName ActionTableName,
	const FName ActionRowName) const
{
	return HasHealingPotionFailedAction()
		&& ActionTableName == ResolveHealingPotionActionTableName(HealingPotionData.FailedActionRow)
		&& ActionRowName == HealingPotionData.FailedActionRow.RowName;
}

FName UMVPlayerConsumable::ResolveHealingPotionActionTableName(
	const FDataTableRowHandle& ActionRow) const
{
	const UDataTable* DataTable = ActionRow.DataTable;
	if (!DataTable)
	{
		return NAME_None;
	}

	FString TableName = DataTable->GetName();
	TableName.RemoveFromStart(TEXT("DT_"));
	return FName(*TableName);
}

void UMVPlayerConsumable::BroadcastHealingPotionStateChanged()
{
	OnHealingPotionStateChanged.Broadcast(GetHealingPotionState());
}
