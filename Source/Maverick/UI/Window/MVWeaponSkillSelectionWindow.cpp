#include "UI/Window/MVWeaponSkillSelectionWindow.h"

#include "Character/PC/MVPlayerCharacter.h"
#include "Combat/MVAbilityBase.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Blueprint/WidgetTree.h"
#include "Weapon/Skill/MVSkillSelectionData.h"
#include "System/MVWorldStateSubsystem.h"
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
#include "CommonInputSubsystem.h"
#include "CommonUITypes.h"
#include "UI/System/MVMenuInputHint.h"
#include "ICommonInputModule.h"
#include "Input/CommonUIInputTypes.h"

#define LOCTEXT_NAMESPACE "MVWeaponPassiveSelection"

namespace
{
FText MVPassiveUIName(const FMVSkillDataTableColumn& Skill)
{
	return Skill.DisplayName.IsEmpty() ? LOCTEXT("Unnamed", "이름 없는 스킬") : Skill.DisplayName;
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

void UMVSkillSlotTabButton::InitializeSlot(UWidgetTree& Tree, const FMVSkillSlotDefinition& Definition)
{
	SkillSlotValue = Definition.Slot;
	InitIsFocusable(true);
	auto* Content = Tree.ConstructWidget<UHorizontalBox>();
	IconImage = Tree.ConstructWidget<UImage>();
	auto* Size = Tree.ConstructWidget<USizeBox>();
	Size->SetWidthOverride(24.0f);
	Size->SetHeightOverride(24.0f);
	Size->SetContent(IconImage);
	Content->AddChildToHorizontalBox(Size)->SetPadding(FMargin(4, 2, 6, 2));
	auto* Label = Tree.ConstructWidget<UTextBlock>();
	Label->SetText(Definition.DisplayName);
	FSlateFontInfo Font = Label->GetFont();
	Font.Size = 18;
	Label->SetFont(Font);
	Label->SetColorAndOpacity(FSlateColor(FLinearColor(0.92f, 0.84f, 0.68f, 1)));
	Content->AddChildToHorizontalBox(Label)->SetVerticalAlignment(VAlign_Center);
	SetContent(Content);
	OnClicked.AddUniqueDynamic(this, &ThisClass::HandleClicked);
	SetSlotSelected(false);
}

void UMVSkillSlotTabButton::HandleClicked() { OnSlotChosen.Broadcast(SkillSlotValue); }

void UMVSkillSlotTabButton::SetSlotSelected(const bool bSelected)
{
	SetBackgroundColor(bSelected ? FLinearColor(0.48f, 0.34f, 0.14f, 1) : FLinearColor(0.10f, 0.08f, 0.045f, 1));
}

void UMVSkillSlotTabButton::SetSlotIcon(const TSoftObjectPtr<UTexture2D>& Icon, UTexture2D* Fallback)
{
	if (IconImage) MVPassiveUISetArtwork(*IconImage, Icon, Fallback);
}

FMVSkillSelectionContext UMVWeaponSkillSelectionWindow::MakeSelectionContext() const
{
	FMVSkillSelectionContext Context;
	Context.ItemTag = ViewedItemTag;
	Context.WeaponType = ViewedWeaponType;
	Context.Slot = ViewedSlot;
	if (auto* Player = ViewedPlayer.Get())
	{
		const auto& Weapons = *Player->WeaponComponent;
		if (const auto* Weapon = MVPassiveUIFindWeapon(Weapons.GetOwnedWeapons(), Weapons, Weapons.GetEquippedWeaponState(), ViewedItemTag))
			Context.Style = Weapon->EquippedStyle;
	}
	return Context;
}

void UMVWeaponSkillSelectionWindow::BuildSlotTabs()
{
	SlotTabs->ClearChildren();
	SlotButtons.Reset();
	for (const auto& Definition : MVSkillSelectionData::GetSlots())
	{
		auto* Tab = WidgetTree->ConstructWidget<UMVSkillSlotTabButton>();
		Tab->InitializeSlot(*WidgetTree, Definition);
		Tab->OnSlotChosen.AddUObject(this, &ThisClass::SelectSkillSlot);
		SlotTabs->AddChildToHorizontalBox(Tab)->SetPadding(FMargin(2, 0));
		SlotButtons.Add(Tab);
	}
}

void UMVWeaponSkillSelectionWindow::RefreshSlotTabs()
{
	auto* Player = ViewedPlayer.Get();
	if (!Player) return;
	for (UMVSkillSlotTabButton* Tab : SlotButtons)
	{
		Tab->SetSlotSelected(Tab->GetSkillSlot() == ViewedSlot);
		auto Context = MakeSelectionContext();
		Context.Slot = Tab->GetSkillSlot();
		FDataTableRowHandle Selected;
		const bool bSelected = Player->WeaponComponent->GetSelectedSkill(Context, Selected);
		const auto* Data = bSelected ? Selected.GetRow<FMVSkillDataTableColumn>(TEXT("SkillSelection.TabIcon")) : nullptr;
		Tab->SetSlotIcon(Data ? Data->Icon : TSoftObjectPtr<UTexture2D>(), MissingArtwork);
	}
}

void UMVWeaponSkillSelectionWindow::SelectSkillSlot(const EMVSkillSlot InSkillSlot)
{
	if (!IsActivated() || IsFadingOut() || bApplyingSelection || (InSkillSlot != EMVSkillSlot::PassiveSkill && !MVSkillSelectionData::IsActiveSlot(InSkillSlot))) return;
	ViewedSlot = InSkillSlot;
	SetRequestMessage(FText::GetEmpty());
	RebuildCandidates();
}

void UMVWeaponSkillSelectionWindow::HandleSkillSelectionChanged(const FMVSkillSelectionContext& Scope)
{
	const auto Context = MakeSelectionContext();
	if (Scope.Style == Context.Style && (Scope.Slot != EMVSkillSlot::PassiveSkill || Scope.WeaponType == Context.WeaponType)) QueueRefresh();
}

void UMVWeaponSkillSelectionWindow::ShowSaveResult(const EMVSkillSelectionSaveStatus Status)
{
	if (Status == EMVSkillSelectionSaveStatus::NotRequested) return;
	LastSaveStatus = Status;
	RetrySaveButton->SetVisibility(Status == EMVSkillSelectionSaveStatus::Failed ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	switch (Status)
	{
	case EMVSkillSelectionSaveStatus::Saved: SetRequestMessage(LOCTEXT("Saved", "선택을 저장했습니다")); break;
	case EMVSkillSelectionSaveStatus::Failed: SetRequestMessage(LOCTEXT("SaveFailed", "선택됨 · 저장 실패. 다시 저장해 주세요")); break;
	case EMVSkillSelectionSaveStatus::Unavailable: SetRequestMessage(LOCTEXT("SaveUnavailable", "선택됨 · 저장 연결이 없습니다")); break;
	case EMVSkillSelectionSaveStatus::SuppressedDuringRestore: SetRequestMessage(LOCTEXT("SaveRestoring", "복원 중에는 저장할 수 없습니다")); break;
	default: break;
	}
	UpdateNavigation();
}

void UMVWeaponSkillSelectionWindow::ShowSelectionResult(const FMVSkillSelectionResult& Result)
{
	if (Result.IsSuccess())
	{
		SetRequestMessage(Result.bSelectionChanged ? LOCTEXT("SelectionChanged", "선택을 변경했습니다") : LOCTEXT("Unchanged", "이미 선택한 스킬입니다"));
		ShowSaveResult(Result.SaveStatus);
		return;
	}
	FText Message;
	switch (Result.Status)
	{
	case EMVSkillSelectionStatus::Locked: Message = LOCTEXT("LockedRequest", "아직 해금하지 않은 스킬입니다"); break;
	case EMVSkillSelectionStatus::CooldownActive: Message = LOCTEXT("Cooldown", "재사용 대기시간이 끝나면 변경할 수 있습니다"); break;
	case EMVSkillSelectionStatus::ChainActive: Message = LOCTEXT("Chain", "스킬 연계가 끝나면 변경할 수 있습니다"); break;
	case EMVSkillSelectionStatus::ProjectilePending: Message = LOCTEXT("Projectile", "발사한 공격이 모두 끝나면 변경할 수 있습니다"); break;
	case EMVSkillSelectionStatus::InvalidExecutionData: Message = LOCTEXT("InvalidDefinition", "스킬 자료에 오류가 있어 변경하지 않았습니다"); break;
	case EMVSkillSelectionStatus::ActionRunning: Message = LOCTEXT("ActionRunning", "행동이 끝나면 변경할 수 있습니다"); break;
	case EMVSkillSelectionStatus::ApplyFailed: Message = LOCTEXT("ApplyFailed", "효과 적용에 실패했습니다. 선택 상태를 확인해 주세요"); break;
	default: Message = LOCTEXT("InvalidRequest", "무기나 후보 상태가 바뀌었습니다. 다시 확인해 주세요"); break;
	}
	ShowSaveResult(Result.SaveStatus);
	SetRequestMessage(Result.SaveStatus == EMVSkillSelectionSaveStatus::Failed
		? FText::Format(LOCTEXT("ApplyAndSaveFailed", "{0}\n선택 기록 저장에도 실패했습니다"), Message) : Message);
}

void UMVWeaponSkillSelectionWindow::RetrySave()
{
	if (!IsActivated() || IsFadingOut() || bApplyingSelection || !ViewedPlayer.IsValid()) return;
	TGuardValue<bool> Guard(bApplyingSelection, true);
	const bool bRestoreFocus = RetrySaveButton && RetrySaveButton->HasUserFocus(GetOwningPlayer());
	ShowSaveResult(ViewedPlayer->WeaponComponent->RetrySkillSelectionSave());
	if (IsActivated() && bRestoreFocus && RetrySaveButton->GetVisibility() == ESlateVisibility::Collapsed)
		NativeGetDesiredFocusTarget()->SetUserFocus(GetOwningPlayer());
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

bool UMVWeaponSkillSelectionWindow::CanOpenForPlayer(const APlayerController* Controller)
{
	const auto* Player = IsValid(Controller) && Controller->IsLocalController()
		? Cast<AMVPlayerCharacter>(Controller->GetPawn()) : nullptr;
	const auto* Transition = UMVFieldTransitionSubsystem::Get(Player);
	return IsValid(Player) && Player->WeaponComponent && Player->CombatComponent
		&& Player->StatComponent && Player->StatComponent->IsBaseStatsReady() && !Player->StatComponent->IsDead()
		&& Player->ActionComponent && !Player->ActionComponent->IsActionRunning()
		&& Transition && !Transition->IsTransitionRunning();
}

UMVWeaponSkillSelectionWindow::UMVWeaponSkillSelectionWindow(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsFocusable(true);
}

void UMVWeaponSkillSelectionWindow::NativeOnActivated()
{
	Super::NativeOnActivated();
	APlayerController* Controller = GetOwningPlayer();
	if (!CanOpenForPlayer(Controller))
	{
		DeactivateWidget();
		return;
	}
	if (!SlotTabs || !SharingScopeText || !RetrySaveButton || !CandidateEntryClass || !WeaponList || !CandidateList || !WeaponName || !PreviewName || !PreviewDescription || !PreviewImage || !ApplyButton)
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
	ViewedSlot = EMVSkillSlot::PassiveSkill;
	LastSaveStatus = EMVSkillSelectionSaveStatus::NotRequested;
	BuildSlotTabs();
	RetrySaveButton->SetVisibility(ESlateVisibility::Collapsed);
	RetrySaveButton->OnClicked.AddUniqueDynamic(this, &ThisClass::RetrySave);
	SkillChangedHandle = Player->WeaponComponent->OnSkillSelectionChanged.AddUObject(this, &ThisClass::HandleSkillSelectionChanged);
	Player->WeaponComponent->OnEquippedWeaponChanged.AddUniqueDynamic(this, &ThisClass::OnEquippedWeaponChanged);
	Player->WeaponComponent->OnOwnedWeaponsChanged.AddUniqueDynamic(this, &ThisClass::QueueRefresh);
	Player->WeaponComponent->OnPassiveOptionsChanged.AddUniqueDynamic(this, &ThisClass::QueueRefresh);
	Player->CombatComponent->OnActivePassiveChanged.AddUniqueDynamic(this, &ThisClass::QueueRefresh);
	Player->ActionComponent->OnActionStarted.AddUniqueDynamic(this, &ThisClass::OnPlayerActionStarted);
	Player->StatComponent->OnDamageApplied.AddUniqueDynamic(this, &ThisClass::OnPlayerDamageApplied);
	Player->StatComponent->OnDead.AddUniqueDynamic(this, &ThisClass::DeactivateWidget);
	Controller->OnPossessedPawnChanged.AddUniqueDynamic(this, &ThisClass::OnPawnChanged);
	UMVFieldTransitionSubsystem::Get(Player)->OnFieldTransitionPhaseChanged.AddUniqueDynamic(this, &ThisClass::OnTransitionChanged);
	ApplyButton->OnClicked.AddUniqueDynamic(this, &ThisClass::ApplyPreviewedCandidate);
	RegisterConfirmInput();
	RefreshInputHints();
	if (UCommonInputSubsystem* Input = UCommonInputSubsystem::Get(GetOwningLocalPlayer()))
	{
		Input->OnInputMethodChangedNative.RemoveAll(this);
		Input->OnInputMethodChangedNative.AddWeakLambda(this, [this](ECommonInputType) { RefreshInputHints(); });
	}
	SetRequestMessage(FText::GetEmpty());
	RefreshWeapons();
	if (const auto* State = UMVWorldStateSubsystem::Get(this); State && State->HasFailedActiveSave())
		ShowSaveResult(EMVSkillSelectionSaveStatus::Failed);
}

void UMVWeaponSkillSelectionWindow::OnEquippedWeaponChanged(const FMVEquippedWeaponState& Weapon)
{
	QueueRefresh();
}

void UMVWeaponSkillSelectionWindow::RefreshWeapons()
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

void UMVWeaponSkillSelectionWindow::BrowseWeapon(UMVPassiveCandidateEntryWidget* Entry, FGameplayTag ItemTag, FGameplayTag WeaponType, bool bFocusCandidates)
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

void UMVWeaponSkillSelectionWindow::RebuildCandidates(bool bFocusCandidates)
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
	const auto Context = MakeSelectionContext();
	const auto Equipped = Weapons.GetEquippedWeaponState();
	const bool bCurrentScope = Equipped.bValid && (ViewedSlot == EMVSkillSlot::PassiveSkill
		? Equipped.WeaponTypeTag == Context.WeaponType : Equipped.EquippedStyle == Context.Style);
	FDataTableRowHandle Selected;
	const bool bHasSelection = Weapons.GetSelectedSkill(Context, Selected);
	UMVPassiveCandidateEntryWidget* Initial = nullptr;
	RefreshSlotTabs();
	SharingScopeText->SetText(ViewedSlot == EMVSkillSlot::PassiveSkill
		? LOCTEXT("TypeScope", "이 무기 종류에 공통 적용") : LOCTEXT("StyleScope", "이 무기 스타일에 공통 적용"));
	auto Candidates = Weapons.GetSkillCandidates(Context);
	Candidates.StableSort([](const FMVSkillSelectionCandidate& A, const FMVSkillSelectionCandidate& B)
	{
		return A.bSelected && !B.bSelected;
	});
	for (const auto& Candidate : Candidates)
	{
		const auto* Skill = Candidate.Row.GetRow<FMVSkillDataTableColumn>(TEXT("SkillSelection.Candidate"));
		if (!Skill) continue;
		auto* Entry = CreateWidget<UMVPassiveCandidateEntryWidget>(GetOwningPlayer(), CandidateEntryClass);
		if (!Entry) continue;
		const bool bPrototype = Candidate.ImplementationState == EMVSkillImplementationState::Prototype;
		const bool bApplied = Candidate.bSelected && bCurrentScope && !bPrototype && (ViewedSlot == EMVSkillSlot::PassiveSkill
			? MVPassiveUIIsActive(*Player->CombatComponent, Candidate.Row) : Player->CombatComponent->IsSkillSlotExecutable(ViewedSlot));
		FText State;
		if (!Candidate.bUnlocked)
			State = FText::Format(LOCTEXT("LockedCost", "잠김 · 필요 포인트 {0}"), FText::AsNumber(Candidate.RequiredPoints));
		else if (Candidate.bSelected)
			State = bApplied ? LOCTEXT("Equipped", "장착 중") : LOCTEXT("Selected", "선택됨");
		if (bPrototype)
			State = FText::Format(LOCTEXT("PrototypeState", "{0} 준비 중 · 전투 사용 불가"), State);
		const bool bCanApply = Candidate.bUnlocked && !Candidate.bSelected;
		Entry->SetCandidate(Candidate.Row, *Skill, State, bCanApply, MissingArtwork);
		AddEntry(Entry);
		if (!Initial) Initial = Entry;
	}
	if (ViewedSlot == EMVSkillSlot::PassiveSkill && (Initial || bHasSelection))
	{
		if (auto* Clear = CreateWidget<UMVPassiveCandidateEntryWidget>(GetOwningPlayer(), CandidateEntryClass))
		{
			Clear->SetClearSelection(Selected);
			AddEntry(Clear);
			if (!Initial) Initial = Clear;
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
		PreviewName->SetText(LOCTEXT("Empty", "등록된 스킬 없음"));
		PreviewDescription->SetText(FText::GetEmpty());
		MVPassiveUISetArtwork(*PreviewImage, {}, nullptr);
		if (bFocusCandidates)
		{
			NativeGetDesiredFocusTarget()->SetUserFocus(GetOwningPlayer());
		}
	}
	UpdateNavigation();
}

void UMVWeaponSkillSelectionWindow::AddEntry(UMVPassiveCandidateEntryWidget* Entry)
{
	CandidateList->AddChild(Entry);
	Entry->OnClicked().AddUObject(this, &ThisClass::PreviewCandidate, Entry);
	Entry->OnFocusReceived().AddUObject(this, &ThisClass::PreviewCandidate, Entry);
}

void UMVWeaponSkillSelectionWindow::PreviewCandidate(UMVPassiveCandidateEntryWidget* Entry)
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
	const FText Description = Skill->Description.IsEmpty() ? LOCTEXT("NoDescription", "등록된 설명이 없습니다") : Skill->Description;
	PreviewDescription->SetText(Skill->ImplementationState == EMVSkillImplementationState::Prototype
		? FText::Format(LOCTEXT("PrototypeDescription", "준비 중 · 전투 사용 불가\n{0}"), Description) : Description);
	MVPassiveUISetArtwork(*PreviewImage, Skill->PreviewImage.IsNull() ? Skill->Icon : Skill->PreviewImage, MissingArtwork);
}

void UMVWeaponSkillSelectionWindow::ApplyPreviewedCandidate()
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
	if (!Item || (ViewedSlot == EMVSkillSlot::PassiveSkill && !ViewedWeaponType.IsValid()) || Item->WeaponTypeTag != ViewedWeaponType)
	{
		RefreshWeapons();
		SetRequestMessage(LOCTEXT("WeaponChanged", "무기 상태가 바뀌었습니다. 다시 선택해 주세요."));
		return;
	}
	const auto Context = MakeSelectionContext();
	FDataTableRowHandle Selected;
	if (Entry->IsClearSelection() && (!Weapons.GetSelectedSkill(Context, Selected) || Selected != Entry->GetSkillRow()))
	{
		RebuildCandidates();
		SetRequestMessage(LOCTEXT("SelectionChanged", "선택 상태가 바뀌었습니다. 다시 확인해 주세요"));
		return;
	}
	const auto Result = Weapons.TrySelectSkill(Context, Entry->IsClearSelection() ? FDataTableRowHandle() : Entry->GetSkillRow());
	GetWorld()->GetTimerManager().ClearTimer(RefreshTimer);
	if (!IsActivated() || ViewedPlayer.Get() != Player) return;
	RebuildCandidates();
	ShowSelectionResult(Result);
}

void UMVWeaponSkillSelectionWindow::QueueRefresh()
{
	if (bApplyingSelection || !IsActivated() || IsFadingOut() || !GetWorld() || RefreshTimer.IsValid())
	{
		return;
	}
	RefreshTimer = GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::RefreshWeapons);
}

void UMVWeaponSkillSelectionWindow::SetRequestMessage(const FText& Message)
{
	if (RequestMessage)
	{
		const FText Display = Message.IsEmpty() && LastSaveStatus == EMVSkillSelectionSaveStatus::Failed
			? LOCTEXT("PendingSave", "저장하지 못한 선택이 있습니다. 다시 저장해 주세요") : Message;
		RequestMessage->SetText(Display);
		RequestMessage->SetVisibility(Display.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	}
}

void UMVWeaponSkillSelectionWindow::UpdateNavigation()
{
	UWidget* Candidate = PreviewedEntry.Get();
	UWidget* Weapon = ViewedWeaponEntry.Get();
	UWidget* Apply = ApplyButton->GetIsEnabled() ? ApplyButton.Get() : nullptr;
	UWidget* Retry = RetrySaveButton && RetrySaveButton->GetVisibility() == ESlateVisibility::Visible ? RetrySaveButton.Get() : nullptr;
	UWidget* ActiveTab = this;
	for (UMVSkillSlotTabButton* Tab : SlotButtons) if (Tab->GetSkillSlot() == ViewedSlot) ActiveTab = Tab;
	for (int32 Index = 0; Index < SlotButtons.Num(); ++Index)
	{
		auto* Tab = SlotButtons[Index].Get();
		Tab->SetNavigationRuleExplicit(EUINavigation::Left, Index > 0 ? SlotButtons[Index - 1].Get() : Tab);
		Tab->SetNavigationRuleExplicit(EUINavigation::Right, Index + 1 < SlotButtons.Num() ? SlotButtons[Index + 1].Get() : Tab);
		Tab->SetNavigationRuleBase(EUINavigation::Up, EUINavigationRule::Stop);
		Tab->SetNavigationRuleExplicit(EUINavigation::Down, Candidate ? Candidate : (Weapon ? Weapon : Tab));
	}
	UWidget* Action = Apply ? Apply : Retry;
	for (int32 Index = 0; Index < WeaponList->GetChildrenCount(); ++Index)
	{
		auto* Entry = WeaponList->GetChildAt(Index);
		Entry->SetNavigationRuleExplicit(EUINavigation::Up, Index > 0 ? WeaponList->GetChildAt(Index - 1) : ActiveTab);
		Entry->SetNavigationRuleExplicit(EUINavigation::Down, Index + 1 < WeaponList->GetChildrenCount() ? WeaponList->GetChildAt(Index + 1) : (Action ? Action : ActiveTab));
		Entry->SetNavigationRuleBase(EUINavigation::Left, EUINavigationRule::Stop);
		Entry->SetNavigationRuleExplicit(EUINavigation::Right, Candidate ? Candidate : ActiveTab);
	}
	for (int32 Index = 0; Index < CandidateList->GetChildrenCount(); ++Index)
	{
		auto* Entry = CandidateList->GetChildAt(Index);
		Entry->SetNavigationRuleExplicit(EUINavigation::Up, Index > 0 ? CandidateList->GetChildAt(Index - 1) : ActiveTab);
		Entry->SetNavigationRuleExplicit(EUINavigation::Down, Index + 1 < CandidateList->GetChildrenCount() ? CandidateList->GetChildAt(Index + 1) : (Action ? Action : ActiveTab));
		Entry->SetNavigationRuleExplicit(EUINavigation::Left, Weapon ? Weapon : ActiveTab);
		Entry->SetNavigationRuleExplicit(EUINavigation::Right, Action ? Action : ActiveTab);
	}
	UWidget* Return = Candidate ? Candidate : ActiveTab;
	ApplyButton->SetNavigationRuleExplicit(EUINavigation::Up, Return);
	ApplyButton->SetNavigationRuleExplicit(EUINavigation::Left, Return);
	ApplyButton->SetNavigationRuleBase(EUINavigation::Right, EUINavigationRule::Stop);
	if (Retry) ApplyButton->SetNavigationRuleExplicit(EUINavigation::Down, Retry);
	else ApplyButton->SetNavigationRuleBase(EUINavigation::Down, EUINavigationRule::Stop);
	if (Retry)
	{
		Retry->SetNavigationRuleExplicit(EUINavigation::Up, Apply ? Apply : Return);
		Retry->SetNavigationRuleExplicit(EUINavigation::Left, Return);
		Retry->SetNavigationRuleBase(EUINavigation::Right, EUINavigationRule::Stop);
		Retry->SetNavigationRuleBase(EUINavigation::Down, EUINavigationRule::Stop);
	}
}

void UMVWeaponSkillSelectionWindow::RegisterConfirmInput()
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
		CancelInputHint->SetInputAction(MVMenuInputHint::GetBackAction(GetOwningLocalPlayer()));
	}
}

