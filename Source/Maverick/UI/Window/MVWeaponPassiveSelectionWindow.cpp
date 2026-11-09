#include "UI/Window/MVWeaponPassiveSelectionWindow.h"

#include "Character/PC/MVPlayerCharacter.h"
#include "Combat/MVAbilityBase.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/MVActionComponent.h"
#include "Components/MVCombatComponent.h"
#include "Components/MVStatComponent.h"
#include "Components/MVWeaponComponent.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerController.h"
#include "Tables/MVSkillDataTableColumn.h"
#include "UI/Base/MVHUDWidgetBase.h"
#include "UI/System/MVUISubsystem.h"
#include "Engine/GameInstance.h"
#include "CommonActionWidget.h"
#include "ICommonInputModule.h"
#include "Input/CommonUIInputTypes.h"

#define LOCTEXT_NAMESPACE "MVWeaponPassiveSelection"

namespace
{
FText MVPassiveUIName(const FMVSkillDataTableColumn& Skill)
{
	return Skill.DisplayName.IsEmpty() ? LOCTEXT("Unnamed", "이름 없는 패시브") : Skill.DisplayName;
}

bool MVPassiveUIIsActive(const UMVCombatComponent& Combat, const FDataTableRowHandle& Row)
{
	const UMVAbilityBase* Active = Combat.GetActivePassiveSkill();
	return IsValid(Active) && Active->bAbilityActive && Combat.GetActivePassiveSkillRowHandle() == Row;
}

const FMVWeaponTableRow* MVPassiveUIFindWeapon(const TMap<FGameplayTag, FDataTableRowHandle>& Owned,
	const UMVWeaponComponent& Weapons, const FMVEquippedWeaponState& Equipped, FGameplayTag ItemTag)
{
	if (const auto* Handle = Owned.Find(ItemTag))
	{
		return Handle->GetRow<FMVWeaponTableRow>(TEXT("PassiveSelection.OwnedWeapon"));
	}
	if (Equipped.bValid && Equipped.EquippedStyle == EMVEquippedStyle::BareHand
		&& Equipped.WeaponTypeTag.IsValid() && Equipped.ItemTag == ItemTag)
	{
		return &Weapons.BareHandWeapon;
	}
	return nullptr;
}

void MVPassiveUISetArtwork(UImage& Image, const TSoftObjectPtr<UTexture2D>& Artwork, UTexture2D* Fallback)
{
	// 이전 비동기 요청을 취소하고 새 그림이 준비될 때까지 대체 그림을 표시한다.
	Image.SetBrushFromTexture(Fallback, true);
	if (!Artwork.IsNull())
	{
		Image.SetBrushFromSoftTexture(Artwork, true);
	}
	Image.SetVisibility(Artwork.IsNull() && !Fallback ? ESlateVisibility::Hidden : ESlateVisibility::HitTestInvisible);
}
}

void UMVPassiveCandidateEntryWidget::SetWeapon(const FMVWeaponTableRow& Weapon, bool bEquipped, UTexture2D* MissingArtwork)
{
	CandidateName->SetText(Weapon.DisplayName.IsEmpty() ? FText::FromName(Weapon.ItemTag.GetTagName()) : Weapon.DisplayName);
	CandidateState->SetText(bEquipped ? LOCTEXT("EquippedWeapon", "장착 중") : FText::GetEmpty());
	MVPassiveUISetArtwork(*CandidateIcon, Weapon.Icon, MissingArtwork);
	SetPreviewed(false);
}

void UMVPassiveCandidateEntryWidget::SetCandidate(const FDataTableRowHandle& InHandle,
	const FMVSkillDataTableColumn& Data, const FText& State, bool bInCanApply, UTexture2D* MissingArtwork)
{
	SkillRow = InHandle;
	bClearSelection = false;
	bCanApply = bInCanApply;
	CandidateName->SetText(MVPassiveUIName(Data));
	CandidateState->SetText(State);
	MVPassiveUISetArtwork(*CandidateIcon, Data.Icon, MissingArtwork);
	SetPreviewed(false);
}

