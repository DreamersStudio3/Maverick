#include "UI/HUD/MVPlayerSkillHUDWidget.h"

#include "Blueprint/WidgetTree.h"
#include "CommonInputSubsystem.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/MVCombatComponent.h"
#include "Components/MVWeaponComponent.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedPlayerInput.h"
#include "Engine/LocalPlayer.h"
#include "Engine/Texture2D.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "UI/HUD/MVSkillHUDPresentation.h"
#include "UI/HUD/MVSkillSlotWidget.h"
#include "Weapon/Skill/MVSkillSelectionData.h"

UMVPlayerSkillHUDWidget::UMVPlayerSkillHUDWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	FallbackSkillIcon = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/UI/Textures/SkillSelection/State/T_UI_Unknown.T_UI_Unknown")));
	const auto AddBinding = [this](EMVSkillSlot InSkillSlot, const TCHAR* ActionName, bool bModifier)
	{
		auto& Binding = SkillInputBindings.AddDefaulted_GetRef();
		Binding.Slot = InSkillSlot;
		Binding.Action = TSoftObjectPtr<UInputAction>(FSoftObjectPath(FString::Printf(TEXT("/Game/Input/Actions/IA_%s.IA_%s"), ActionName, ActionName)));
		if (bModifier) Binding.Modifier = TSoftObjectPtr<UInputAction>(FSoftObjectPath(TEXT("/Game/Input/Actions/IA_SkillModifier.IA_SkillModifier")));
	};
	AddBinding(EMVSkillSlot::QSkill, TEXT("SkillQ"), false);
	AddBinding(EMVSkillSlot::WSkill, TEXT("LightAttack"), true);
	AddBinding(EMVSkillSlot::ESkill, TEXT("HeavyChargeAttack"), true);
	AddBinding(EMVSkillSlot::RSkill, TEXT("SkillR"), false);
}

UMVSkillSlotWidget* UMVPlayerSkillHUDWidget::GetSkillSlot(const EMVSkillSlot InSkillSlot) const
{
	switch (InSkillSlot)
	{
	case EMVSkillSlot::QSkill: return QSkillSlot;
	case EMVSkillSlot::WSkill: return WSkillSlot;
	case EMVSkillSlot::ESkill: return ESkillSlot;
	case EMVSkillSlot::RSkill: return RSkillSlot;
	default: return nullptr;
	}
}

void UMVPlayerSkillHUDWidget::BindToCombatComponent(UMVCombatComponent* InCombatComponent)
{
	if (BoundCombatComponent.Get() == InCombatComponent)
	{
		RefreshSkillSlots();
		return;
	}
	UnbindCombatComponent();
	BoundCombatComponent = InCombatComponent;
	if (InCombatComponent && InCombatComponent->GetOwner())
	{
		BoundWeaponComponent = InCombatComponent->GetOwner()->FindComponentByClass<UMVWeaponComponent>();
		if (auto* Weapons = BoundWeaponComponent.Get())
		{
			SelectionChangedHandle = Weapons->OnSkillSelectionChanged.AddUObject(this, &ThisClass::HandleSelectionChanged);
			Weapons->OnEquippedWeaponChanged.AddUniqueDynamic(this, &ThisClass::HandleWeaponChanged);
		}
	}
	RefreshPresentation();
	RefreshInputHints();
	RefreshSkillSlots();
}

void UMVPlayerSkillHUDWidget::UnbindCombatComponent()
{
	if (auto* Weapons = BoundWeaponComponent.Get())
	{
		Weapons->OnSkillSelectionChanged.Remove(SelectionChangedHandle);
		Weapons->OnEquippedWeaponChanged.RemoveDynamic(this, &ThisClass::HandleWeaponChanged);
	}
	SelectionChangedHandle.Reset();
	BoundWeaponComponent.Reset();
	BoundCombatComponent.Reset();
	RefreshPresentation();
	RefreshSkillSlots();
}

void UMVPlayerSkillHUDWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	BuildNativeWidgetTree();
	for (const auto& Definition : MVSkillSelectionData::GetSlots())
		if (auto* Widget = GetSkillSlot(Definition.Slot))
		{
			Widget->SetSlotSize(SkillSlotSize);
			Widget->SetSlotLabel(Definition.DisplayName);
		}
	if (QChainTimer)
	{
		QChainTimer->SetHotKeyText(FText::GetEmpty());
		QChainTimer->SetSlotSize(SkillSlotSize * ChainTimerScale);
	}
	RefreshPresentation();
	RefreshInputHints();
	RefreshSkillSlots();
}

void UMVPlayerSkillHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	BindInputNotifications();
	if (auto* Controller = GetOwningPlayer()) Controller->OnPossessedPawnChanged.AddUniqueDynamic(this, &ThisClass::HandlePawnChanged);
	APawn* Pawn = GetOwningPlayerPawn();
	BindToCombatComponent(Pawn ? Pawn->FindComponentByClass<UMVCombatComponent>() : nullptr);
}

void UMVPlayerSkillHUDWidget::NativeDestruct()
{
	if (auto* Controller = GetOwningPlayer()) Controller->OnPossessedPawnChanged.RemoveDynamic(this, &ThisClass::HandlePawnChanged);
	UnbindInputNotifications();
	UnbindCombatComponent();
	Super::NativeDestruct();
}

void UMVPlayerSkillHUDWidget::HandlePawnChanged(APawn* Previous, APawn* Next)
{
	BindToCombatComponent(Next ? Next->FindComponentByClass<UMVCombatComponent>() : nullptr);
}

void UMVPlayerSkillHUDWidget::NativeTick(const FGeometry& MyGeometry, const float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	// 무기 알림 뒤에도 구성이 갱신될 수 있으므로 스타일 변경만 가볍게 대조한다.
	if (BoundCombatComponent.IsValid() && DisplayedStyle != BoundCombatComponent->CurrentWeaponStyle) RefreshPresentation();
	RefreshSkillSlots();
}

bool UMVPlayerSkillHUDWidget::BuildNativeWidgetTree()
{
	if (!WidgetTree || WidgetTree->RootWidget) return false;
	auto* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("SkillRow"));
	WidgetTree->RootWidget = Row;
	for (const auto& Definition : MVSkillSelectionData::GetSlots())
	{
		if (!MVSkillSelectionData::IsActiveSlot(Definition.Slot)) continue;
		const FName Name(*FString::Printf(TEXT("%sSkillSlot"), *Definition.DisplayName.ToString()));
		auto* Widget = WidgetTree->ConstructWidget<UMVSkillSlotWidget>(UMVSkillSlotWidget::StaticClass(), Name);
		UWidget* Column = Widget;
		switch (Definition.Slot)
		{
		case EMVSkillSlot::QSkill:
		{
			QSkillSlot = Widget;
			auto* QColumn = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("QSkillColumn"));
			QChainTimer = WidgetTree->ConstructWidget<UMVSkillSlotWidget>(UMVSkillSlotWidget::StaticClass(), TEXT("QChainTimer"));
			auto* TimerSlot = QColumn->AddChildToVerticalBox(QChainTimer);
			TimerSlot->SetHorizontalAlignment(HAlign_Left);
			TimerSlot->SetPadding(FMargin(0, 0, 0, ChainTimerGap));
			QColumn->AddChildToVerticalBox(Widget);
			Column = QColumn;
			break;
		}
		case EMVSkillSlot::WSkill: WSkillSlot = Widget; break;
		case EMVSkillSlot::ESkill: ESkillSlot = Widget; break;
		case EMVSkillSlot::RSkill: RSkillSlot = Widget; break;
		default: break;
		}
		auto* ColumnSlot = Row->AddChildToHorizontalBox(Column);
		ColumnSlot->SetVerticalAlignment(VAlign_Bottom);
		ColumnSlot->SetPadding(FMargin(SkillSlotGap * 0.5f, 0, SkillSlotGap * 0.5f, 0));
	}
	return true;
}

void UMVPlayerSkillHUDWidget::HandleSelectionChanged(const FMVSkillSelectionContext& Scope)
{
	if (BoundCombatComponent.IsValid() && Scope.Style == BoundCombatComponent->CurrentWeaponStyle
		&& MVSkillSelectionData::IsActiveSlot(Scope.Slot)) RefreshPresentation();
}

void UMVPlayerSkillHUDWidget::HandleWeaponChanged(const FMVEquippedWeaponState& State)
{
	RefreshPresentation();
	RefreshSkillSlots();
}

void UMVPlayerSkillHUDWidget::RefreshPresentation()
{
	DisplayedStyle = BoundCombatComponent.IsValid() ? BoundCombatComponent->CurrentWeaponStyle : EMVEquippedStyle::None;
	UTexture2D* Fallback = FallbackSkillIcon.LoadSynchronous();
	for (const auto& Definition : MVSkillSelectionData::GetSlots())
	{
		auto* Widget = GetSkillSlot(Definition.Slot);
		if (!Widget) continue;
		FDataTableRowHandle Selected;
		if (BoundWeaponComponent.IsValid() && DisplayedStyle != EMVEquippedStyle::None)
			BoundWeaponComponent->GetEquippedSkillRow(DisplayedStyle, Definition.Slot, Selected);
		const auto Presentation = MVSkillHUDPresentation::Build(Selected, FallbackSkillIcon);
		TArray<UTexture2D*> Icons;
		for (const auto& Icon : Presentation.Icons)
		{
			auto* Texture = Icon.LoadSynchronous();
			Icons.Add(Texture ? Texture : Fallback);
		}
		Widget->SetIconStack(Icons);
		Widget->SetStatusText(Presentation.Status);
		Widget->SetToolTipText(FText::Format(NSLOCTEXT("SkillHUD", "Tooltip", "{0}\n{1}"), Presentation.Name, Presentation.Status));
		if (Definition.Slot == EMVSkillSlot::QSkill && QChainTimer) QChainTimer->SetIconStack(Icons);
	}
}

