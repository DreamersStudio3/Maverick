#include "Components/MVWeaponComponent.h"
#include "Character/MVCharacterBase.h"
#include "Components/MVActionComponent.h"
#include "Components/MVCombatComponent.h"
#include "Public/Tables/MVSkillDataTableColumn.h"
#include "Weapon/Skill/MVSkillSelectionData.h"
#include "Weapon/Skill/MVSkillExecutionData.h"

EMVSkillSelectionStatus UMVWeaponComponent::ValidateSkillSelectionContext(const FMVSkillSelectionContext& Context) const
{
	if (!Context.ItemTag.IsValid() || (Context.Slot == EMVSkillSlot::PassiveSkill && !Context.WeaponType.IsValid())
		|| (Context.Slot != EMVSkillSlot::PassiveSkill && !MVSkillSelectionData::IsActiveSlot(Context.Slot)))
		return EMVSkillSelectionStatus::InvalidContext;
	const auto* Owned = OwnedWeapons.Find(Context.ItemTag);
	const FMVWeaponTableRow* Weapon = Owned ? ResolveWeaponRow(*Owned, TEXT("SkillSelection.Context")) : nullptr;
	// 맨손은 소유 목록 밖의 내부 기본 무기지만, 현재 장착 문맥에 한해 기존 패시브 조회를 유지한다.
	if (!Weapon && Context.ItemTag == BareHandWeapon.ItemTag && CurrentWeaponState.bValid
		&& CurrentWeaponState.ItemTag == Context.ItemTag && CurrentWeaponState.EquippedStyle == EMVEquippedStyle::BareHand)
		Weapon = &BareHandWeapon;
	if (!Weapon) return EMVSkillSelectionStatus::WeaponNotOwned;
	if (Weapon->ItemTag != Context.ItemTag || Weapon->WeaponTypeTag != Context.WeaponType || Weapon->EquippedStyle != Context.Style)
		return EMVSkillSelectionStatus::ContextMismatch;
	const auto* Map = SkillBlueprint.Find(Context.Style);
	if (!Map || !*Map || !SkillTreeStates.Contains(Context.Style)) return EMVSkillSelectionStatus::MissingSkillMap;
	if (Context.Slot == EMVSkillSlot::PassiveSkill)
	{
		EMVEquippedStyle TypeStyle;
		if (!ResolveOwnedWeaponTypeStyle(Context.WeaponType, TypeStyle) || TypeStyle != Context.Style)
			return EMVSkillSelectionStatus::ContextMismatch;
	}
	return EMVSkillSelectionStatus::Unchanged;
}

TArray<FMVSkillSelectionCandidate> UMVWeaponComponent::GetSkillCandidates(const FMVSkillSelectionContext& Context) const
{
	TArray<FMVSkillSelectionCandidate> Result;
	if (ValidateSkillSelectionContext(Context) != EMVSkillSelectionStatus::Unchanged) return Result;
	FDataTableRowHandle Selected;
	GetSelectedSkill(Context, Selected);
	const auto AddCandidate = [&](const FDataTableRowHandle& Handle, int32 Cost, bool bUnlocked)
	{
		const auto* Row = Handle.DataTable->FindRow<FMVSkillDataTableColumn>(Handle.RowName, TEXT("SkillSelection.Candidate"), false);
		if (!Row || (Context.Slot == EMVSkillSlot::PassiveSkill && Row->ImplementationState != EMVSkillImplementationState::Ready)) return;
		FMVSkillSelectionCandidate& Candidate = Result.AddDefaulted_GetRef();
		Candidate.Row = Handle;
		Candidate.DisplayName = Row->DisplayName;
		Candidate.Description = Row->Description;
		Candidate.Icon = Row->Icon;
		Candidate.PreviewImage = Row->PreviewImage;
		Candidate.ImplementationState = Row->ImplementationState;
		Candidate.RequiredPoints = FMath::Max(0, Cost);
		Candidate.bUnlocked = bUnlocked;
		Candidate.bSelected = Handle == Selected;
	};
	if (Context.Slot == EMVSkillSlot::PassiveSkill)
	{
		for (const auto& Candidate : GetPassiveCandidates(Context.WeaponType))
			AddCandidate(Candidate.SkillRowHandle, Candidate.RequiredPoints, Candidate.bUnlocked);
	}
	else
	{
		const auto& State = SkillTreeStates.FindChecked(Context.Style);
		for (const auto& Candidate : SkillBlueprint.FindChecked(Context.Style)->GetSortedSkillCandidates(Context.Slot))
			AddCandidate(Candidate.SkillRowHandle.RowHandle, Candidate.RequiredPoints, State.UnlockedSkills.Contains(Candidate.SkillRowHandle));
	}
	return Result;
}

bool UMVWeaponComponent::GetSelectedSkill(const FMVSkillSelectionContext& Context, FDataTableRowHandle& Output) const
{
	Output = {};
	if (ValidateSkillSelectionContext(Context) != EMVSkillSelectionStatus::Unchanged) return false;
	return Context.Slot == EMVSkillSlot::PassiveSkill
		? GetSelectedPassive(Context.WeaponType, Output) : GetEquippedSkillRow(Context.Style, Context.Slot, Output);
}