void UMVPassiveCandidateEntryWidget::SetClearSelection(const FDataTableRowHandle& Selected)
{
	SkillRow = Selected;
	bClearSelection = true;
	bCanApply = !Selected.IsNull();
	CandidateName->SetText(LOCTEXT("ClearSelection", "선택 해제"));
	CandidateState->SetText(bCanApply ? FText::GetEmpty() : LOCTEXT("NoSelection", "미선택"));
	MVPassiveUISetArtwork(*CandidateIcon, {}, nullptr);
	SetPreviewed(false);
}

void UMVPassiveCandidateEntryWidget::SetPreviewed(bool bPreviewed)
{
	if (PreviewMarker)
	{
		PreviewMarker->SetVisibility(bPreviewed ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
	}
}

bool UMVWeaponPassiveSelectionWindow::CanOpenForPlayer(const APlayerController* Controller)
{
	const auto* Player = IsValid(Controller) && Controller->IsLocalController()
		? Cast<AMVPlayerCharacter>(Controller->GetPawn()) : nullptr;
	const auto* Transition = UMVFieldTransitionSubsystem::Get(Player);
	return IsValid(Player) && Player->WeaponComponent && Player->CombatComponent
		&& Player->StatComponent && Player->StatComponent->IsBaseStatsReady() && !Player->StatComponent->IsDead()
		&& Player->ActionComponent && !Player->ActionComponent->IsActionRunning()
		&& Transition && !Transition->IsTransitionRunning();
}

void UMVWeaponPassiveSelectionWindow::NativeOnActivated()
{
	Super::NativeOnActivated();
	APlayerController* Controller = GetOwningPlayer();
	if (!CanOpenForPlayer(Controller))
	{
		DeactivateWidget();
		return;
	}
	if (!CandidateEntryClass || !WeaponList || !CandidateList || !WeaponName || !PreviewName || !PreviewDescription || !PreviewImage || !ApplyButton || !CloseButton)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: assign CandidateEntryClass and all required designer widgets."), *GetName());
		DeactivateWidget();
		return;
	}

	// 빈 영역 클릭도 창으로 전달해 게임 화면으로 초점이 빠지지 않게 한다.
	SetVisibility(ESlateVisibility::Visible);
	AMVPlayerCharacter* Player = CastChecked<AMVPlayerCharacter>(Controller->GetPawn());
	auto* Instance = GetGameInstance();
	if (auto* UI = Instance ? Instance->GetSubsystem<UMVUISubsystem>() : nullptr)
	{
		if (auto* HUD = UI->GetMainHUD())
		{
			SuppressedHUD = HUD;
			PreviousHUDVisibility = HUD->GetVisibility();
			HUD->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
	ViewedPlayer = Player;
	Player->WeaponComponent->OnEquippedWeaponChanged.AddUniqueDynamic(this, &ThisClass::OnEquippedWeaponChanged);
	Player->WeaponComponent->OnOwnedWeaponsChanged.AddUniqueDynamic(this, &ThisClass::QueueRefresh);
	Player->WeaponComponent->OnPassiveOptionsChanged.AddUniqueDynamic(this, &ThisClass::QueueRefresh);
	Player->CombatComponent->OnActivePassiveChanged.AddUniqueDynamic(this, &ThisClass::QueueRefresh);
	Player->ActionComponent->OnActionStarted.AddUniqueDynamic(this, &ThisClass::OnPlayerActionStarted);
	Player->StatComponent->OnDamageApplied.AddUniqueDynamic(this, &ThisClass::OnPlayerDamageApplied);
	Player->StatComponent->OnDead.AddUniqueDynamic(this, &ThisClass::DeactivateWidget);
	Controller->OnPossessedPawnChanged.AddUniqueDynamic(this, &ThisClass::OnPawnChanged);
	UMVFieldTransitionSubsystem::Get(Player)->OnFieldTransitionPhaseChanged.AddUniqueDynamic(this, &ThisClass::OnTransitionChanged);
	CloseButton->OnClicked.AddUniqueDynamic(this, &ThisClass::DeactivateWidgetWithFade);
	ApplyButton->OnClicked.AddUniqueDynamic(this, &ThisClass::ApplyPreviewedCandidate);
	RegisterConfirmInput();
	SetRequestMessage(FText::GetEmpty());
	RefreshWeapons();
}

void UMVWeaponPassiveSelectionWindow::OnEquippedWeaponChanged(const FMVEquippedWeaponState& Weapon)
{
	QueueRefresh();
}

void UMVWeaponPassiveSelectionWindow::RefreshWeapons()
{
	AMVPlayerCharacter* Player = ViewedPlayer.Get();
	if (!Player || !IsActivated())
	{
		return;
	}
	if (auto* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RefreshTimer);
	}
	SetRequestMessage(FText::GetEmpty());
	const UMVWeaponComponent& Weapons = *Player->WeaponComponent;
	const FMVEquippedWeaponState Equipped = Weapons.GetEquippedWeaponState();
	const auto OwnedWeapons = Weapons.GetOwnedWeapons();
	TArray<FGameplayTag> Items;
	OwnedWeapons.GetKeys(Items);
	Items.Sort([](const FGameplayTag& A, const FGameplayTag& B)
	{
		return A.GetTagName().LexicalLess(B.GetTagName());
	});
	if (Equipped.bValid && MVPassiveUIFindWeapon(OwnedWeapons, Weapons, Equipped, Equipped.ItemTag))
	{
		Items.Remove(Equipped.ItemTag);
		Items.Insert(Equipped.ItemTag, 0);
	}
	const FGameplayTag Preferred = ViewedWeaponEntry.IsValid() ? ViewedItemTag : Equipped.ItemTag;
	ViewedWeaponEntry.Reset();
	WeaponList->ClearChildren();
	UMVPassiveCandidateEntryWidget* Initial = nullptr;
	for (const FGameplayTag& Tag : Items)
	{
		const auto* Item = MVPassiveUIFindWeapon(OwnedWeapons, Weapons, Equipped, Tag);
		if (!Item)
		{
			continue;
		}
		auto* Entry = CreateWidget<UMVPassiveCandidateEntryWidget>(GetOwningPlayer(), CandidateEntryClass);
		if (!Entry)
		{
			continue;
		}
		Entry->SetWeapon(*Item, Equipped.bValid && Tag == Equipped.ItemTag, MissingArtwork);
		WeaponList->AddChild(Entry);
		Entry->OnClicked().AddUObject(this, &ThisClass::BrowseWeapon, Entry, Tag, Item->WeaponTypeTag, true);
		Entry->OnFocusReceived().AddUObject(this, &ThisClass::BrowseWeapon, Entry, Tag, Item->WeaponTypeTag, false);
		if (!Initial || Tag == Preferred)
		{
			Initial = Entry;
			ViewedItemTag = Tag;
			ViewedWeaponType = Item->WeaponTypeTag;
		}
	}
	WeaponName->SetVisibility(Initial ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	if (Initial)
	{
		BrowseWeapon(Initial, ViewedItemTag, ViewedWeaponType, true);
	}
	else
	{
		ViewedItemTag = {};
		ViewedWeaponType = {};
		RebuildCandidates();
	}
}

void UMVWeaponPassiveSelectionWindow::BrowseWeapon(UMVPassiveCandidateEntryWidget* Entry, FGameplayTag ItemTag, FGameplayTag WeaponType, bool bFocusCandidates)
{
	if (!IsActivated() || IsFadingOut() || !IsValid(Entry) || Entry->GetParent() != WeaponList)
	{
		return;
	}
	if (ViewedWeaponEntry.Get() == Entry)
	{
		if (bFocusCandidates)
		{
			NativeGetDesiredFocusTarget()->SetUserFocus(GetOwningPlayer());
		}
		return;
	}
	if (auto* Previous = ViewedWeaponEntry.Get())
	{
		Previous->SetPreviewed(false);
	}
	ViewedWeaponEntry = Entry;
	ViewedItemTag = ItemTag;
	ViewedWeaponType = WeaponType;
	Entry->SetPreviewed(true);
	WeaponList->ScrollWidgetIntoView(Entry, false);
	SetRequestMessage(FText::GetEmpty());
	RebuildCandidates(bFocusCandidates);
}

void UMVWeaponPassiveSelectionWindow::RebuildCandidates(bool bFocusCandidates)
{
	AMVPlayerCharacter* Player = ViewedPlayer.Get();
	if (!Player || !IsActivated())
	{
		return;
	}
	PreviewedEntry.Reset();
	ApplyButton->SetIsEnabled(false);
	CandidateList->ClearChildren();
	const UMVWeaponComponent& Weapons = *Player->WeaponComponent;
	const auto Equipped = Weapons.GetEquippedWeaponState();
	const bool bCurrentType = Equipped.bValid && Equipped.WeaponTypeTag == ViewedWeaponType;
	FDataTableRowHandle Selected;
	const bool bHasSelection = Weapons.GetSelectedPassive(ViewedWeaponType, Selected);
	const UMVCombatComponent& Combat = *Player->CombatComponent;
	UMVPassiveCandidateEntryWidget* Initial = nullptr;
	if (ViewedWeaponType.IsValid())
	{
		for (const auto& Candidate : Weapons.GetPassiveCandidates(ViewedWeaponType))
		{
			const auto* Skill = Candidate.SkillRowHandle.GetRow<FMVSkillDataTableColumn>(TEXT("PassiveSelection.Candidate"));
			if (!Skill)
			{
				continue;
			}
			auto* Entry = CreateWidget<UMVPassiveCandidateEntryWidget>(GetOwningPlayer(), CandidateEntryClass);
			if (!Entry)
			{
				continue;
			}
			const bool bSelected = Candidate.SkillRowHandle == Selected;
			const bool bEquipped = bSelected && bCurrentType && MVPassiveUIIsActive(Combat, Selected);
			FText State;
			if (bEquipped)
			{
				State = LOCTEXT("Equipped", "장착 중");
			}
			else if (!Candidate.bUnlocked)
			{
				State = LOCTEXT("Locked", "잠김");
			}
			else if (bSelected)
			{
				State = bCurrentType ? LOCTEXT("SelectedOnly", "선택됨 · 미적용")
					: LOCTEXT("SelectedForLater", "선택됨 · 해당 무기 장착 시 적용");
			}
			const bool bCanApply = Candidate.bUnlocked && !bEquipped && !(bSelected && !bCurrentType);
			Entry->SetCandidate(Candidate.SkillRowHandle, *Skill, State, bCanApply, MissingArtwork);
			AddEntry(Entry);
			if (!Initial || bSelected)
			{
				Initial = Entry;
			}
		}
		if (Initial || bHasSelection)
		{
			if (auto* ClearEntry = CreateWidget<UMVPassiveCandidateEntryWidget>(GetOwningPlayer(), CandidateEntryClass))
			{
				ClearEntry->SetClearSelection(Selected);
				AddEntry(ClearEntry);
				if (!Initial)
				{
					Initial = ClearEntry;
				}
			}
		}
	}
	if (Initial)
	{
		PreviewCandidate(Initial);
		if (bFocusCandidates)
		{
			Initial->SetUserFocus(GetOwningPlayer());
		}
	}
	else
	{
		PreviewName->SetText(LOCTEXT("Empty", "표시할 패시브가 없습니다"));
		PreviewDescription->SetText(FText::GetEmpty());
		MVPassiveUISetArtwork(*PreviewImage, {}, nullptr);
		if (bFocusCandidates)
		{
			NativeGetDesiredFocusTarget()->SetUserFocus(GetOwningPlayer());
		}
	}
	UpdateNavigation();
}

void UMVWeaponPassiveSelectionWindow::AddEntry(UMVPassiveCandidateEntryWidget* Entry)
{
	CandidateList->AddChild(Entry);
	Entry->OnClicked().AddUObject(this, &ThisClass::PreviewCandidate, Entry);
	Entry->OnFocusReceived().AddUObject(this, &ThisClass::PreviewCandidate, Entry);
}

void UMVWeaponPassiveSelectionWindow::PreviewCandidate(UMVPassiveCandidateEntryWidget* Entry)
{
	if (!IsActivated() || IsFadingOut() || !IsValid(Entry) || Entry->GetParent() != CandidateList || PreviewedEntry.Get() == Entry)
	{
		return;
	}
	const FDataTableRowHandle& Row = Entry->GetSkillRow();
	const auto* Skill = Row.IsNull() ? nullptr : Row.GetRow<FMVSkillDataTableColumn>(TEXT("PassiveSelection.Preview"));
	if (!Skill && !Entry->IsClearSelection())
	{
		return;
	}
	if (auto* Previous = PreviewedEntry.Get())
	{
		Previous->SetPreviewed(false);
	}
	PreviewedEntry = Entry;
	SetRequestMessage(FText::GetEmpty());
	Entry->SetPreviewed(true);
	ApplyButton->SetIsEnabled(Entry->CanApply());
	CandidateList->ScrollWidgetIntoView(Entry, false);
	UpdateNavigation();
	if (Entry->IsClearSelection())
	{
		PreviewName->SetText(LOCTEXT("ClearSelection", "선택 해제"));
		if (Skill)
		{
			PreviewDescription->SetText(FText::Format(LOCTEXT("ClearNamed", "적용을 누르면 ‘{0}’ 패시브를 해제합니다."), MVPassiveUIName(*Skill)));
		}
		else
		{
			PreviewDescription->SetText(Entry->CanApply() ? LOCTEXT("ClearSaved", "적용을 누르면 현재 무기의 패시브 선택을 해제합니다.")
				: LOCTEXT("AlreadyClear", "현재 선택한 패시브가 없습니다."));
		}
		MVPassiveUISetArtwork(*PreviewImage, {}, nullptr);
		return;
	}
	PreviewName->SetText(MVPassiveUIName(*Skill));
	PreviewDescription->SetText(Skill->Description.IsEmpty() ? LOCTEXT("NoDescription", "등록된 설명이 없습니다") : Skill->Description);
	MVPassiveUISetArtwork(*PreviewImage, Skill->PreviewImage.IsNull() ? Skill->Icon : Skill->PreviewImage, MissingArtwork);
}

void UMVWeaponPassiveSelectionWindow::ApplyPreviewedCandidate()
{
	AMVPlayerCharacter* Player = ViewedPlayer.Get();
	const auto* Entry = PreviewedEntry.Get();
	if (!IsActivated() || IsFadingOut() || bApplyingSelection || !Player || !Entry || Entry->GetParent() != CandidateList || !Entry->CanApply())
	{
		return;
	}
	TGuardValue<bool> ApplyingGuard(bApplyingSelection, true);
	auto* Controller = GetOwningPlayer();
	if (!CanOpenForPlayer(Controller) || Controller->GetPawn() != Player)
	{
		DeactivateWidget();
		return;
	}

	UMVWeaponComponent& Weapons = *Player->WeaponComponent;
	const FMVEquippedWeaponState Weapon = Weapons.GetEquippedWeaponState();
	const auto* Item = MVPassiveUIFindWeapon(Weapons.GetOwnedWeapons(), Weapons, Weapon, ViewedItemTag);
	if (!Item || !ViewedWeaponType.IsValid() || Item->WeaponTypeTag != ViewedWeaponType)
	{
		RefreshWeapons();
		SetRequestMessage(LOCTEXT("WeaponChanged", "무기 상태가 바뀌었습니다. 다시 선택해 주세요."));
		return;
	}
	const FDataTableRowHandle Requested = Entry->GetSkillRow();
	const FGameplayTag RequestedType = ViewedWeaponType;
	FDataTableRowHandle Selected;
	const bool bSameSelection = Weapons.GetSelectedPassive(ViewedWeaponType, Selected) && Selected == Requested;
	const bool bClearing = Entry->IsClearSelection();
	bool bAccepted = false;
	if (bClearing)
	{
		// 미리보기 뒤 외부에서 선택을 바꿨다면 새 패시브를 해제하지 않는다.
		if (bSameSelection)
		{
			bAccepted = Weapons.ClearSelectedPassive(ViewedWeaponType);
		}
	}
	else
	{
		const bool bCurrentType = Weapon.bValid && Weapon.WeaponTypeTag == ViewedWeaponType;
		if (!bSameSelection || (bCurrentType && !MVPassiveUIIsActive(*Player->CombatComponent, Requested)))
		{
			bAccepted = Weapons.SelectPassive(ViewedWeaponType, Requested);
		}
		else
		{
			bAccepted = true;
		}
	}
	// 변경 알림은 다음 틱으로 모으고, 이 요청의 결과는 여기서 한 번 갱신한다.
	GetWorld()->GetTimerManager().ClearTimer(RefreshTimer);
	if (!IsActivated() || ViewedPlayer.Get() != Player)
	{
		return;
	}
	RefreshWeapons();
	if (ViewedWeaponType != RequestedType)
	{
		return;
	}
	FDataTableRowHandle FinalSelection;
	const bool bHasSelection = Weapons.GetSelectedPassive(ViewedWeaponType, FinalSelection);
	if (bClearing && !bHasSelection)
	{
		SetRequestMessage(LOCTEXT("Cleared", "패시브 선택을 해제했습니다."));
	}
	else if (!bClearing && bHasSelection && FinalSelection == Requested)
	{
		const auto Equipped = Weapons.GetEquippedWeaponState();
		if (!Equipped.bValid || Equipped.WeaponTypeTag != ViewedWeaponType)
		{
			SetRequestMessage(LOCTEXT("Remembered", "이 종류의 무기를 장착하면 적용됩니다."));
		}
		else if (MVPassiveUIIsActive(*Player->CombatComponent, Requested))
		{
			SetRequestMessage(LOCTEXT("Applied", "패시브를 적용했습니다."));
		}
		else
		{
			SetRequestMessage(LOCTEXT("ApplyFailed", "선택됐지만 효과 적용에 실패했습니다. 다시 적용할 수 있습니다."));
		}
	}
	else if (!bAccepted)
	{
		SetRequestMessage(LOCTEXT("Rejected", "현재는 변경할 수 없습니다. 선택 상태를 확인해 주세요."));
	}
}

void UMVWeaponPassiveSelectionWindow::QueueRefresh()
{
	if (bApplyingSelection || !IsActivated() || IsFadingOut() || !GetWorld() || RefreshTimer.IsValid())
	{
		return;
	}
	RefreshTimer = GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::RefreshWeapons);
}

void UMVWeaponPassiveSelectionWindow::SetRequestMessage(const FText& Message)
{
	if (RequestMessage)
	{
		RequestMessage->SetText(Message);
		RequestMessage->SetVisibility(Message.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	}
}

void UMVWeaponPassiveSelectionWindow::UpdateNavigation()
{
	UWidget* CandidateTarget = PreviewedEntry.Get();
	UWidget* WeaponTarget = ViewedWeaponEntry.Get();
	UWidget* ActionTarget = ApplyButton->GetIsEnabled() ? ApplyButton.Get() : CloseButton.Get();
	for (int32 Index = 0; Index < WeaponList->GetChildrenCount(); ++Index)
	{
		UWidget* Entry = WeaponList->GetChildAt(Index);
		Entry->SetNavigationRuleExplicit(EUINavigation::Up, Index > 0 ? WeaponList->GetChildAt(Index - 1) : Entry);
		Entry->SetNavigationRuleExplicit(EUINavigation::Down,
			Index + 1 < WeaponList->GetChildrenCount() ? WeaponList->GetChildAt(Index + 1) : CloseButton.Get());
		Entry->SetNavigationRuleBase(EUINavigation::Left, EUINavigationRule::Stop);
		Entry->SetNavigationRuleExplicit(EUINavigation::Right, CandidateTarget ? CandidateTarget : CloseButton.Get());
	}
	for (int32 Index = 0; Index < CandidateList->GetChildrenCount(); ++Index)
	{
		UWidget* Entry = CandidateList->GetChildAt(Index);
		Entry->SetNavigationRuleExplicit(EUINavigation::Up, Index > 0 ? CandidateList->GetChildAt(Index - 1) : Entry);
		Entry->SetNavigationRuleExplicit(EUINavigation::Down,
			Index + 1 < CandidateList->GetChildrenCount() ? CandidateList->GetChildAt(Index + 1) : ActionTarget);
		Entry->SetNavigationRuleExplicit(EUINavigation::Left, WeaponTarget ? WeaponTarget : CloseButton.Get());
		Entry->SetNavigationRuleExplicit(EUINavigation::Right, ActionTarget);
	}
	UWidget* ReturnTarget = CandidateTarget ? CandidateTarget : WeaponTarget;
	if (!ReturnTarget)
	{
		ReturnTarget = CloseButton;
	}
	ApplyButton->SetNavigationRuleExplicit(EUINavigation::Up, ReturnTarget);
	ApplyButton->SetNavigationRuleExplicit(EUINavigation::Left, ReturnTarget);
	ApplyButton->SetNavigationRuleExplicit(EUINavigation::Right, CloseButton);
	CloseButton->SetNavigationRuleExplicit(EUINavigation::Up, ReturnTarget);
	CloseButton->SetNavigationRuleExplicit(EUINavigation::Left, ApplyButton->GetIsEnabled() ? ApplyButton.Get() : ReturnTarget);
}

void UMVWeaponPassiveSelectionWindow::RegisterConfirmInput()
{
	const UCommonInputSettings& Settings = ICommonInputModule::GetSettings();
	const FDataTableRowHandle ConfirmAction = Settings.GetDefaultClickAction();
	if (!ConfirmAction.IsNull())
	{
		for (EInputEvent Event : { IE_Pressed, IE_Repeat, IE_Released })
		{
			FBindUIActionArgs Args(ConfirmAction, false, FSimpleDelegate::CreateUObject(this, &ThisClass::HandleConfirmInput, Event));
			Args.InputMode = ECommonInputMode::Menu;
			Args.KeyEvent = Event;
			Args.bConsumeInput = true;
			const FUIActionBindingHandle Handle = RegisterUIActionBinding(Args);
			if (Handle.IsValid())
			{
				ConfirmInputBindings.Add(Handle);
			}
		}
	}
	if (ConfirmInputHint)
	{
		ConfirmInputHint->SetInputAction(ConfirmAction);
	}
	if (CancelInputHint)
	{
		CancelInputHint->SetInputAction(Settings.GetDefaultBackAction());
	}
}

void UMVWeaponPassiveSelectionWindow::HandleConfirmInput(EInputEvent InputEvent)
{
	if (InputEvent != IE_Pressed || !IsActivated() || IsFadingOut() || bApplyingSelection)
	{
		return;
	}
	APlayerController* Controller = GetOwningPlayer();
	if (CloseButton->HasUserFocus(Controller))
	{
		DeactivateWidgetWithFade();
	}
	else if (WeaponList->HasUserFocusedDescendants(Controller))
	{
		NativeGetDesiredFocusTarget()->SetUserFocus(Controller);
	}
	else if (CandidateList->HasUserFocusedDescendants(Controller) || ApplyButton->HasUserFocus(Controller))
	{
		ApplyPreviewedCandidate();
	}
}

FReply UMVWeaponPassiveSelectionWindow::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	if (!Reply.IsEventHandled() && IsActivated() && !IsFadingOut())
	{
		if (UWidget* FocusTarget = NativeGetDesiredFocusTarget())
		{
			return FReply::Handled().SetUserFocus(FocusTarget->TakeWidget(), EFocusCause::Mouse);
		}
	}
	return Reply;
}

UWidget* UMVWeaponPassiveSelectionWindow::NativeGetDesiredFocusTarget() const
{
	if (auto* Entry = PreviewedEntry.Get())
	{
		return Entry;
	}
	if (auto* WeaponEntry = ViewedWeaponEntry.Get())
	{
		return WeaponEntry;
	}
	return CloseButton.Get();
}

void UMVWeaponPassiveSelectionWindow::OnPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	if (!NewPawn || NewPawn != ViewedPlayer.Get())
	{
		DeactivateWidget();
	}
}