void UMVWeaponSkillSelectionWindow::RefreshInputHints()
{
	const FDataTableRowHandle Action = MVMenuInputHint::GetBackAction(GetOwningLocalPlayer());
	bool bHasIcon = false;
	if (CancelInputHint)
	{
		CancelInputHint->SetInputAction(Action);
		const FSlateBrush Icon = CancelInputHint->GetIcon();
		bHasIcon = Icon.DrawAs != ESlateBrushDrawType::NoDrawType
			&& (Icon.GetResourceObject() || !Icon.GetResourceName().IsNone());
	}
	if (CancelInputLabel)
	{
		const FText Label = LOCTEXT("BackHint", "돌아가기");
		CancelInputLabel->SetText(bHasIcon ? Label : MVMenuInputHint::MakeActionLabel(GetOwningLocalPlayer(), Action, Label));
	}
}

void UMVWeaponSkillSelectionWindow::HandleConfirmInput(EInputEvent InputEvent)
{
	if (InputEvent != IE_Pressed || !IsActivated() || IsFadingOut() || bApplyingSelection)
	{
		return;
	}
	APlayerController* Controller = GetOwningPlayer();
	if (RetrySaveButton && RetrySaveButton->HasUserFocus(Controller))
	{
		RetrySave();
	}
	else if (SlotTabs && SlotTabs->HasUserFocusedDescendants(Controller))
	{
		for (UMVSkillSlotTabButton* Tab : SlotButtons) if (Tab->HasUserFocus(Controller)) { SelectSkillSlot(Tab->GetSkillSlot()); break; }
		NativeGetDesiredFocusTarget()->SetUserFocus(Controller);
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

FReply UMVWeaponSkillSelectionWindow::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
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

bool UMVWeaponSkillSelectionWindow::IsConfirmKey(const FKey InKey) const
{
	const auto Action = ICommonInputModule::GetSettings().GetDefaultClickAction();
	const auto* Data = Action.GetRow<FCommonInputActionDataBase>(TEXT("SkillSelection.ConfirmKey"));
	if (!Data) return false;
	const auto* Input = UCommonInputSubsystem::Get(GetOwningLocalPlayer());
	const FName Gamepad = Input ? Input->GetCurrentGamepadName() : NAME_None;
	return Data->GetInputTypeInfo(ECommonInputType::MouseAndKeyboard, NAME_None).GetKey() == InKey
		|| Data->GetInputTypeInfo(ECommonInputType::Gamepad, Gamepad).GetKey() == InKey;
}

FReply UMVWeaponSkillSelectionWindow::NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (IsActivated() && !IsFadingOut() && IsConfirmKey(InKeyEvent.GetKey()))
	{
		HandleConfirmInput(InKeyEvent.IsRepeat() ? IE_Repeat : IE_Pressed);
		return FReply::Handled();
	}
	return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
}

FReply UMVWeaponSkillSelectionWindow::NativeOnKeyUp(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (IsActivated() && IsConfirmKey(InKeyEvent.GetKey())) return FReply::Handled();
	return Super::NativeOnKeyUp(InGeometry, InKeyEvent);
}

UWidget* UMVWeaponSkillSelectionWindow::NativeGetDesiredFocusTarget() const
{
	if (auto* Entry = PreviewedEntry.Get())
	{
		return Entry;
	}
	for (UMVSkillSlotTabButton* Tab : SlotButtons) if (Tab && Tab->GetSkillSlot() == ViewedSlot) return Tab;
	if (auto* WeaponEntry = ViewedWeaponEntry.Get()) return WeaponEntry;
	return const_cast<UMVWeaponSkillSelectionWindow*>(this);
}

void UMVWeaponSkillSelectionWindow::OnPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	if (!NewPawn || NewPawn != ViewedPlayer.Get())
	{
		DeactivateWidget();
	}
}