void UMVPlayerSkillHUDWidget::BindInputNotifications()
{
	UnbindInputNotifications();
	if (auto* Local = GetOwningLocalPlayer())
	{
		BoundInputSubsystem = Local->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
		if (auto* Input = BoundInputSubsystem.Get()) Input->ControlMappingsRebuiltDelegate.AddUniqueDynamic(this, &ThisClass::RefreshInputHints);
		if (auto* Common = UCommonInputSubsystem::Get(Local))
			Common->OnInputMethodChangedNative.AddWeakLambda(this, [this](ECommonInputType) { RefreshInputHints(); });
	}
	RefreshInputHints();
}

void UMVPlayerSkillHUDWidget::UnbindInputNotifications()
{
	if (auto* Input = BoundInputSubsystem.Get()) Input->ControlMappingsRebuiltDelegate.RemoveDynamic(this, &ThisClass::RefreshInputHints);
	BoundInputSubsystem.Reset();
	if (auto* Common = UCommonInputSubsystem::Get(GetOwningLocalPlayer())) Common->OnInputMethodChangedNative.RemoveAll(this);
}

void UMVPlayerSkillHUDWidget::RefreshInputHints()
{
	const auto* Input = BoundInputSubsystem.Get();
	const auto* Common = UCommonInputSubsystem::Get(GetOwningLocalPlayer());
	const auto Type = Common ? Common->GetCurrentInputType() : ECommonInputType::MouseAndKeyboard;
	for (const auto& Definition : MVSkillSelectionData::GetSlots())
	{
		auto* Widget = GetSkillSlot(Definition.Slot);
		if (!Widget) continue;
		const auto* Binding = SkillInputBindings.FindByPredicate([&Definition](const auto& Value) { return Value.Slot == Definition.Slot; });
		const auto* PlayerInput = Input ? Input->GetPlayerInput() : nullptr;
		const auto Mappings = PlayerInput ? PlayerInput->GetEnhancedActionMappingsView() : TConstArrayView<const FEnhancedActionKeyMapping>();
		const auto* Modifier = Binding ? Binding->Modifier.LoadSynchronous() : nullptr;
		if (Binding && !Binding->Modifier.IsNull() && !Modifier)
		{
			Widget->SetHotKeyText(NSLOCTEXT("SkillHUD", "Unmapped", "미지정"));
			continue;
		}
		Widget->SetHotKeyText(MVSkillHUDPresentation::MakeMappedKeyLabel(Mappings,
			Binding ? Binding->Action.LoadSynchronous() : nullptr, Modifier,
			Type == ECommonInputType::Gamepad, Type == ECommonInputType::Touch));
	}
}

void UMVPlayerSkillHUDWidget::RefreshSkillSlots()
{
	for (const auto& Definition : MVSkillSelectionData::GetSlots())
	{
		auto* Widget = GetSkillSlot(Definition.Slot);
		if (!Widget) continue;
		FMVSkillSlotRuntimeState State;
		const bool bAvailable = BoundCombatComponent.IsValid() && BoundCombatComponent->GetSkillSlotRuntimeState(Definition.Slot, State);
		if (bAvailable && State.bGaugeControlled) Widget->SetGaugeRuntimeState(State.ActiveStackIndex, State.GaugeRatio, State.bLoadoutAvailable);
		else Widget->SetRuntimeState(State.ActiveStackIndex, bAvailable ? State.CooldownRemaining : 0.0f, State.CooldownDuration, bAvailable && State.bLoadoutAvailable);
		if (Definition.Slot == EMVSkillSlot::QSkill && QChainTimer)
		{
			QChainTimer->SetVisibility(bAvailable && State.bChainTimerVisible && State.ChainWindowRemaining > KINDA_SMALL_NUMBER
				? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
			QChainTimer->SetRuntimeState(State.ChainTimerIconIndex, State.ChainWindowRemaining, State.ChainWindowDuration, bAvailable);
		}
	}
}