void UMVWeaponPassiveSelectionWindow::OnTransitionChanged(EMVFieldTransitionPhase Phase)
{
	if (Phase != EMVFieldTransitionPhase::Idle)
	{
		DeactivateWidget();
	}
}

void UMVWeaponPassiveSelectionWindow::OnPlayerActionStarted(FName TableName, FName RowName)
{
	DeactivateWidget();
}

void UMVWeaponPassiveSelectionWindow::OnPlayerDamageApplied(float AppliedDamage, float PreviousHP, float CurrentHP,
	const FMVResolvedHitData& HitData)
{
	DeactivateWidget();
}

void UMVWeaponPassiveSelectionWindow::ReleasePreview()
{
	if (auto* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RefreshTimer);
	}
	for (FUIActionBindingHandle& Handle : ConfirmInputBindings)
	{
		RemoveActionBinding(Handle);
		Handle.Unregister();
	}
	ConfirmInputBindings.Reset();
	if (auto* HUD = SuppressedHUD.Get())
	{
		auto* Instance = GetGameInstance();
		auto* UI = Instance ? Instance->GetSubsystem<UMVUISubsystem>() : nullptr;
		if (UI && UI->GetMainHUD() == HUD)
		{
			HUD->SetVisibility(PreviousHUDVisibility);
		}
	}
	SuppressedHUD.Reset();
	if (AMVPlayerCharacter* Player = ViewedPlayer.Get())
	{
		Player->WeaponComponent->OnEquippedWeaponChanged.RemoveDynamic(this, &ThisClass::OnEquippedWeaponChanged);
		Player->WeaponComponent->OnOwnedWeaponsChanged.RemoveDynamic(this, &ThisClass::QueueRefresh);
		Player->WeaponComponent->OnPassiveOptionsChanged.RemoveDynamic(this, &ThisClass::QueueRefresh);
		Player->CombatComponent->OnActivePassiveChanged.RemoveDynamic(this, &ThisClass::QueueRefresh);
		Player->ActionComponent->OnActionStarted.RemoveDynamic(this, &ThisClass::OnPlayerActionStarted);
		Player->StatComponent->OnDamageApplied.RemoveDynamic(this, &ThisClass::OnPlayerDamageApplied);
		Player->StatComponent->OnDead.RemoveDynamic(this, &ThisClass::DeactivateWidget);
	}
	if (auto* Controller = GetOwningPlayer())
	{
		Controller->OnPossessedPawnChanged.RemoveDynamic(this, &ThisClass::OnPawnChanged);
	}
	if (auto* Transition = UMVFieldTransitionSubsystem::Get(this))
	{
		Transition->OnFieldTransitionPhaseChanged.RemoveDynamic(this, &ThisClass::OnTransitionChanged);
	}
	if (CloseButton)
	{
		CloseButton->OnClicked.RemoveDynamic(this, &ThisClass::DeactivateWidgetWithFade);
	}
	if (ApplyButton)
	{
		ApplyButton->OnClicked.RemoveDynamic(this, &ThisClass::ApplyPreviewedCandidate);
	}
	ViewedPlayer.Reset();
	ViewedWeaponEntry.Reset();
	ViewedItemTag = {};
	ViewedWeaponType = {};
	if (WeaponList)
	{
		WeaponList->ClearChildren();
	}
	PreviewedEntry.Reset();
	if (CandidateList)
	{
		CandidateList->ClearChildren();
	}
	if (PreviewImage)
	{
		PreviewImage->SetBrushFromTexture(nullptr);
	}
}

void UMVWeaponPassiveSelectionWindow::NativeOnDeactivated()
{
	ReleasePreview();
	Super::NativeOnDeactivated();
}

void UMVWeaponPassiveSelectionWindow::NativeDestruct()
{
	ReleasePreview();
	Super::NativeDestruct();
}

#undef LOCTEXT_NAMESPACE