FMVSkillSelectionResult UMVWeaponComponent::TrySelectSkill(const FMVSkillSelectionContext& Context, FDataTableRowHandle Row)
{
	FMVSkillSelectionResult Result;
	Result.Status = ValidateSkillSelectionContext(Context);
	if (Result.Status != EMVSkillSelectionStatus::Unchanged) return Result;
	FDataTableRowHandle Previous;
	GetSelectedSkill(Context, Previous);
	const bool bClear = !Row.DataTable && Row.RowName.IsNone();
	if (!bClear || Context.Slot != EMVSkillSlot::PassiveSkill)
	{
		const auto Candidates = GetSkillCandidates(Context);
		const auto* Candidate = Candidates.FindByPredicate([&](const auto& Entry) { return Entry.Row == Row; });
		if (!Candidate) { Result.Status = EMVSkillSelectionStatus::InvalidCandidate; return Result; }
		if (!Candidate->bUnlocked) { Result.Status = EMVSkillSelectionStatus::Locked; return Result; }
	}
	if (Previous == Row) return Result;
	const auto* Character = Cast<AMVCharacterBase>(GetOwner());
	if (Character && Character->ActionComponent && Character->ActionComponent->IsActionRunning())
	{
		Result.Status = EMVSkillSelectionStatus::ActionRunning;
		return Result;
	}
	if (Context.Slot != EMVSkillSlot::PassiveSkill)
	{
		Result.Status = TryEquipActiveSkill(Context.Style, Context.Slot, Row, &Result.SaveStatus);
		Result.bSelectionChanged = Result.Status == EMVSkillSelectionStatus::Changed;
		return Result;
	}
	bool bSuccess;
	{
		// 기존 패시브 함수의 즉시 저장을 미루고 실제 변경 결과를 확인한 뒤 한 번만 기록한다.
		TGuardValue<bool> SaveGuard(bDeferSkillSelectionSave, true);
		bSuccess = bClear ? ClearSelectedPassive(Context.WeaponType) : SelectPassive(Context.WeaponType, Row);
	}
	FDataTableRowHandle After;
	GetSelectedSkill(Context, After);
	Result.bSelectionChanged = Previous != After;
	Result.Status = bSuccess ? EMVSkillSelectionStatus::Changed : EMVSkillSelectionStatus::ApplyFailed;
	if (Result.bSelectionChanged) Result.SaveStatus = StoreWeaponProgress(true);
	return Result;
}

EMVSkillSelectionStatus UMVWeaponComponent::TryEquipActiveSkill(const EMVEquippedStyle Style, const EMVSkillSlot Slot, const FDataTableRowHandle& Row, EMVSkillSelectionSaveStatus* OutSaveStatus)
{
	if (OutSaveStatus) *OutSaveStatus = EMVSkillSelectionSaveStatus::NotRequested;
	const auto* Map = SkillBlueprint.Find(Style);
	auto* State = SkillTreeStates.Find(Style);
	if (!MVSkillSelectionData::IsActiveSlot(Slot) || !Map || !*Map || !State) return EMVSkillSelectionStatus::InvalidCandidate;
	const auto Candidates = (*Map)->GetSortedSkillCandidates(Slot);
	if (!Candidates.ContainsByPredicate([&](const auto& Candidate) { return Candidate.SkillRowHandle.RowHandle == Row; })) return EMVSkillSelectionStatus::InvalidCandidate;
	if (!State->UnlockedSkills.Contains(FMVHashDataTableRowHandle(Row))) return EMVSkillSelectionStatus::Locked;
	if (const auto* Previous = State->EquippedSkill.Find(Slot); Previous && *Previous == Row) return EMVSkillSelectionStatus::Unchanged;
	const auto* Character = Cast<AMVCharacterBase>(GetOwner());
	if (Character && Character->ActionComponent && Character->ActionComponent->IsActionRunning()) return EMVSkillSelectionStatus::ActionRunning;
	TArray<FName> Rows;
	if (!MVSkillExecutionData::ValidateDefinition(Row, Rows)) return EMVSkillSelectionStatus::InvalidExecutionData;
	if (CurrentWeaponState.bValid && Style == CurrentWeaponState.EquippedStyle)
	{
		auto* Combat = GetOwner() ? GetOwner()->FindComponentByClass<UMVCombatComponent>() : nullptr;
		if (!Combat || Combat->CurrentWeaponStyle != Style) return EMVSkillSelectionStatus::ApplyFailed;
		const auto Result = Combat->TryReplaceSkillSlot(Slot, Row);
		if (Result != EMVSkillSelectionStatus::Changed && Result != EMVSkillSelectionStatus::Unchanged) return Result;
	}
	// 전투 준비·교체 과정에는 외부 알림 없음. 선택을 확정한 뒤 저장 자료와 구독자를 갱신한다.
	State->EquippedSkill.Add(Slot, Row);
	const auto SaveStatus = StoreWeaponProgress(true);
	if (OutSaveStatus) *OutSaveStatus = SaveStatus;
	BroadcastSkillSelectionChanged(Style, Slot);
	return EMVSkillSelectionStatus::Changed;
}

EMVSkillSelectionSaveStatus UMVWeaponComponent::RetrySkillSelectionSave()
{
	return StoreWeaponProgress(true);
}

void UMVWeaponComponent::BroadcastSkillSelectionChanged(const EMVEquippedStyle Style, const EMVSkillSlot Slot, const FGameplayTag WeaponType)
{
	FMVSkillSelectionContext Scope;
	Scope.Style = Style;
	Scope.Slot = Slot;
	Scope.WeaponType = WeaponType;
	// 알림은 공유 단위. ItemTag는 비우고 QWER은 WeaponType도 비워 동일 스타일 화면이 함께 갱신되게 한다.
	OnSkillSelectionChanged.Broadcast(Scope);
}
