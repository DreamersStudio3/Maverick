#include "Character/PC/Consumable/MVPlayerConsumableComponent.h"

#include "Character/PC/MVPlayerCharacter.h"
#include "Components/MVActionComponent.h"
#include "Components/MVInputManagerComponent.h"
#include "Components/MVStatComponent.h"
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
		if (!ActionComponent
			|| !ActionComponent->TryStartActionFromRowHandle(
				HealingPotionData.UseActionRow,
				HealingPotionData.UseActionStartSection))
		{
			BroadcastHealingPotionStateChanged();
			return false;
		}
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
		|| HealingPotionState.HealAmount <= 0.0f
		|| StatComponent->CurrentHP >= StatComponent->MaxHP)
	{
		BroadcastHealingPotionStateChanged();
		return false;
	}

	StatComponent->RecoverHP(HealingPotionState.HealAmount);
	SetHealingPotionCount(HealingPotionState.CurrentCount - 1);
	return true;
}

void UMVPlayerConsumableComponent::RestoreHealingPotionCountForWorldReset()
{
	const int32 MaxCarryCount = FMath::Max(0, HealingPotionData.MaxCarryCount);
	const int32 DefaultCount = FMath::Clamp(HealingPotionData.DefaultCount, 0, MaxCarryCount);
	HealingPotionState.CurrentCount = DefaultCount;
	HealingPotionState.MaxCarryCount = MaxCarryCount;
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

void UMVPlayerConsumableComponent::HandleActionInputSubmitted(
	const int32 ActionId,
	const FVector2D /*ControllerSpaceInput*/,
	const bool /*bHasMovementInput*/)
{
	if (ActionId == MVActionIds::UseConsumable)
	{
		TryUseHealingPotion();
	}
}

void UMVPlayerConsumableComponent::HandleHPChanged(float /*CurrentValue*/, float /*MaxValue*/)
{
	BroadcastHealingPotionStateChanged();
}

void UMVPlayerConsumableComponent::BindOwnerEvents()
{
	if (!OwnerCharacter)
	{
		return;
	}

	if (UMVInputManagerComponent* InputManager = OwnerCharacter->InputManagerComponent)
	{
		InputManager->OnActionInputSubmitted.RemoveDynamic(
			this,
			&UMVPlayerConsumableComponent::HandleActionInputSubmitted);
		InputManager->OnActionInputSubmitted.AddUniqueDynamic(
			this,
			&UMVPlayerConsumableComponent::HandleActionInputSubmitted);
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
		InputManager->OnActionInputSubmitted.RemoveDynamic(
			this,
			&UMVPlayerConsumableComponent::HandleActionInputSubmitted);
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
		return false;
	}

	return HealingPotionState.CurrentCount > 0
		&& HealingPotionState.HealAmount > 0.0f
		&& StatComponent->CurrentHP < StatComponent->MaxHP;
}

void UMVPlayerConsumableComponent::BroadcastHealingPotionStateChanged()
{
	HealingPotionState.bCanUse = CanUseHealingPotion();
	OnHealingPotionStateChanged.Broadcast(HealingPotionState);
}