void UMVWeaponSkillSelectionWindow::OnTransitionChanged(EMVFieldTransitionPhase Phase)
{
	if (Phase != EMVFieldTransitionPhase::Idle)
	{
		DeactivateWidget();
	}
}

void UMVWeaponSkillSelectionWindow::OnPlayerActionStarted(FName TableName, FName RowName)
{
	DeactivateWidget();
}

void UMVWeaponSkillSelectionWindow::OnPlayerDamageApplied(float AppliedDamage, float PreviousHP, float CurrentHP,
	const FMVResolvedHitData& HitData)
{
	DeactivateWidget();
}

void UMVWeaponSkillSelectionWindow::ReleasePreview()
{
	if (UCommonInputSubsystem* Input = UCommonInputSubsystem::Get(GetOwningLocalPlayer()))
	{
		Input->OnInputMethodChangedNative.RemoveAll(this);
	}
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
		Player->WeaponComponent->OnSkillSelectionChanged.Remove(SkillChangedHandle);
		SkillChangedHandle.Reset();
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
	if (ApplyButton)
	{
		ApplyButton->OnClicked.RemoveDynamic(this, &ThisClass::ApplyPreviewedCandidate);
	}
	if (RetrySaveButton) RetrySaveButton->OnClicked.RemoveDynamic(this, &ThisClass::RetrySave);
	for (UMVSkillSlotTabButton* Tab : SlotButtons) if (Tab) Tab->OnSlotChosen.RemoveAll(this);
	SlotButtons.Reset();
	if (SlotTabs) SlotTabs->ClearChildren();
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

void UMVWeaponSkillSelectionWindow::NativeOnDeactivated()
{
	ReleasePreview();
	Super::NativeOnDeactivated();
}

void UMVWeaponSkillSelectionWindow::NativeDestruct()
{
	ReleasePreview();
	Super::NativeDestruct();
}

#undef LOCTEXT_NAMESPACE
