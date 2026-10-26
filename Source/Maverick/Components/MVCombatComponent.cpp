// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/MVCombatComponent.h"

#include "Combat/MVAbilityBase.h"
#include "Combat/Projectile/MVProjectileBase.h"
#include "Components/MVActionComponent.h"
#include "Components/MVWeaponComponent.h"
#include "Character/MVCharacterBase.h"
#include "Combat/MVHitResolverSubsystem.h"
#include "Components/MVStatComponent.h"
#include "Engine/DataTable.h"
#include "Public/Interface/MVAbilityInterface.h"
#include "Tags/MVGameplayTags.h"
#include "Character/NPC/Enemy/MVEnemy.h"

DEFINE_LOG_CATEGORY_STATIC(LogMVCombatComponent, Log, All);

namespace
{
FString MVCombatActionTypeToString(const EMVCombatActionTypes ActionType)
{
	const UEnum* EnumPtr = StaticEnum<EMVCombatActionTypes>();
	return EnumPtr
		? EnumPtr->GetNameStringByValue(static_cast<int64>(ActionType))
		: FString(TEXT("None"));
}

FString MVCombatRowHandleToString(const FDataTableRowHandle& RowHandle)
{
	return FString::Printf(
		TEXT("%s.%s"),
		*GetNameSafe(RowHandle.DataTable),
		*RowHandle.RowName.ToString());
}

FString MVCombatSkillRowNamesToString(const TArray<FName>& RowNames)
{
	FString Result;
	for (int32 Index = 0; Index < RowNames.Num(); ++Index)
	{
		if (Index > 0)
		{
			Result += TEXT(" -> ");
		}

		Result += RowNames[Index].ToString();
	}

	return Result;
}

bool MVCombatIsSprintContextualBasicAttackActionType(const EMVCombatActionTypes ActionType)
{
	return ActionType == EMVCombatActionTypes::SprintLightAttack
		|| ActionType == EMVCombatActionTypes::SprintHeavyAttack;
}

bool MVCombatIsDodgeContextualBasicAttackActionType(const EMVCombatActionTypes ActionType)
{
	return ActionType == EMVCombatActionTypes::DodgeLightAttack
		|| ActionType == EMVCombatActionTypes::DodgeHeavyAttack;
}

bool MVCombatIsActiveSkillSlot(const EMVSkillSlot SkillSlot)
{
	return SkillSlot == EMVSkillSlot::QSkill
		|| SkillSlot == EMVSkillSlot::RSkill
		|| SkillSlot == EMVSkillSlot::WSkill
		|| SkillSlot == EMVSkillSlot::ESkill;
}

EMVSkillSlot MVCombatResolveSkillSlotFromInputTag(const FGameplayTag ActionInputTag)
{
	if (ActionInputTag.MatchesTagExact(MVGameplayTags::Action_Input_Skill_Q))
	{
		return EMVSkillSlot::QSkill;
	}

	if (ActionInputTag.MatchesTagExact(MVGameplayTags::Action_Input_Skill_R))
	{
		return EMVSkillSlot::RSkill;
	}

	if (ActionInputTag.MatchesTagExact(MVGameplayTags::Action_Input_Skill_W))
	{
		return EMVSkillSlot::WSkill;
	}

	if (ActionInputTag.MatchesTagExact(MVGameplayTags::Action_Input_Skill_E))
	{
		return EMVSkillSlot::ESkill;
	}

	return EMVSkillSlot::NONE;
}

// Todo: 해당 기능을 하는 함수가 필요한 지 확인 후 로직 정리예정
FName MVCombatActionTableNameFromDataTable(const UDataTable* DataTable)
{
	if (!DataTable)
	{
		return NAME_None;
	}

	FString TableName = DataTable->GetName();
	TableName.RemoveFromStart(TEXT("DT_"));
	return FName(*TableName);
}

// Todo: 추후 해당 Swing Direction 관련 삭제예정
EMVAttackSwingDirection MVCombatGetOppositeSwingDirection(const EMVAttackSwingDirection Direction)
{
	switch (Direction)
	{
	case EMVAttackSwingDirection::RightToLeft:
		return EMVAttackSwingDirection::LeftToRight;
	case EMVAttackSwingDirection::LeftToRight:
		return EMVAttackSwingDirection::RightToLeft;
	default:
		return EMVAttackSwingDirection::None;
	}
}

}

// Sets default values for this component's properties
UMVCombatComponent::UMVCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UMVCombatComponent::BeginPlay()
{
	RefreshActionMaps();




	// This goes to the UActorComponent BeginPlay()
	// And UActorComponent will goes to the Blueprint Function If Blueprint exist
	Super::BeginPlay();


	AMVCharacterBase* OwnerCharacter = Cast<AMVCharacterBase>(GetOwner());
	if (!OwnerCharacter)
	{
		return;
	}

	if (UMVInputManagerComponent* InputManager = OwnerCharacter->InputManagerComponent)
	{
		InputManager->RegisterActionInputHandler(this, MVActionInputHandlerPriorities::Combat);
	}

	if (UMVHitResolverSubsystem* HitResolver = UMVHitResolverSubsystem::Get(this))
	{
		HitResolver->OnHitResolved.RemoveDynamic(this, &UMVCombatComponent::HandleHitResolved);
		HitResolver->OnHitResolved.AddUniqueDynamic(this, &UMVCombatComponent::HandleHitResolved);
		HitResolver->OnHitDeliveryFinished.RemoveDynamic(this, &UMVCombatComponent::HandleHitDeliveryFinished);
		HitResolver->OnHitDeliveryFinished.AddUniqueDynamic(this, &UMVCombatComponent::HandleHitDeliveryFinished);
	}

	if (UMVActionComponent* ActionComponent = OwnerCharacter->ActionComponent)
	{
		ActionComponent->OnActionEnded.RemoveDynamic(this, &UMVCombatComponent::HandleActionEnded);
		ActionComponent->OnActionEnded.AddUniqueDynamic(this, &UMVCombatComponent::HandleActionEnded);
	}

	StatComponent = OwnerCharacter->FindComponentByClass<UMVStatComponent>();
	if (!StatComponent)
	{
		return;
	}
	
	StatComponent->OnDeathStarted.RemoveDynamic(this, &UMVCombatComponent::HandleOwnerDeathStarted);
	StatComponent->OnDeathStarted.AddUniqueDynamic(this, &UMVCombatComponent::HandleOwnerDeathStarted);
	ResetRSkillGauge();
}

void UMVCombatComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearPassiveSkill();

	if (const AMVCharacterBase* OwnerCharacter = Cast<AMVCharacterBase>(GetOwner()))
	{
		if (UMVInputManagerComponent* InputManager = OwnerCharacter->InputManagerComponent)
		{
			InputManager->UnregisterActionInputHandler(this);
		}

		if (UMVHitResolverSubsystem* HitResolver = UMVHitResolverSubsystem::Get(this))
		{
			HitResolver->OnHitResolved.RemoveDynamic(this, &UMVCombatComponent::HandleHitResolved);
			HitResolver->OnHitDeliveryFinished.RemoveDynamic(this, &UMVCombatComponent::HandleHitDeliveryFinished);
		}

		if (UMVActionComponent* ActionComponent = OwnerCharacter->ActionComponent)
		{
			ActionComponent->OnActionEnded.RemoveDynamic(this, &UMVCombatComponent::HandleActionEnded);
		}
	}

	if (StatComponent)
	{
		StatComponent->OnDeathStarted.RemoveDynamic(this, &UMVCombatComponent::HandleOwnerDeathStarted);
	}

	for (TPair<int32, FMVProjectileExecutionRecord>& Pair : ProjectileExecutions)
	{
		for (AMVProjectileBase* Projectile : Pair.Value.ActiveProjectiles)
		{
			if (IsValid(Projectile))
			{
				Projectile->OnProjectileHit.RemoveDynamic(
					this, &UMVCombatComponent::HandleRegisteredProjectileHit);
				Projectile->OnProjectileFinished.RemoveDynamic(
					this, &UMVCombatComponent::HandleRegisteredProjectileFinished);
			}
		}
	}
	ProjectileExecutions.Reset();

	Super::EndPlay(EndPlayReason);
}

bool UMVCombatComponent::ApplyPassiveSkill(const FDataTableRowHandle& PassiveSkillRowHandle)
{
	if (ActivePassiveSkillRowHandle == PassiveSkillRowHandle
		&& IsValid(ActivePassiveAbilityInstance)
		&& ActivePassiveAbilityInstance->bAbilityActive)
	{
		return true;
	}

	// 교체는 요청된 장착 상태로의 동기화다. 신규 적용 실패 시 이전 Passive를 복구하지 않는다.
	ClearPassiveSkill();

	if (!PassiveSkillRowHandle.DataTable || PassiveSkillRowHandle.RowName.IsNone())
	{
		UE_LOG(LogMVCombatComponent, Warning, TEXT("ApplyPassiveSkill: Invalid passive skill row handle."));
		return false;
	}

	const UDataTable* PassiveSkillTable = PassiveSkillRowHandle.DataTable.Get();
	if (!PassiveSkillTable->GetRowStruct()
		|| !PassiveSkillTable->GetRowStruct()->IsChildOf(FMVSkillDataTableColumn::StaticStruct()))
	{
		UE_LOG(
			LogMVCombatComponent,
			Warning,
			TEXT("ApplyPassiveSkill: Passive row must use FMVSkillDataTableColumn. Row=%s"),
			*MVCombatRowHandleToString(PassiveSkillRowHandle));
		return false;
	}

	const FMVSkillDataTableColumn* PassiveSkillData = PassiveSkillTable->FindRow<FMVSkillDataTableColumn>(
		PassiveSkillRowHandle.RowName,
		TEXT("UMVCombatComponent::ApplyPassiveSkill"),
		false);
	if (!PassiveSkillData || !PassiveSkillData->AbilityReference)
	{
		UE_LOG(
			LogMVCombatComponent,
			Warning,
			TEXT("ApplyPassiveSkill: Passive row or AbilityReference is invalid. Row=%s"),
			*MVCombatRowHandleToString(PassiveSkillRowHandle));
		return false;
	}

	if (!FMath::IsNearlyZero(PassiveSkillData->HpCost)
		|| !FMath::IsNearlyZero(PassiveSkillData->StaminaCost)
		|| !FMath::IsNearlyZero(PassiveSkillData->MpCost))
	{
		UE_LOG(
			LogMVCombatComponent,
			Warning,
			TEXT("ApplyPassiveSkill: Passive ability cannot have an activation cost. Row=%s"),
			*MVCombatRowHandleToString(PassiveSkillRowHandle));
		return false;
	}

	if (PassiveSkillData->bIsChained || !PassiveSkillData->NextChainName.IsNone())
	{
		UE_LOG(
			LogMVCombatComponent,
			Warning,
			TEXT("ApplyPassiveSkill: Passive ability cannot use a skill chain. Row=%s"),
			*MVCombatRowHandleToString(PassiveSkillRowHandle));
		return false;
	}

	UMVAbilityBase* NewPassiveAbility = NewObject<UMVAbilityBase>(this, PassiveSkillData->AbilityReference);
	if (!IsValid(NewPassiveAbility))
	{
		UE_LOG(
			LogMVCombatComponent,
			Warning,
			TEXT("ApplyPassiveSkill: Failed to create passive ability. Row=%s"),
			*MVCombatRowHandleToString(PassiveSkillRowHandle));
		return false;
	}

	NewPassiveAbility->SetOwner(this);
	NewPassiveAbility->InitAbility(*PassiveSkillData);
	ActivePassiveAbilityInstance = NewPassiveAbility;
	ActivePassiveSkillRowHandle = PassiveSkillRowHandle;

	IMVAbilityInterface::Execute_StartAbility(NewPassiveAbility, INDEX_NONE);
	if (!NewPassiveAbility->bAbilityActive)
	{
		ActivePassiveAbilityInstance = nullptr;
		ActivePassiveSkillRowHandle = FDataTableRowHandle();
		IMVAbilityInterface::Execute_EndAbility(NewPassiveAbility);

		UE_LOG(
			LogMVCombatComponent,
			Warning,
			TEXT("ApplyPassiveSkill: Passive ability did not become active. Blueprint overrides must call the parent StartAbility. Row=%s"),
			*MVCombatRowHandleToString(PassiveSkillRowHandle));
		return false;
	}

	return true;
}

void UMVCombatComponent::ClearPassiveSkill()
{
	UMVAbilityBase* PassiveAbilityToDeactivate = ActivePassiveAbilityInstance.Get();
	ActivePassiveAbilityInstance = nullptr;
	ActivePassiveSkillRowHandle = FDataTableRowHandle();

	if (IsValid(PassiveAbilityToDeactivate)
		&& PassiveAbilityToDeactivate->Implements<UMVAbilityInterface>())
	{
		IMVAbilityInterface::Execute_EndAbility(PassiveAbilityToDeactivate);
	}
}


// Called every frame
void UMVCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	UpdateContextualBasicAttackResets();
	TryUpdateHeavyChargeAttack(0.0f);
}

bool UMVCombatComponent::GetSkillSlotRuntimeState(
	const EMVSkillSlot SkillSlot,
	FMVSkillSlotRuntimeState& OutState) const
{
	OutState = FMVSkillSlotRuntimeState();

	const bool bRSkillGaugeSlot = bUseRSkillGauge && SkillSlot == EMVSkillSlot::RSkill;

	OutState.bGaugeControlled = bRSkillGaugeSlot;

	if (bRSkillGaugeSlot)
	{
		OutState.GaugeCurrent = CurrentRSkillGauge;
		OutState.GaugeMax = FMath::Max(RSkillGaugeMax, KINDA_SMALL_NUMBER);
		OutState.GaugeRatio = FMath::Clamp(OutState.GaugeCurrent / OutState.GaugeMax, 0.0f, 1.0f);
		OutState.bGaugeReady = IsRSkillGaugeReady();
	}

	const FMVSkillEntry* SkillEntry = SkillMap.Find(SkillSlot);
	const UWorld* World = GetWorld();
	if (!SkillEntry || !World)
	{
		return false;
	}

	const float CurrentTime = World->GetTimeSeconds();
	const bool bChainRuntimeActive = SkillEntry->bChainActive;
	const bool bInputWindowActive = bChainRuntimeActive	&& SkillEntry->IsInputWindowValid(CurrentTime);

	OutState.bAvailable = SkillEntry->SkillRowNames.Num() > 0;
	OutState.bChainActive = bInputWindowActive;
	OutState.bChainTimerVisible =
		bInputWindowActive
		&& SkillEntry->CurrentChainStageIndex > 0;
	OutState.StackSize = SkillEntry->SkillRowNames.Num();
	OutState.ActiveStackIndex = bInputWindowActive
		? FMath::Clamp(SkillEntry->CurrentChainStageIndex, 0, FMath::Max(0, OutState.StackSize - 1))
		: 0;

	OutState.ChainTimerIconIndex = SkillEntry->CurrentChainStageIndex > 0 ?
		FMath::Clamp(SkillEntry->CurrentChainStageIndex - 1, 0, FMath::Max(0, OutState.StackSize - 1)) : 0;

	const bool bSlotAbilityRunning = bCurrentAbilityAwaitingCompletion
		&& CurrentAbilityInstance
		&& SkillEntry->ContainsAbility(CurrentAbilityInstance);
	if (bSlotAbilityRunning)
	{
		for (int32 AbilityIndex = 0; AbilityIndex < SkillEntry->AbilityInstances.Num(); ++AbilityIndex)
		{
			if (SkillEntry->AbilityInstances[AbilityIndex].Get() == CurrentAbilityInstance.Get())
			{
				OutState.ActiveStackIndex = AbilityIndex;
				break;
			}
		}
	}

	if (bChainRuntimeActive)
	{
		const FMVSkillDataTableColumn* CurrentStageData = SkillEntry->GetCurrentSkillData();
		OutState.CooldownDuration = CurrentStageData
			? FMath::Max(0.0f, CurrentStageData->InterStageCooldown)
			: 0.0f;
		if (OutState.CooldownDuration > 0.0f)
		{
			const float StageElapsedTime = FMath::Max(0.0f, CurrentTime - SkillEntry->LastStageActivationTime);
			OutState.CooldownRemaining = FMath::Clamp(
				OutState.CooldownDuration - StageElapsedTime,
				0.0f,
				OutState.CooldownDuration);
		}

		if (CurrentStageData && bInputWindowActive)
		{
			OutState.ChainWindowDuration = FMath::Max(0.0f, CurrentStageData->InterStageCooldown) +
				FMath::Max(0.0f, CurrentStageData->InputWindowDuration);

			OutState.ChainWindowRemaining = FMath::Clamp(
				SkillEntry->GetRemainingInputWindowTime(CurrentTime),
				0.0f,
				OutState.ChainWindowDuration);
		}
	}
	else
	{
		OutState.CooldownDuration = FMath::Max(0.0f, SkillEntry->MainCooldownDuration);
		OutState.CooldownRemaining = SkillEntry->GetMainCooldownRemaining(CurrentTime);
	}

	if (bSlotAbilityRunning)
	{
		OutState.CooldownRemaining = OutState.CooldownDuration;
	}

	OutState.bOnCooldown = OutState.CooldownRemaining > KINDA_SMALL_NUMBER;

	OutState.bLoadoutAvailable =
		OutState.bAvailable
		&& CurrentWeaponStyle != EMVEquippedStyle::DualWield;

	OutState.bUsable =
		OutState.bLoadoutAvailable
		&& !OutState.bOnCooldown
		&& (!OutState.bGaugeControlled || OutState.bGaugeReady);

	return OutState.bAvailable;
}

bool UMVCombatComponent::CanStartCombatAction() const
{
	const AMVCharacterBase* OwnerCharacter = Cast<AMVCharacterBase>(GetOwner());
	const UMVActionComponent* ActionComponent = OwnerCharacter
		? OwnerCharacter->ActionComponent
		: nullptr;
	if (!ActionComponent)
	{
		return false;
	}

	const UMVInputManagerComponent* InputManager = OwnerCharacter->InputManagerComponent;
	const bool bRecoveryEscapeWindowOpen = !InputManager || InputManager->IsRecoveryEscapeWindowOpen();
	return !ActionComponent->IsActionRunning()
		|| (bRecoveryEscapeWindowOpen && ActionComponent->CanInterruptActiveAction());
}

bool UMVCombatComponent::TryBasicCombatAction(
	const EMVCombatActionTypes InActionType,
	const FName StartSection)
{
	if (!CanStartCombatAction())
	{
		return false;
	}

	const EMVCombatActionTypes ResolvedActionType = ResolveContextualBasicAttackActionType(InActionType);
	if (!IsBasicAttackActionType(ResolvedActionType))
	{
		return false;
	}

	const bool bStarted = TryBasicAttack(ResolvedActionType, StartSection);
	if (bStarted)
	{
		MarkContextualBasicAttackStarted(ResolvedActionType);
		BroadcastCombatActionStarted(ResolvedActionType, EMVSkillSlot::NONE);
	}

	return bStarted;
}

bool UMVCombatComponent::TrySkillCombatAction(
	const EMVSkillSlot SkillSlot,
	const FName StartSection)
{
	if (!MVCombatIsActiveSkillSlot(SkillSlot) || !CanStartCombatAction())
	{
		return false;
	}

	if (SkillSlot == EMVSkillSlot::RSkill && bUseRSkillGauge && !IsRSkillGaugeReady())
	{
		return false;
	}

	const bool bStarted = TrySkill(SkillSlot, StartSection);
	if (bStarted)
	{
		if (SkillSlot == EMVSkillSlot::RSkill && bUseRSkillGauge)
		{
			ResetRSkillGauge();
		}

		BroadcastCombatActionStarted(EMVCombatActionTypes::Skill, SkillSlot);
	}

	return bStarted;
}

bool UMVCombatComponent::TryHandleActionInput(
	const FGameplayTag ActionInputTag,
	const FVector2D /*ControllerSpaceInput*/,
	const bool /*bHasMovementInput*/)
{
	if (!IsCombatActionInputTag(ActionInputTag))
	{
		return false;
	}

	return ChooseTryCombatAction(ActionInputTag);
}

bool UMVCombatComponent::TryHandleHoldActionInput(
	const FGameplayTag ActionInputTag,
	const EMVActionInputPhase Phase,
	const float HeldSeconds,
	const FVector2D /*ControllerSpaceInput*/,
	const bool /*bHasMovementInput*/)
{
	if (!IsHeavyChargeActionInputTag(ActionInputTag))
	{
		return false;
	}

	switch (Phase)
	{
	case EMVActionInputPhase::Started:
		return TryBeginHeavyChargeAttack(HeldSeconds);
	case EMVActionInputPhase::Triggered:
		return TryUpdateHeavyChargeAttack(HeldSeconds);
	case EMVActionInputPhase::Completed:
		return TryReleaseHeavyChargeAttack(HeldSeconds, false);
	case EMVActionInputPhase::Canceled:
		return TryReleaseHeavyChargeAttack(HeldSeconds, true);
	default:
		return false;
	}
}

bool UMVCombatComponent::ChooseTryCombatAction(const FGameplayTag ActionInputTag)
{
	UpdateContextualBasicAttackResets();

	if (ActionInputTag.MatchesTag(MVGameplayTags::Action_Input_Skill))
	{
		const EMVSkillSlot SkillSlot = MVCombatResolveSkillSlotFromInputTag(ActionInputTag);
		if (SkillSlot == EMVSkillSlot::NONE || !SkillMap.Contains(SkillSlot))
		{
			// 알 수 없는 스킬 입력은 실행 대상이 아니거나 현재 장비에 등록되지 않은 스킬 입력은 즉시 소비
			return true;
		}


		const bool bRSkillInput = SkillSlot == EMVSkillSlot::RSkill;
		if (bRSkillInput && bUseRSkillGauge && !IsRSkillGaugeReady())
		{
			// 게이지 미충전 상태의 R 입력 즉시 소비
			return true;
		}

		return TrySkillCombatAction(SkillSlot);
	}
	// Basic attack - Light attack, Heavy Attack, Charge Attack
	else if(ActionInputTag.MatchesTagExact(MVGameplayTags::Action_Input_LightAttack))
	{
		return TryBasicCombatAction(EMVCombatActionTypes::LightAttack);
	}
	else if (ActionInputTag.MatchesTagExact(MVGameplayTags::Action_Input_HeavyAttack))
	{
		return TryBasicCombatAction(EMVCombatActionTypes::HeavyAttack);
	}
	else if (ActionInputTag.MatchesTagExact(MVGameplayTags::Action_Input_HeavyChargeAttack))
	{
		return TryBasicCombatAction(EMVCombatActionTypes::HeavyAttack);
	}
	else if (ActionInputTag.MatchesTagExact(MVGameplayTags::Action_Input_ChargeAttack))
	{
		if (ShouldSuppressChargeAttackInputForSprint())
		{
			const bool bCanStartSprintHeavyAttack = IsSprintAttackContext()
				&& !bSprintContextualBasicAttackConsumed;
			if (bCanStartSprintHeavyAttack)
			{
				return TryBasicCombatAction(EMVCombatActionTypes::HeavyAttack);
			}

			return true;
		}

		return TryBasicCombatAction(EMVCombatActionTypes::ChargeAttack);
	}

	return false;
}

bool UMVCombatComponent::IsCombatActionInputTag(const FGameplayTag ActionInputTag) const
{
	return ActionInputTag.MatchesTagExact(MVGameplayTags::Action_Input_LightAttack)
		|| ActionInputTag.MatchesTagExact(MVGameplayTags::Action_Input_HeavyAttack)
		|| ActionInputTag.MatchesTagExact(MVGameplayTags::Action_Input_HeavyChargeAttack)
		|| ActionInputTag.MatchesTagExact(MVGameplayTags::Action_Input_ChargeAttack)
		|| ActionInputTag.MatchesTag(MVGameplayTags::Action_Input_Skill);
}

bool UMVCombatComponent::IsHeavyChargeActionInputTag(const FGameplayTag ActionInputTag) const
{
	return ActionInputTag.MatchesTagExact(MVGameplayTags::Action_Input_HeavyChargeAttack)
		|| ActionInputTag.MatchesTagExact(MVGameplayTags::Action_Input_HeavyAttack);
}




bool UMVCombatComponent::TryBasicAttack(
	EMVCombatActionTypes InActionType,
	FName StartSection)
{
	FMVSkillEntry* ActionEntry = BasicAttackMap.Find(InActionType);

	if (!ActionEntry)
	{
		UE_LOG(LogTemp, Warning, TEXT("MVCombatComponent::TryBasicAttack - Action '%s' not found"), *MVCombatActionTypeToString(InActionType));
		return false;
	}

	float CurrentTime = GetWorld()->GetTimeSeconds();

	// Reset Basic Attack's Index and Start Attack
	if (CurrentTime - LastBasicAttackedTime > ResetBasicAttackTime)
	{
		for (TPair<EMVCombatActionTypes, FMVSkillEntry>& Pair : BasicAttackMap)
		{
			FMVSkillEntry& CurrentEntry = Pair.Value;
			CurrentEntry.ResetChain();
		}
		ActiveBasicAttackType = EMVCombatActionTypes::None;
		ClearLastBasicAttackSwingDirection();
	}

	if (ActiveBasicAttackType != EMVCombatActionTypes::None && ActiveBasicAttackType != InActionType)
	{
		ResetOtherBasicAttackChains(InActionType);
		ActionEntry->ResetChain();
	}

	const int32 OriginalChainStageIndex = ActionEntry->CurrentChainStageIndex;
	const int32 SelectedChainStageIndex = SelectBasicAttackChainStageForSwing(*ActionEntry);
	ActionEntry->CurrentChainStageIndex = SelectedChainStageIndex;
	const FDataTableRowHandle CurrentRowHandle = ActionEntry->GetCurrentActionRowHandle();

	if (!CanConsumeActionCost(ActionEntry->GetCurrentSkillData()))
	{
		ActionEntry->CurrentChainStageIndex = OriginalChainStageIndex;
		return false;
	}

	UE_LOG(
		LogMVCombatComponent,
		Log,
		TEXT("TryBasicAttack: Action=%s Row=%s ChainStage=%d ChainActive=%s"),
		*MVCombatActionTypeToString(InActionType),
		*MVCombatRowHandleToString(CurrentRowHandle),
		ActionEntry->CurrentChainStageIndex,
		ActionEntry->bChainActive ? TEXT("true") : TEXT("false"));

	if (ActionEntry->bChainActive)
	{
		if (TryStartActionWithAbility(*ActionEntry, CurrentRowHandle, StartSection))
		{
			UpdateLastBasicAttackSwingDirection(CurrentRowHandle);
			ActionEntry->TryAdvanceChainStage(CurrentTime);
			LastBasicAttackedTime = CurrentTime;
			ActiveBasicAttackType = InActionType;
			return true;
		}
		else
		{
			ActionEntry->CurrentChainStageIndex = OriginalChainStageIndex;
			UE_LOG(LogTemp, Warning, TEXT("MVCombatComponent::TryBasicAttack - Failed to send data to ActionComponent for action '%s'"), *MVCombatActionTypeToString(InActionType));
			return false;
		}

	}
	else
	{
		if (TryStartActionWithAbility(*ActionEntry, CurrentRowHandle, StartSection))
		{
			UpdateLastBasicAttackSwingDirection(CurrentRowHandle);
			ActionEntry->ActivateChain(CurrentTime);
			ActionEntry->CurrentChainStageIndex = SelectedChainStageIndex;
			ActionEntry->TryAdvanceChainStage(CurrentTime);
			ResetOtherBasicAttackChains(InActionType);
			ActiveBasicAttackType = InActionType;
		}
		else
		{
			ActionEntry->CurrentChainStageIndex = OriginalChainStageIndex;
			UE_LOG(LogTemp, Warning, TEXT("MVCombatComponent::TryBasicAttack - Failed to send data to ActionComponent for action '%s'"), *MVCombatActionTypeToString(InActionType));
			return false;
		}
	}
		LastBasicAttackedTime = CurrentTime;
		return true;

}

bool UMVCombatComponent::TrySkill(const EMVSkillSlot SkillSlot, FName StartSection)
{
	FMVSkillEntry* ActionEntry = SkillMap.Find(SkillSlot);
	if (!ActionEntry)
	{
		UE_LOG(LogTemp, Warning, TEXT("MVCombatComponent::TrySkill - Skill slot '%d' not found"), static_cast<int32>(SkillSlot));
		return false;
	}

	if (!CanConsumeActionCost(ActionEntry->GetCurrentSkillData()))
	{
		return false;
	}

	float CurrentTime = GetWorld()->GetTimeSeconds();

	// Chain Is Active --> doing chaining actions
	if (ActionEntry->bChainActive)
	{
		if (!ActionEntry->IsInterStageCooldownValid(CurrentTime))
		{
			return false;
		}

		// Input Window is expired
		if (!ActionEntry->IsInputWindowValid(CurrentTime))
		{
			ActionEntry->ResetChain();
			if (!ActionEntry->IsMainCooldownReady(CurrentTime))
			{
				return false;
			}

			if (TryStartActionWithAbility(*ActionEntry, ActionEntry->GetCurrentActionRowHandle(), StartSection))
			{
				ClearLastBasicAttackSwingDirection();
				ActionEntry->ActivateChain(CurrentTime);

				const FMVSkillDataTableColumn* RestartedSkillData = ActionEntry->GetCurrentSkillData();

				if (RestartedSkillData && RestartedSkillData->ChainAdvancePolicy == EMVChainAdvancePolicy::Immediate)
				{
					ActionEntry->TryAdvanceChainStage(CurrentTime);
				}
				return true;
			}
			else
			{
				return false;
			}
		}

		// Inter-stage cooldown is not ready
		if (!ActionEntry->IsInterStageCooldownValid(CurrentTime))
		{
			return false;
		}

		if (ActionEntry->CurrentChainStageIndex < ActionEntry->AbilityInstances.Num())
		{

			if (TryStartActionWithAbility(*ActionEntry, ActionEntry->GetCurrentActionRowHandle(), StartSection))
			{
				ClearLastBasicAttackSwingDirection();

				const FMVSkillDataTableColumn* CurrentSkillData = ActionEntry->GetCurrentSkillData();

				if (CurrentSkillData && CurrentSkillData->ChainAdvancePolicy == EMVChainAdvancePolicy::Immediate)
				{
					ActionEntry->TryAdvanceChainStage(CurrentTime);
				}
				return true;
			}
			else
			{
				return false;
			}
		}
	}
	// Chain is not Active --> Start Action
	else
	{
		if (!ActionEntry->IsMainCooldownReady(CurrentTime))
		{
			return false;
		}

		if (TryStartActionWithAbility(*ActionEntry, ActionEntry->GetCurrentActionRowHandle(), StartSection))
		{
			ClearLastBasicAttackSwingDirection();
			ActionEntry->ActivateChain(CurrentTime);

			const FMVSkillDataTableColumn* CurrentSkillData = ActionEntry->GetCurrentSkillData();

			if (CurrentSkillData && CurrentSkillData->ChainAdvancePolicy == EMVChainAdvancePolicy::Immediate)
			{
				ActionEntry->TryAdvanceChainStage(CurrentTime);
			}
			return true;
		}
		else
		{
			return false;
		}

	}
	return false;
}

bool UMVCombatComponent::RegisterLaunchedProjectile(
	AMVProjectileBase* Projectile,
	UMVAbilityBase* SourceAbility)
{
	if (!IsValid(Projectile) ||
		!IsValid(SourceAbility) ||
		CurrentAbilityInstance.Get() != SourceAbility ||
		!SourceAbility->bAbilityActive ||
		CurrentAttackInstanceId == INDEX_NONE ||
		SourceAbility->GetAttackInstanceId() != CurrentAttackInstanceId ||
		Projectile->GetAttackInstanceId() != CurrentAttackInstanceId)
	{
		return false;
	}

	FMVProjectileExecutionRecord* Record = ProjectileExecutions.Find(CurrentAttackInstanceId);

	if (Record)
	{
		if (Record->SourceAbility.Get() != SourceAbility ||
			!IsValid(Record->ExecutionAbility.Get()) ||
			Record->bActionEnded ||
			Record->ActiveProjectiles.Contains(Projectile))
		{
			return false;
		}
	}
	else
	{
		// 원본 Ability가 다음 공격에서 다시 준비되기 전에 실행 값을 복사한다.
		UMVAbilityBase* ExecutionAbility = NewObject<UMVAbilityBase>(this);
		if (!IsValid(ExecutionAbility))
		{
			return false;
		}

		ExecutionAbility->SetOwner(this);
		ExecutionAbility->AbilityData = SourceAbility->AbilityData;
		ExecutionAbility->SetAttackInstanceId(CurrentAttackInstanceId);
		ExecutionAbility->SetExecutionSource(
			SourceAbility->GetSourceSkillIndex(),
			SourceAbility->GetSourceWeaponItemTag());
		ExecutionAbility->ConsumedMPThisExecution =
			SourceAbility->GetConsumedMPThisExecution();
		ExecutionAbility->bAbilityCostConsumed =
			SourceAbility->bAbilityCostConsumed;
		ExecutionAbility->HitLaunchData =
			SourceAbility->GetHitLaunchData();

		Record = &ProjectileExecutions.FindOrAdd(CurrentAttackInstanceId);
		Record->SourceAbility = SourceAbility;
		Record->ExecutionAbility = ExecutionAbility;
		Record->SourceWeaponItemTag = SourceAbility->GetSourceWeaponItemTag();
		Record->bWasBasicAttack = IsBasicAttackAbility(SourceAbility);
		Record->bWasRSkill = IsCurrentAbilityRSkill();

		const int32 SourceSkillIndex = SourceAbility->GetSourceSkillIndex();
		if (SourceSkillIndex >= 0 && SourceSkillIndex < MVCombatSkillSlots::Count)
		{
			const FMVSkillEntry* SourceSkillEntry = SkillMap.Find(MVCombatMakeSkillMapKey(SourceSkillIndex));
			if (SourceSkillEntry)
			{
				for (int32 StageIndex = 0;
					StageIndex < SourceSkillEntry->AbilityInstances.Num();
					++StageIndex)
				{
					if (SourceSkillEntry->AbilityInstances[StageIndex].Get()
						== SourceAbility)
					{
						Record->SourceChainStageIndex = StageIndex;
						break;
					}
				}
			}
		}
	}

	Record->ActiveProjectiles.Add(Projectile);

	Projectile->OnProjectileHit.AddUniqueDynamic(
		this, &UMVCombatComponent::HandleRegisteredProjectileHit);
	Projectile->OnProjectileFinished.AddUniqueDynamic(
		this, &UMVCombatComponent::HandleRegisteredProjectileFinished);
	return true;
}

void UMVCombatComponent::HandleRegisteredProjectileHit(
	AMVProjectileBase* Projectile,
	const FMVResolvedHitData& HitData)
{
	if (!IsValid(Projectile) ||
		HitData.AttackInstanceId != Projectile->GetAttackInstanceId() ||
		HitData.Attacker.Get() != GetOwner())
	{
		return;
	}

	FMVProjectileExecutionRecord* Record = ProjectileExecutions.Find(HitData.AttackInstanceId);
	if (Record && Record->ActiveProjectiles.Contains(Projectile))
	{
		Record->bHadHit = true;
	}
}

void UMVCombatComponent::HandleRegisteredProjectileFinished(
	AMVProjectileBase* Projectile,
	int32 AttackInstanceId)
{
	FMVProjectileExecutionRecord* Record = ProjectileExecutions.Find(AttackInstanceId);
	if (!Record)
	{
		return;
	}

	Record->ActiveProjectiles.Remove(Projectile);
	FinalizeProjectileExecution(AttackInstanceId);
}

void UMVCombatComponent::CloseProjectileExecution(int32 AttackInstanceId)
{
	FMVProjectileExecutionRecord* Record = ProjectileExecutions.Find(AttackInstanceId);
	if (!Record)
	{
		return;
	}

	Record->bActionEnded = true;
	FinalizeProjectileExecution(AttackInstanceId);
}

void UMVCombatComponent::FinalizeProjectileExecution(int32 AttackInstanceId)
{
	FMVProjectileExecutionRecord* Record = ProjectileExecutions.Find(AttackInstanceId);
	if (!Record || !Record->bActionEnded || !Record->ActiveProjectiles.IsEmpty())
	{
		return;
	}

	if (Record->bMissPending && !Record->bHadHit && !Record->bMissNotified)
	{
		Record->bMissNotified = true;

		UMVAbilityBase* MissedAbility = Record->ExecutionAbility.Get();
		const AMVCharacterBase* OwnerCharacter = Cast<AMVCharacterBase>(GetOwner());
		const UMVWeaponComponent* Weapon = IsValid(OwnerCharacter)
			? OwnerCharacter->WeaponComponent.Get()
			: nullptr;
		const FGameplayTag EquippedWeaponItemTag = IsValid(Weapon)
			? Weapon->GetEquippedWeaponState().ItemTag
			: FGameplayTag();

		if (IsValid(MissedAbility) &&
			EquippedWeaponItemTag == Record->SourceWeaponItemTag)
		{
			OnAttackMissed.Broadcast(MissedAbility);
		}
	}

	ProjectileExecutions.Remove(AttackInstanceId);
}

void UMVCombatComponent::HandleAbilityEnded(const UMVAbilityBase* EndedAbility)
{
	if (!EndedAbility || !GetWorld())
	{
		return;
	}

	const float CurrentTime = GetWorld()->GetTimeSeconds();
	bool bHandledAbility = false;

	for (TPair<EMVCombatActionTypes, FMVSkillEntry>& Pair : BasicAttackMap)
	{
		if (Pair.Value.ContainsAbility(EndedAbility))
		{
			Pair.Value.StartPostAbilityResetWindow(CurrentTime);
			LastBasicAttackedTime = CurrentTime;
			bHandledAbility = true;
			break;
		}
	}

	if (!bHandledAbility)
	{
		for (TPair<EMVSkillSlot, FMVSkillEntry>& Pair : SkillMap)
		{
			if (Pair.Value.ContainsAbility(EndedAbility))
			{
				Pair.Value.StartPostAbilityResetWindow(CurrentTime);
				bHandledAbility = true;
				break;
			}
		}
	}

	if (bHandledAbility && CurrentAbilityInstance.Get() == EndedAbility)
	{
		FMVProjectileExecutionRecord* ProjectileRecord =
			ProjectileExecutions.Find(EndedAbility->GetAttackInstanceId());
		const bool bOwnProjectileExecution =
			ProjectileRecord &&
			ProjectileRecord->SourceAbility.Get() == EndedAbility;

		if (bCurrentAbilityAwaitingCompletion
			&& EndedAbility->bAbilityCostConsumed
			&& !bCurrentAbilityHitConfirmed)
		{
			if (bOwnProjectileExecution)
			{
				ProjectileRecord->bMissPending = true;
			}
			else
			{
				OnAttackMissed.Broadcast(
					const_cast<UMVAbilityBase*>(EndedAbility));
			}
		}

		bCurrentAbilityAwaitingCompletion = false;
	}
}

void UMVCombatComponent::HandleHitResolved(const FMVResolvedHitData& HitData)
{
	AMVCharacterBase* OwnerCharacter = Cast<AMVCharacterBase>(GetOwner());
	const bool bCurrentAttackHit = IsCurrentAttackHit(HitData);

	FMVProjectileExecutionRecord* ProjectileRecord =
		HitData.Origin == EMVResolvedHitOrigin::AttackCollision
			? ProjectileExecutions.Find(HitData.AttackInstanceId)
			: nullptr;

	if (!bCurrentAttackHit && !ProjectileRecord)
	{
		return;
	}

	UMVAbilityBase* EventAbility = ProjectileRecord
		? ProjectileRecord->ExecutionAbility.Get()
		: CurrentAbilityInstance.Get();

	if (!IsValid(OwnerCharacter) ||
	!IsValid(EventAbility) ||
	HitData.Attacker.Get() != OwnerCharacter)
	{
		return;
	}

	const bool bProjectileExecution = ProjectileRecord != nullptr;
	const TWeakObjectPtr<UMVAbilityBase> ProjectileSourceAbility =
		bProjectileExecution
			? ProjectileRecord->SourceAbility
			: TWeakObjectPtr<UMVAbilityBase>();
	const int32 ProjectileSourceSkillIndex = bProjectileExecution
		? EventAbility->GetSourceSkillIndex()
		: INDEX_NONE;
	const int32 ProjectileSourceChainStageIndex = bProjectileExecution
		? ProjectileRecord->SourceChainStageIndex
		: INDEX_NONE;
	const FGameplayTag ProjectileSourceWeaponItemTag = bProjectileExecution
		? ProjectileRecord->SourceWeaponItemTag
		: FGameplayTag();

	const bool bWasRSkill = ProjectileRecord
		? ProjectileRecord->bWasRSkill
		: IsCurrentAbilityRSkill();

	const bool bPublishPassiveEvents =
		!ProjectileRecord ||
		IsProjectileSourceWeaponEquipped(*ProjectileRecord, HitData);

	// 이벤트가 다른 액션을 시작하더라도 이 공격의 적중 여부를 먼저 고정한다.
	if (ProjectileRecord)
	{
		ProjectileRecord->bHadHit = true;
	}
	if (bCurrentAttackHit)
	{
		bCurrentAbilityHitConfirmed = true;
	}

	if (HitData.Origin == EMVResolvedHitOrigin::AttackCollision &&
		bPublishPassiveEvents)
	{
		OnValidatedAttackHit.Broadcast(HitData, EventAbility);
	}

	AMVEnemy* HitEnemy = Cast<AMVEnemy>(HitData.Victim.Get());

	if (IsValid(OwnerCharacter) &&
		OwnerCharacter->IsPlayerControlled() &&
		IsValid(HitEnemy) &&
		HitData.FinalDamage > 0.0f &&
		IsValid(StatComponent.Get()))
	{
		const float MPRecoveryAmount =
			FMath::Max(0.0f, EventAbility->AbilityData.MpRecoveryPerHit);
		StatComponent->RecoverMP(MPRecoveryAmount);
	}

	if (bUseRSkillGauge && IsValid(HitEnemy) && !bWasRSkill)
	{
		AddRSkillGauge(EventAbility->AbilityData.RSkillGaugeGainMultiplier);
	}

	EventAbility->ApplyOnHitStatusEffect(HitData);

	if (!GetWorld())
	{
		return;
	}

	if (bProjectileExecution)
	{
		if (HitData.AttackInstanceId != CurrentAttackInstanceId ||
			ProjectileSourceSkillIndex < 0 ||
			ProjectileSourceSkillIndex >= MVCombatSkillSlots::Count ||
			ProjectileSourceChainStageIndex == INDEX_NONE ||
			!IsValid(ProjectileSourceAbility.Get()) ||
			!IsValid(OwnerCharacter))
		{
			return;
		}

		const UMVWeaponComponent* Weapon =
			OwnerCharacter->WeaponComponent.Get();
		const FGameplayTag CurrentWeaponItemTag = IsValid(Weapon)
			? Weapon->GetEquippedWeaponState().ItemTag
			: FGameplayTag();

		if (CurrentWeaponItemTag != ProjectileSourceWeaponItemTag)
		{
			return;
		}
	}
	else if (!IsCurrentAttackHit(HitData))
	{
		return;
	}

	const float CurrentTime = GetWorld()->GetTimeSeconds();

	for (TPair<EMVSkillSlot, FMVSkillEntry>& Pair : SkillMap)
	{
		FMVSkillEntry& SkillEntry = Pair.Value;

		if (bProjectileExecution)
		{
			if (Pair.Key != MVCombatMakeSkillMapKey(ProjectileSourceSkillIndex) ||
				SkillEntry.CurrentChainStageIndex !=
					ProjectileSourceChainStageIndex ||
				SkillEntry.GetCurrentAbility() != ProjectileSourceAbility.Get() ||
				!SkillEntry.IsInputWindowValid(CurrentTime))
			{
				continue;
			}
		}
		else if (SkillEntry.GetCurrentAbility() != CurrentAbilityInstance)
		{
			continue;
		}

		const FMVSkillDataTableColumn* CurrentSkillData = SkillEntry.GetCurrentSkillData();

		if (!CurrentSkillData ||
			CurrentSkillData->ChainAdvancePolicy !=
				EMVChainAdvancePolicy::OnHitConfirmed ||
			SkillEntry.bCurrentStageHitConfirmed)
		{
			return;
		}

		SkillEntry.bCurrentStageHitConfirmed = true;
		SkillEntry.TryAdvanceChainStage(CurrentTime);
		return;
	}

}

void UMVCombatComponent::HandleHitDeliveryFinished(
	const FMVResolvedHitData& HitData)
{
	if (HitData.Origin != EMVResolvedHitOrigin::AttackCollision)
	{
		return;
	}

	const FMVProjectileExecutionRecord* ProjectileRecord = ProjectileExecutions.Find(HitData.AttackInstanceId);

	if (ProjectileRecord)
	{
		if (IsValid(ProjectileRecord->ExecutionAbility.Get()) &&
			HitData.Attacker.Get() == GetOwner() &&
			IsProjectileSourceWeaponEquipped(*ProjectileRecord, HitData))
		{
			OnValidatedAttackHitAfterDamage.Broadcast(
				HitData, ProjectileRecord->ExecutionAbility.Get());
		}
		return;
	}

	if (IsCurrentAttackHit(HitData))
	{
		OnValidatedAttackHitAfterDamage.Broadcast(
			HitData, CurrentAbilityInstance.Get());
	}
}

bool UMVCombatComponent::IsCurrentAttackHit(
	const FMVResolvedHitData& HitData) const
{
	const AMVCharacterBase* OwnerCharacter =
		Cast<AMVCharacterBase>(GetOwner());

	return IsValid(OwnerCharacter)
		&& HitData.Attacker.Get() == OwnerCharacter
		&& bCurrentAbilityAwaitingCompletion
		&& IsValid(CurrentAbilityInstance.Get())
		&& CurrentAbilityInstance->bAbilityActive
		&& HitData.AttackInstanceId != INDEX_NONE
		&& HitData.AttackInstanceId == CurrentAttackInstanceId;
}

bool UMVCombatComponent::IsBasicAttackAbility(const UMVAbilityBase* Ability) const
{
	if (!IsValid(Ability))
	{
		return false;
	}

	const FMVProjectileExecutionRecord* ProjectileRecord =
		ProjectileExecutions.Find(Ability->GetAttackInstanceId());
	if (ProjectileRecord &&
		ProjectileRecord->ExecutionAbility.Get() == Ability)
	{
		return ProjectileRecord->bWasBasicAttack;
	}

	for (const TPair<FName, FMVSkillEntry>& Pair : BasicAttackMap)
	{
		if (Pair.Value.ContainsAbility(Ability))
		{
			return true;
		}
	}

	return false;
}

bool UMVCombatComponent::IsProjectileSourceWeaponEquipped(
	const FMVProjectileExecutionRecord& Record,
	const FMVResolvedHitData& HitData) const
{
	const AMVCharacterBase* OwnerCharacter = Cast<AMVCharacterBase>(GetOwner());
	if (!IsValid(OwnerCharacter) ||
		HitData.WeaponSnapshot.ItemTag != Record.SourceWeaponItemTag)
	{
		return false;
	}

	const UMVWeaponComponent* Weapon = OwnerCharacter->WeaponComponent.Get();
	const FGameplayTag CurrentWeaponTag = IsValid(Weapon)
		? Weapon->GetEquippedWeaponState().ItemTag
		: FGameplayTag();

	return CurrentWeaponTag == Record.SourceWeaponItemTag;
}

void UMVCombatComponent::ApplyOutgoingAttackDamageModifiers(FMVResolvedHitData& HitData)
{
	if (HitData.Origin != EMVResolvedHitOrigin::AttackCollision)
	{
		return;
	}

	const FMVProjectileExecutionRecord* ProjectileRecord =
		ProjectileExecutions.Find(HitData.AttackInstanceId);

	if (ProjectileRecord)
	{
		if (!IsValid(ProjectileRecord->ExecutionAbility.Get()) ||
			HitData.Attacker.Get() != GetOwner() ||
			!IsProjectileSourceWeaponEquipped(*ProjectileRecord, HitData))
		{
			return;
		}
	}
	else if (!IsCurrentAttackHit(HitData))
	{
		return;
	}

	float ModifiedDamage = HitData.FinalDamage;
	OnModifyOutgoingAttackDamage.Broadcast(HitData, ModifiedDamage);

	if (FMath::IsFinite(ModifiedDamage))
	{
		HitData.FinalDamage = FMath::Max(0.0f, ModifiedDamage);
	}
}

void UMVCombatComponent::HandleActionEnded(
	const FName ActionTableName,
	const FName ActionRowName,
	const bool /*bInterrupted*/)
{
	if (HeavyChargeAttackState.bActive
		&& ActionTableName == HeavyChargeAttackState.ChargeActionTableName
		&& ActionRowName == HeavyChargeAttackState.ChargeRowHandle.RowName)
	{
		ResetHeavyChargeAttackState();
	}

	if (!IsCurrentAbilityAction(ActionTableName, ActionRowName))
	{
		return;
	}

	UMVAbilityBase* EndedAbility = CurrentAbilityInstance.Get();
	if (EndedAbility && EndedAbility->bAbilityActive)
	{
		IMVAbilityInterface::Execute_EndAbility(EndedAbility);
	}

	if (bCurrentAbilityAwaitingCompletion && CurrentAbilityInstance.Get() == EndedAbility)
	{
		HandleAbilityEnded(EndedAbility);
	}

	CloseProjectileExecution(CurrentAttackInstanceId);
	CurrentAbilityInstance = nullptr;
	CurrentAbilityActionTableName = NAME_None;
	CurrentAbilityActionRowName = NAME_None;
	bCurrentAbilityAwaitingCompletion = false;
}

void UMVCombatComponent::RefreshActionMaps()
{
	ResetBasicAttackMap();
	ResetSkillMap();
}

void UMVCombatComponent::ResetBasicAttackMap()
{
	BasicAttackMap.Reset();
	ActiveBasicAttackType = EMVCombatActionTypes::None;
	LastBasicAttackedTime = 0.0;
	bSprintContextualBasicAttackConsumed = false;
	bWasSprintAttackContextActive = false;
	ConsumedDodgeContextActionInstanceId = INDEX_NONE;
	PendingDodgeContextActionInstanceId = INDEX_NONE;
	ClearLastBasicAttackSwingDirection();
	ResetHeavyChargeAttackState();

	const AActor* Owner = GetOwner();
	const UMVWeaponComponent* WeaponComponent = Owner
		? Owner->FindComponentByClass<UMVWeaponComponent>()
		: nullptr;
	if (!WeaponComponent)
	{
		UE_LOG(LogMVCombatComponent, Warning, TEXT("ResetBasicAttackMap: WeaponComponent not found."));
		return;
	}

	const EMVCombatActionTypes TypeArray[] =
	{
		EMVCombatActionTypes::LightAttack,
		EMVCombatActionTypes::DodgeLightAttack,
		EMVCombatActionTypes::SprintLightAttack,
		EMVCombatActionTypes::HeavyAttack,
		EMVCombatActionTypes::DodgeHeavyAttack,
		EMVCombatActionTypes::SprintHeavyAttack,
		EMVCombatActionTypes::ChargeAttack
	};

	for (const EMVCombatActionTypes ActionType : TypeArray)
	{
		FDataTableRowHandle StartingRowHandle;
		if (!WeaponComponent->GetBasicAttackRow(CurrentWeaponStyle, ActionType, StartingRowHandle))
		{
			UE_LOG(
				LogMVCombatComponent,
				Verbose,
				TEXT("ResetBasicAttackMap: WeaponComponent has no row for Style=%d Action=%s."),
				static_cast<int32>(CurrentWeaponStyle),
				*MVCombatActionTypeToString(ActionType));
			continue;
		}

		FMVSkillEntry SkillEntry;
		if (!BuildSkillEntryFromRowHandle(StartingRowHandle, SkillEntry))
		{
			UE_LOG(
				LogMVCombatComponent,
				Warning,
				TEXT("ResetBasicAttackMap: Failed to build Action=%s Start=%s."),
				*MVCombatActionTypeToString(ActionType),
				*MVCombatRowHandleToString(StartingRowHandle));
			continue;
		}

		BasicAttackMap.Add(ActionType, SkillEntry);
		UE_LOG(
			LogMVCombatComponent,
			Log,
			TEXT("ResetBasicAttackMap: Registered Action=%s Start=%s Chain=%s"),
			*MVCombatActionTypeToString(ActionType),
			*MVCombatRowHandleToString(StartingRowHandle),
			*MVCombatSkillRowNamesToString(SkillEntry.SkillRowNames));
	}
}

void UMVCombatComponent::ResetSkillMap()
{
	SkillMap.Reset();

	const EMVSkillSlot SkillSlots[] =
	{
		EMVSkillSlot::PassiveSkill,
		EMVSkillSlot::QSkill,
		EMVSkillSlot::RSkill,
		EMVSkillSlot::WSkill,
		EMVSkillSlot::ESkill
	};

	for (const EMVSkillSlot SkillSlot : SkillSlots)
	{
		RefreshSkillSlot(SkillSlot);
	}
}

void UMVCombatComponent::RefreshSkillSlot(const EMVSkillSlot SkillSlot)
{
	const AActor* Owner = GetOwner();
	const UMVWeaponComponent* WeaponComponent = Owner
		? Owner->FindComponentByClass<UMVWeaponComponent>()
		: nullptr;

	FDataTableRowHandle SkillRowHandle;
	if (SkillSlot == EMVSkillSlot::PassiveSkill)
	{
		if (!WeaponComponent
			|| !WeaponComponent->GetEquippedSkillRow(CurrentWeaponStyle, SkillSlot, SkillRowHandle))
		{
			ClearPassiveSkill();
			return;
		}

		ApplyPassiveSkill(SkillRowHandle);
		return;
	}

	if (!MVCombatIsActiveSkillSlot(SkillSlot))
	{
		UE_LOG(
			LogMVCombatComponent,
			Warning,
			TEXT("RefreshSkillSlot: Unsupported slot %d."),
			static_cast<int32>(SkillSlot));
		return;
	}

	SkillMap.Remove(SkillSlot);

	if (!WeaponComponent)
	{
		UE_LOG(LogMVCombatComponent, Warning, TEXT("RefreshSkillSlot: WeaponComponent not found."));
		return;
	}

	if (!WeaponComponent->GetEquippedSkillRow(CurrentWeaponStyle, SkillSlot, SkillRowHandle))
	{
		UE_LOG(
			LogMVCombatComponent,
			Verbose,
			TEXT("RefreshSkillSlot: No equipped row for Style=%d Slot=%d."),
			static_cast<int32>(CurrentWeaponStyle),
			static_cast<int32>(SkillSlot));
		return;
	}

	FMVSkillEntry SkillEntry;
	if (!BuildSkillEntryFromRowHandle(SkillRowHandle, SkillEntry))
	{
		UE_LOG(
			LogMVCombatComponent,
			Warning,
			TEXT("RefreshSkillSlot: Failed to build Slot=%d Start=%s."),
			static_cast<int32>(SkillSlot),
			*MVCombatRowHandleToString(SkillRowHandle));
		return;
	}

	SkillMap.Add(SkillSlot, SkillEntry);
	UE_LOG(
		LogMVCombatComponent,
		Log,
		TEXT("RefreshSkillSlot: Registered Action=%s Slot=%d Start=%s Chain=%s."),
		*StaticEnum<EMVSkillSlot>()->GetNameStringByValue(static_cast<int64>(SkillSlot)),
		static_cast<int32>(SkillSlot),
		*MVCombatRowHandleToString(SkillRowHandle),
		*MVCombatSkillRowNamesToString(SkillEntry.SkillRowNames));
}

void UMVCombatComponent::ChangeWeapon(EMVEquippedStyle NewStyle)
{
	// Change Current Weapon Style
	CurrentWeaponStyle = NewStyle;

	// Refresh Attack Index Maps
	RefreshActionMaps();
}

bool UMVCombatComponent::BuildSkillEntryFromRowHandle(const FDataTableRowHandle& StartingRowHandle, FMVSkillEntry& OutEntry)
{
	if (!IsValidSkillActionRowHandle(StartingRowHandle, TEXT("BuildSkillEntryFromRowHandle")))
	{
		return false;
	}

	OutEntry = FMVSkillEntry();
	OutEntry.DataTable = const_cast<UDataTable*>(StartingRowHandle.DataTable.Get());

	const UDataTable* CurrentDT = StartingRowHandle.DataTable.Get();
	FName CurrentName = StartingRowHandle.RowName;
	int32 ChainLength = 0;
	int32 MaxChainLength = 100;

	while (!CurrentName.IsNone() && ChainLength < MaxChainLength)
	{
		FMVSkillDataTableColumn* RowData = CurrentDT->FindRow<FMVSkillDataTableColumn>(CurrentName, TEXT("BuildSkillEntryFromRowHandle"));
		if (!RowData || !RowData->AbilityReference)
		{
			UE_LOG(LogTemp, Warning, TEXT("MVCombatComponent::BuildSkillEntryFromRowHandle - Invalid row '%s'"), *CurrentName.ToString());
			break;
		}

		if (ChainLength == 0)
		{
			OutEntry.MainCooldownDuration = RowData->CooldownDuration;
		}

		// Create ability instance
		UMVAbilityBase* AbilityInstance = NewObject<UMVAbilityBase>(this, RowData->AbilityReference);
		if (AbilityInstance)
		{
			AbilityInstance->SetOwner(this);
			AbilityInstance->InitAbility(*RowData);
			OutEntry.AbilityInstances.Add(AbilityInstance);
			OutEntry.SkillRowNames.Add(CurrentName);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("MVCombatComponent::BuildSkillEntryFromRowHandle - Failed to create ability for row '%s'"), *CurrentName.ToString());
			break;
		}

		// Move to next row in chain
		if (RowData->bIsChained && !RowData->NextChainName.IsNone())
		{
			CurrentName = RowData->NextChainName;
		}
		else
		{
			CurrentName = NAME_None;
		}

		ChainLength++;
	}

	if (!CurrentName.IsNone() && ChainLength >= MaxChainLength)
	{
		UE_LOG(LogTemp, Warning, TEXT("MVCombatComponent::BuildSkillEntryFromRowHandle - Chain exceeded max length. StartRow='%s'"), *StartingRowHandle.RowName.ToString());
	}

	OutEntry.bIsChained = OutEntry.AbilityInstances.Num() > 1;
	return OutEntry.AbilityInstances.Num() > 0;
}

bool UMVCombatComponent::IsValidSkillActionRowHandle(const FDataTableRowHandle& RowHandle, const TCHAR* Context) const
{
	if (!RowHandle.DataTable || RowHandle.RowName.IsNone())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("MVCombatComponent::%s - Invalid row handle. DataTable=%s RowName=%s"),
			Context,
			*GetNameSafe(RowHandle.DataTable),
			*RowHandle.RowName.ToString());
		return false;
	}

	if (!RowHandle.DataTable->GetRowStruct() || !RowHandle.DataTable->GetRowStruct()->IsChildOf(FMVSkillDataTableColumn::StaticStruct()))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("MVCombatComponent::%s - DataTable '%s' has invalid row struct '%s'"),
			Context,
			*RowHandle.DataTable->GetName(),
			RowHandle.DataTable->GetRowStruct() ? *RowHandle.DataTable->GetRowStruct()->GetName() : TEXT("None"));
		return false;
	}

	return true;
}

bool UMVCombatComponent::IsCurrentAbilityAction(
	const FName ActionTableName,
	const FName ActionRowName) const
{
	return !CurrentAbilityActionTableName.IsNone()
		&& !CurrentAbilityActionRowName.IsNone()
		&& ActionTableName == CurrentAbilityActionTableName
		&& ActionRowName == CurrentAbilityActionRowName;
}

bool UMVCombatComponent::TryStartActionWithAbility(
	FMVSkillEntry& ActionEntry,
	const FDataTableRowHandle& RowHandle,
	FName StartSection,
	const bool bForceTransition,
	const float TransitionBlendOutTime)
{
	if (!IsValidSkillActionRowHandle(RowHandle, TEXT("TryStartActionWithAbility")))
	{
		return false;
	}

	AMVCharacterBase* Owner = Cast<AMVCharacterBase>(GetOwner());
	if (!Owner)
	{
		return false;
	}
	
	UMVActionComponent* ActionComponent = Owner->ActionComponent;
	if (!ActionComponent)
	{
		return false;
	}

	UMVInputManagerComponent* InputManager = Owner->InputManagerComponent;
	const bool bRecoveryOpen = InputManager ? InputManager->IsRecoveryEscapeWindowOpen() : true;
	const bool bIsActionRunning = ActionComponent->IsActionRunning();
	const bool bCanInterrupt = ActionComponent->CanInterruptActiveAction();

	const FMVSkillDataTableColumn* SpeedRow =
		RowHandle.DataTable->FindRow<FMVSkillDataTableColumn>(
			RowHandle.RowName, TEXT("TryStartActionWithAbility"), false);
	const bool bUseAttackSpeed = SpeedRow && SpeedRow->bUseAttackSpeed;

	const auto GetPlayRateMultiplier = [this, bUseAttackSpeed]() -> float
	{
		if (!bUseAttackSpeed || !IsValid(StatComponent.Get()))
		{
			return 1.0f;
		}

		const float Speed = StatComponent->AttackSpeed;
		return FMath::IsFinite(Speed) && Speed > 0.0f ? Speed : 1.0f;
	};

	const auto PrepareCurrentAbility =
	[this, &ActionEntry, &RowHandle, Owner]() -> UMVAbilityBase*
	{
		UMVAbilityBase* NextAbility = ActionEntry.GetCurrentAbility();
		PreviousAbilityInstance = CurrentAbilityInstance;
		if (PreviousAbilityInstance && PreviousAbilityInstance->Implements<UMVAbilityInterface>())
		{
			IMVAbilityInterface::Execute_EndAbility(PreviousAbilityInstance);
			CloseProjectileExecution(PreviousAbilityInstance->GetAttackInstanceId());
		}

		CurrentAbilityInstance = NextAbility;
		bCurrentAbilityHitConfirmed = false;
		bCurrentAbilityAwaitingCompletion = CurrentAbilityInstance != nullptr;
		CurrentAbilityActionTableName = MVCombatActionTableNameFromDataTable(RowHandle.DataTable);
		CurrentAbilityActionRowName = RowHandle.RowName;
		if (CurrentAbilityInstance)
		{
			int32 SourceSkillIndex = INDEX_NONE;
			for (int32 SkillIndex = 0; SkillIndex < MVCombatSkillSlots::Count; ++SkillIndex)
			{
				if (SkillMap.Find(MVCombatMakeSkillMapKey(SkillIndex)) == &ActionEntry)
				{
					SourceSkillIndex = SkillIndex;
					break;
				}
			}

			const FGameplayTag SourceWeaponItemTag =
				IsValid(Owner->WeaponComponent.Get())
					? Owner->WeaponComponent->GetEquippedWeaponState().ItemTag
					: FGameplayTag();

			CurrentAttackInstanceId = ++NextAttackInstanceId;
			CurrentAbilityInstance->SetAttackInstanceId(CurrentAttackInstanceId);
			CurrentAbilityInstance->PrepareAbilityExecution();
			CurrentAbilityInstance->SetExecutionSource(
				SourceSkillIndex, SourceWeaponItemTag);
		}

		return NextAbility;
	};

	const auto ClearPreparedAbilityOnFailure =
		[this](const UMVAbilityBase* PreparedAbility)
		{
			if (CurrentAbilityInstance.Get() == PreparedAbility)
			{
				CurrentAbilityInstance = nullptr;
				CurrentAbilityActionTableName = NAME_None;
				CurrentAbilityActionRowName = NAME_None;
				CurrentAttackInstanceId = INDEX_NONE;
				bCurrentAbilityAwaitingCompletion = false;
			}
		};

	// If an action is running, only allow transition when recovery is open or action is interruptible.
	if (bIsActionRunning)
	{
		if (bForceTransition || (bRecoveryOpen && bCanInterrupt))
		{
			const UMVAbilityBase* PreparedAbility = PrepareCurrentAbility();
			const float PlayRateMultiplier = GetPlayRateMultiplier();
			const bool bStarted = ActionComponent->TryTransitionActionFromRowHandle(
				RowHandle,
				StartSection,
				FMath::Max(0.0f, TransitionBlendOutTime),
				PlayRateMultiplier);
			if (!bStarted)
			{
				ClearPreparedAbilityOnFailure(PreparedAbility);
			}
			return bStarted;
		}
		else
		{
			// Cannot transition now -> keep buffered input (return false)
			return false;
		}
	}
	else
	{
		// No active action -> start normally
		const UMVAbilityBase* PreparedAbility = PrepareCurrentAbility();
		const float PlayRateMultiplier = GetPlayRateMultiplier();
		const bool bStarted = ActionComponent->TryStartActionFromRowHandle(
			RowHandle,
			StartSection,
			0.1f,
			PlayRateMultiplier);
		if (!bStarted)
		{
			ClearPreparedAbilityOnFailure(PreparedAbility);
		}
		return bStarted;
	}
}

bool UMVCombatComponent::IsRSkillGaugeReady() const
{
	return bUseRSkillGauge
	&& RSkillGaugeMax > KINDA_SMALL_NUMBER
	&& CurrentRSkillGauge >= RSkillGaugeMax - KINDA_SMALL_NUMBER;
}

bool UMVCombatComponent::IsCurrentAbilityRSkill() const
{
	if (!CurrentAbilityInstance)
	{
		return false;
	}

	const FMVSkillEntry* RSkillEntry = SkillMap.Find(EMVSkillSlot::RSkill);

	return RSkillEntry && RSkillEntry->ContainsAbility(CurrentAbilityInstance);
}

void UMVCombatComponent::AddRSkillGauge(float GainMultiplier)
{
	if (!bUseRSkillGauge || RSkillGaugeMax <= KINDA_SMALL_NUMBER || GainMultiplier <= 0.0f)
	{
		return;
	}

	const float GainAmount = FMath::Max(0.0f, RSkillGaugeGainPerHit) * GainMultiplier;

	CurrentRSkillGauge = FMath::Clamp(CurrentRSkillGauge + GainAmount, 0.0f, RSkillGaugeMax);
}

void UMVCombatComponent::ResetRSkillGauge()
{
	CurrentRSkillGauge = 0.0f;
}

void UMVCombatComponent::HandleOwnerDeathStarted(const FMVDeathContext& /*DeathContext*/)
{
	ResetRSkillGauge();
}

bool UMVCombatComponent::CanConsumeActionCost(const FMVSkillDataTableColumn* SkillData) const
{
	if (!SkillData)
	{
		return false;
	}

	if (!StatComponent)
	{
		return false;
	}

	if (!StatComponent->CanConsumeHP(SkillData->HpCost))
	{
		return false;
	}

	if (SkillData->StaminaCost > 0.0f && !StatComponent->HasAnyStamina())
	{
		return false;
	}
	
	if (!StatComponent->HasMP(SkillData->MpCost))
	{
		return false;
	}

	return true;
}

bool UMVCombatComponent::IsBasicAttackActionType(const EMVCombatActionTypes ActionType) const
{
	return ActionType == EMVCombatActionTypes::LightAttack
		|| ActionType == EMVCombatActionTypes::HeavyAttack
		|| ActionType == EMVCombatActionTypes::ChargeAttack
		|| ActionType == EMVCombatActionTypes::SprintLightAttack
		|| ActionType == EMVCombatActionTypes::SprintHeavyAttack
		|| ActionType == EMVCombatActionTypes::DodgeLightAttack
		|| ActionType == EMVCombatActionTypes::DodgeHeavyAttack;
}

int32 UMVCombatComponent::SelectBasicAttackChainStageForSwing(const FMVSkillEntry& ActionEntry) const
{
	const int32 CurrentStageIndex = ActionEntry.SkillRowNames.IsValidIndex(ActionEntry.CurrentChainStageIndex)
		? ActionEntry.CurrentChainStageIndex
		: 0;
	const EMVAttackSwingDirection DesiredSwingDirection =
		MVCombatGetOppositeSwingDirection(LastBasicAttackSwingDirection);
	if (DesiredSwingDirection == EMVAttackSwingDirection::None || !ActionEntry.DataTable)
	{
		return CurrentStageIndex;
	}

	for (int32 StageIndex = CurrentStageIndex; StageIndex < ActionEntry.SkillRowNames.Num(); ++StageIndex)
	{
		const FMVSkillDataTableColumn* SkillData = GetBasicAttackSkillDataAtStage(ActionEntry, StageIndex);
		if (SkillData && SkillData->SwingDirection == DesiredSwingDirection)
		{
			return StageIndex;
		}
	}

	return CurrentStageIndex;
}

const FMVSkillDataTableColumn* UMVCombatComponent::GetBasicAttackSkillDataAtStage(
	const FMVSkillEntry& ActionEntry,
	const int32 ChainStageIndex) const
{
	if (!ActionEntry.DataTable || !ActionEntry.SkillRowNames.IsValidIndex(ChainStageIndex))
	{
		return nullptr;
	}

	return ActionEntry.DataTable->FindRow<FMVSkillDataTableColumn>(
		ActionEntry.SkillRowNames[ChainStageIndex],
		TEXT("GetBasicAttackSkillDataAtStage"),
		false);
}

void UMVCombatComponent::UpdateLastBasicAttackSwingDirection(const FDataTableRowHandle& RowHandle)
{
	LastBasicAttackSwingDirection = EMVAttackSwingDirection::None;
	if (!RowHandle.DataTable || RowHandle.RowName.IsNone())
	{
		return;
	}

	const FMVSkillDataTableColumn* SkillData = RowHandle.DataTable->FindRow<FMVSkillDataTableColumn>(
		RowHandle.RowName,
		TEXT("UpdateLastBasicAttackSwingDirection"),
		false);
	if (SkillData)
	{
		LastBasicAttackSwingDirection = SkillData->SwingDirection;
	}
}

void UMVCombatComponent::ClearLastBasicAttackSwingDirection()
{
	LastBasicAttackSwingDirection = EMVAttackSwingDirection::None;
}

void UMVCombatComponent::ResetOtherBasicAttackChains(const EMVCombatActionTypes ExceptActionType)
{
	for (TPair<EMVCombatActionTypes, FMVSkillEntry>& Pair : BasicAttackMap)
	{
		if (Pair.Key != ExceptActionType)
		{
			Pair.Value.ResetChain();
		}
	}
}

void UMVCombatComponent::MarkBasicAttackChainStarted(
	FMVSkillEntry& ActionEntry,
	const EMVCombatActionTypes ActionType,
	const int32 StartedChainStageIndex,
	const float CurrentTime)
{
	const int32 ValidStartedChainStageIndex = ActionEntry.SkillRowNames.IsValidIndex(StartedChainStageIndex)
		? StartedChainStageIndex
		: 0;

	ActionEntry.CurrentChainStageIndex = ValidStartedChainStageIndex;
	if (!ActionEntry.bChainActive)
	{
		ActionEntry.ActivateChain(CurrentTime);
		ActionEntry.CurrentChainStageIndex = ValidStartedChainStageIndex;
	}

	ActionEntry.TryAdvanceChainStage(CurrentTime);
	ResetOtherBasicAttackChains(ActionType);
	LastBasicAttackedTime = CurrentTime;
	ActiveBasicAttackType = ActionType;
}

bool UMVCombatComponent::IsHeavyChargeBasicAttackType(const EMVCombatActionTypes ActionType) const
{
	return ActionType == EMVCombatActionTypes::HeavyAttack
		|| ActionType == EMVCombatActionTypes::ChargeAttack;
}

int32 UMVCombatComponent::ResolveHeavyChargeStartChainStageIndex(const FMVSkillEntry& ChargeEntry) const
{
	int32 ChainStageIndex = ChargeEntry.SkillRowNames.IsValidIndex(ChargeEntry.CurrentChainStageIndex)
		? ChargeEntry.CurrentChainStageIndex
		: 0;

	if (!IsHeavyChargeBasicAttackType(ActiveBasicAttackType))
	{
		return ChainStageIndex;
	}

	const FMVSkillEntry* ActiveEntry = BasicAttackMap.Find(ActiveBasicAttackType);
	if (ActiveEntry && ActiveEntry->SkillRowNames.IsValidIndex(ActiveEntry->CurrentChainStageIndex))
	{
		ChainStageIndex = ActiveEntry->CurrentChainStageIndex;
	}

	return ChainStageIndex;
}

int32 UMVCombatComponent::ResolveHeavyChargeEarlyReleaseChainStageIndex(const FMVSkillEntry& HeavyEntry) const
{
	const bool bContinueHeavyChain = ActiveBasicAttackType == EMVCombatActionTypes::HeavyAttack
		&& HeavyEntry.bChainActive;
	const bool bUseChargeChainStage = !bContinueHeavyChain
		&& HeavyEntry.SkillRowNames.IsValidIndex(HeavyChargeAttackState.ChargeChainStageIndex);
	int32 CurrentStageIndex = 0;
	if (bContinueHeavyChain && HeavyEntry.SkillRowNames.IsValidIndex(HeavyEntry.CurrentChainStageIndex))
	{
		CurrentStageIndex = HeavyEntry.CurrentChainStageIndex;
	}
	else if (bUseChargeChainStage)
	{
		CurrentStageIndex = HeavyChargeAttackState.ChargeChainStageIndex;
	}

	const EMVAttackSwingDirection DesiredSwingDirection =
		MVCombatGetOppositeSwingDirection(LastBasicAttackSwingDirection);
	if (DesiredSwingDirection == EMVAttackSwingDirection::None || !HeavyEntry.DataTable)
	{
		return CurrentStageIndex;
	}

	for (int32 StageIndex = CurrentStageIndex; StageIndex < HeavyEntry.SkillRowNames.Num(); ++StageIndex)
	{
		const FMVSkillDataTableColumn* SkillData = GetBasicAttackSkillDataAtStage(HeavyEntry, StageIndex);
		if (SkillData && SkillData->SwingDirection == DesiredSwingDirection)
		{
			return StageIndex;
		}
	}

	if (!bContinueHeavyChain && !bUseChargeChainStage)
	{
		for (int32 StageIndex = 0; StageIndex < CurrentStageIndex; ++StageIndex)
		{
			const FMVSkillDataTableColumn* SkillData = GetBasicAttackSkillDataAtStage(HeavyEntry, StageIndex);
			if (SkillData && SkillData->SwingDirection == DesiredSwingDirection)
			{
				return StageIndex;
			}
		}
	}

	return CurrentStageIndex;
}

bool UMVCombatComponent::TryBeginHeavyChargeAttack(const float /*HeldSeconds*/)
{
	if (HeavyChargeAttackState.bActive)
	{
		return true;
	}

	UpdateContextualBasicAttackResets();

	if (IsDodgeAttackContext())
	{
		return TryBasicCombatAction(EMVCombatActionTypes::HeavyAttack);
	}

	if (ShouldSuppressChargeAttackInputForSprint())
	{
		const bool bCanStartSprintHeavyAttack = IsSprintAttackContext()
			&& !bSprintContextualBasicAttackConsumed;
		return bCanStartSprintHeavyAttack
			? TryBasicCombatAction(EMVCombatActionTypes::HeavyAttack)
			: true;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	FMVSkillEntry* ChargeEntry = BasicAttackMap.Find(EMVCombatActionTypes::ChargeAttack);
	if (!ChargeEntry)
	{
		return false;
	}
	FMVSkillEntry* HeavyEntry = BasicAttackMap.Find(EMVCombatActionTypes::HeavyAttack);

	const float CurrentTime = World->GetTimeSeconds();
	if (CurrentTime - LastBasicAttackedTime > ResetBasicAttackTime)
	{
		for (TPair<EMVCombatActionTypes, FMVSkillEntry>& Pair : BasicAttackMap)
		{
			Pair.Value.ResetChain();
		}
		ActiveBasicAttackType = EMVCombatActionTypes::None;
		ClearLastBasicAttackSwingDirection();
	}

	const bool bContinueHeavyChargeChain = IsHeavyChargeBasicAttackType(ActiveBasicAttackType);
	if (ActiveBasicAttackType != EMVCombatActionTypes::None && !bContinueHeavyChargeChain)
	{
		ResetOtherBasicAttackChains(EMVCombatActionTypes::ChargeAttack);
		ChargeEntry->ResetChain();
		if (HeavyEntry)
		{
			HeavyEntry->ResetChain();
		}
	}

	const int32 OriginalChainStageIndex = ChargeEntry->CurrentChainStageIndex;
	ChargeEntry->CurrentChainStageIndex = ResolveHeavyChargeStartChainStageIndex(*ChargeEntry);
	const int32 SelectedChainStageIndex = SelectBasicAttackChainStageForSwing(*ChargeEntry);
	ChargeEntry->CurrentChainStageIndex = SelectedChainStageIndex;
	const FDataTableRowHandle ChargeRowHandle = ChargeEntry->GetCurrentActionRowHandle();
	const FMVSkillDataTableColumn* ChargeSkillData = ChargeEntry->GetCurrentSkillData();
	if (!CanConsumeActionCost(ChargeSkillData))
	{
		ChargeEntry->CurrentChainStageIndex = OriginalChainStageIndex;
		return false;
	}

	const bool bStarted = TryStartActionWithAbility(*ChargeEntry, ChargeRowHandle, HeavyChargeStartSection);
	if (!bStarted)
	{
		ChargeEntry->CurrentChainStageIndex = OriginalChainStageIndex;
		return false;
	}

	HeavyChargeAttackState = FMVCombatHeavyChargeAttackRuntimeState();
	HeavyChargeAttackState.bActive = true;
	HeavyChargeAttackState.StartedWorldTime = CurrentTime;
	HeavyChargeAttackState.CommitTime = ResolveHeavyChargeCommitTime(ChargeSkillData);
	HeavyChargeAttackState.ChargeActionTableName = MVCombatActionTableNameFromDataTable(ChargeRowHandle.DataTable);
	HeavyChargeAttackState.ChargeActionType = EMVCombatActionTypes::ChargeAttack;
	HeavyChargeAttackState.ChargeChainStageIndex = SelectedChainStageIndex;
	HeavyChargeAttackState.ChargeRowHandle = ChargeRowHandle;
	return true;
}

bool UMVCombatComponent::TryUpdateHeavyChargeAttack(const float HeldSeconds)
{
	if (!HeavyChargeAttackState.bActive)
	{
		return false;
	}

	if (!IsHeavyChargeActionRunning())
	{
		ResetHeavyChargeAttackState();
		return false;
	}

	TryCommitHeavyChargeAttack(HeldSeconds);
	return true;
}

bool UMVCombatComponent::TryReleaseHeavyChargeAttack(const float HeldSeconds, const bool bCanceled)
{
	if (!HeavyChargeAttackState.bActive)
	{
		return false;
	}

	const float EffectiveHeldSeconds = GetWorld()
		? FMath::Max(0.0f, GetWorld()->GetTimeSeconds() - HeavyChargeAttackState.StartedWorldTime)
		: FMath::Max(0.0f, HeldSeconds);
	const bool bShouldCommitCharge = HeavyChargeAttackState.bCommitted
		|| (!bCanceled && EffectiveHeldSeconds >= HeavyChargeAttackState.CommitTime);

	if (bShouldCommitCharge)
	{
		TryCommitHeavyChargeAttack(EffectiveHeldSeconds);
		ResetHeavyChargeAttackState();
		return true;
	}

	return TryEarlyReleaseHeavyChargeAttack(EffectiveHeldSeconds);
}

bool UMVCombatComponent::TryCommitHeavyChargeAttack(const float /*HeldSeconds*/)
{
	if (!HeavyChargeAttackState.bActive)
	{
		return false;
	}

	if (HeavyChargeAttackState.bCommitted)
	{
		return true;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	const float EffectiveHeldSeconds =
		FMath::Max(0.0f, World->GetTimeSeconds() - HeavyChargeAttackState.StartedWorldTime);
	if (EffectiveHeldSeconds < HeavyChargeAttackState.CommitTime)
	{
		return false;
	}

	FMVSkillEntry* ChargeEntry = BasicAttackMap.Find(HeavyChargeAttackState.ChargeActionType);
	if (!ChargeEntry || !IsHeavyChargeActionRunning())
	{
		ResetHeavyChargeAttackState();
		return false;
	}

	MarkBasicAttackChainStarted(
		*ChargeEntry,
		HeavyChargeAttackState.ChargeActionType,
		HeavyChargeAttackState.ChargeChainStageIndex,
		World->GetTimeSeconds());
	UpdateLastBasicAttackSwingDirection(HeavyChargeAttackState.ChargeRowHandle);
	HeavyChargeAttackState.bCommitted = true;
	return true;
}

bool UMVCombatComponent::TryEarlyReleaseHeavyChargeAttack(const float /*HeldSeconds*/)
{
	if (!HeavyChargeAttackState.bActive || !IsHeavyChargeActionRunning())
	{
		ResetHeavyChargeAttackState();
		return false;
	}

	FMVSkillEntry* HeavyEntry = nullptr;
	FDataTableRowHandle HeavyRowHandle;
	int32 HeavyChainStageIndex = INDEX_NONE;
	if (!ResolveHeavyChargeEarlyReleaseRow(HeavyEntry, HeavyRowHandle, HeavyChainStageIndex)
		|| !HeavyEntry)
	{
		ResetHeavyChargeAttackState();
		return false;
	}

	const FMVSkillDataTableColumn* ChargeSkillData = HeavyChargeAttackState.ChargeRowHandle.DataTable
		? HeavyChargeAttackState.ChargeRowHandle.DataTable->FindRow<FMVSkillDataTableColumn>(
			HeavyChargeAttackState.ChargeRowHandle.RowName,
			TEXT("TryEarlyReleaseHeavyChargeAttack"),
			false)
		: nullptr;
	const FName EarlyReleaseStartSection = ChargeSkillData && !ChargeSkillData->EarlyReleaseStartSection.IsNone()
		? ChargeSkillData->EarlyReleaseStartSection
		: FName(TEXT("Attack"));
	const float EarlyReleaseBlendOutTime = ChargeSkillData
		? FMath::Max(0.0f, ChargeSkillData->EarlyReleaseBlendOutTime)
		: 0.1f;

	const int32 OriginalChainStageIndex = HeavyEntry->CurrentChainStageIndex;
	HeavyEntry->CurrentChainStageIndex = HeavyChainStageIndex;
	if (!CanConsumeActionCost(HeavyEntry->GetCurrentSkillData()))
	{
		HeavyEntry->CurrentChainStageIndex = OriginalChainStageIndex;
		ResetHeavyChargeAttackState();
		return false;
	}

	const bool bStarted = TryStartActionWithAbility(
		*HeavyEntry,
		HeavyRowHandle,
		EarlyReleaseStartSection,
		true,
		EarlyReleaseBlendOutTime);
	if (!bStarted)
	{
		HeavyEntry->CurrentChainStageIndex = OriginalChainStageIndex;
		ResetHeavyChargeAttackState();
		return false;
	}

	if (UWorld* World = GetWorld())
	{
		MarkBasicAttackChainStarted(
			*HeavyEntry,
			EMVCombatActionTypes::HeavyAttack,
			HeavyChainStageIndex,
			World->GetTimeSeconds());
	}
	UpdateLastBasicAttackSwingDirection(HeavyRowHandle);
	ResetHeavyChargeAttackState();
	return true;
}

bool UMVCombatComponent::IsHeavyChargeActionRunning() const
{
	const AMVCharacterBase* OwnerCharacter = Cast<AMVCharacterBase>(GetOwner());
	const UMVActionComponent* ActionComponent = OwnerCharacter ? OwnerCharacter->ActionComponent : nullptr;
	return HeavyChargeAttackState.bActive
		&& ActionComponent
		&& ActionComponent->IsActionRunning()
		&& ActionComponent->GetActiveActionTableName() == HeavyChargeAttackState.ChargeActionTableName
		&& ActionComponent->GetActiveActionRowName() == HeavyChargeAttackState.ChargeRowHandle.RowName;
}

float UMVCombatComponent::ResolveHeavyChargeCommitTime(const FMVSkillDataTableColumn* ChargeSkillData) const
{
	if (ChargeSkillData && ChargeSkillData->ChargeCommitTime > 0.0f)
	{
		return ChargeSkillData->ChargeCommitTime;
	}

	return FMath::Max(0.0f, DefaultHeavyChargeCommitTime);
}

bool UMVCombatComponent::ResolveHeavyChargeEarlyReleaseRow(
	FMVSkillEntry*& OutHeavyEntry,
	FDataTableRowHandle& OutHeavyRowHandle,
	int32& OutHeavyChainStageIndex)
{
	OutHeavyEntry = nullptr;
	OutHeavyRowHandle = FDataTableRowHandle();
	OutHeavyChainStageIndex = INDEX_NONE;

	OutHeavyEntry = BasicAttackMap.Find(EMVCombatActionTypes::HeavyAttack);
	if (!OutHeavyEntry || !OutHeavyEntry->DataTable)
	{
		return false;
	}

	int32 ChainStageIndex = ResolveHeavyChargeEarlyReleaseChainStageIndex(*OutHeavyEntry);
	if (!OutHeavyEntry->SkillRowNames.IsValidIndex(ChainStageIndex))
	{
		ChainStageIndex = OutHeavyEntry->SkillRowNames.IsValidIndex(0) ? 0 : INDEX_NONE;
	}

	if (ChainStageIndex == INDEX_NONE)
	{
		return false;
	}

	OutHeavyChainStageIndex = ChainStageIndex;
	OutHeavyRowHandle.DataTable = OutHeavyEntry->DataTable.Get();
	OutHeavyRowHandle.RowName = OutHeavyEntry->SkillRowNames[ChainStageIndex];
	return !OutHeavyRowHandle.RowName.IsNone();
}

void UMVCombatComponent::ResetHeavyChargeAttackState()
{
	HeavyChargeAttackState = FMVCombatHeavyChargeAttackRuntimeState();
}

EMVCombatActionTypes UMVCombatComponent::ResolveContextualBasicAttackActionType(
	const EMVCombatActionTypes RequestedActionType)
{
	PendingDodgeContextActionInstanceId = INDEX_NONE;

	const bool bRequestedLightAttack = RequestedActionType == EMVCombatActionTypes::LightAttack;
	const bool bRequestedHeavyAttack = RequestedActionType == EMVCombatActionTypes::HeavyAttack;
	if (!bRequestedLightAttack && !bRequestedHeavyAttack)
	{
		return RequestedActionType;
	}

	const int32 DodgeContextActionInstanceId = GetDodgeAttackContextInstanceId();
	const bool bSprintAttackContext = IsSprintAttackContext();
	if (DodgeContextActionInstanceId != INDEX_NONE
		&& DodgeContextActionInstanceId != ConsumedDodgeContextActionInstanceId)
	{
		PendingDodgeContextActionInstanceId = DodgeContextActionInstanceId;
		return bRequestedLightAttack
			? EMVCombatActionTypes::DodgeLightAttack
			: EMVCombatActionTypes::DodgeHeavyAttack;
	}

	if (bSprintAttackContext && !bSprintContextualBasicAttackConsumed)
	{
		return bRequestedLightAttack
			? EMVCombatActionTypes::SprintLightAttack
			: EMVCombatActionTypes::SprintHeavyAttack;
	}

	return RequestedActionType;
}

void UMVCombatComponent::MarkContextualBasicAttackStarted(const EMVCombatActionTypes StartedActionType)
{
	if (MVCombatIsSprintContextualBasicAttackActionType(StartedActionType))
	{
		bSprintContextualBasicAttackConsumed = true;
		return;
	}

	if (MVCombatIsDodgeContextualBasicAttackActionType(StartedActionType)
		&& PendingDodgeContextActionInstanceId != INDEX_NONE)
	{
		ConsumedDodgeContextActionInstanceId = PendingDodgeContextActionInstanceId;
		PendingDodgeContextActionInstanceId = INDEX_NONE;
	}
}

void UMVCombatComponent::BroadcastCombatActionStarted(
	const EMVCombatActionTypes ActionType,
	const EMVSkillSlot SkillSlot)
{
	FMVCombatActionEvent Event;
	Event.Instigator = GetOwner();
	Event.ActionType = ActionType;
	Event.SkillSlot = SkillSlot;

	const AMVCharacterBase* OwnerCharacter = Cast<AMVCharacterBase>(GetOwner());
	const UMVActionComponent* ActionComponent = OwnerCharacter ? OwnerCharacter->ActionComponent : nullptr;
	if (ActionComponent)
	{
		Event.ActionTableName = ActionComponent->GetActiveActionTableName();
		Event.ActionRowName = ActionComponent->GetActiveActionRowName();
	}

	OnCombatActionStarted.Broadcast(Event);
}

void UMVCombatComponent::UpdateContextualBasicAttackResets()
{
	const AMVCharacterBase* OwnerCharacter = Cast<AMVCharacterBase>(GetOwner());
	if (!OwnerCharacter || !OwnerCharacter->CharacterInputState.WantsToSprint)
	{
		bSprintContextualBasicAttackConsumed = false;
		bWasSprintAttackContextActive = false;
		return;
	}

	const bool bSprintAttackContextActive = IsSprintAttackContext();
	if (bSprintAttackContextActive && !bWasSprintAttackContextActive)
	{
		bSprintContextualBasicAttackConsumed = false;
	}

	bWasSprintAttackContextActive = bSprintAttackContextActive;
}

int32 UMVCombatComponent::GetDodgeAttackContextInstanceId() const
{
	const AMVCharacterBase* OwnerCharacter = Cast<AMVCharacterBase>(GetOwner());
	const UMVActionComponent* ActionComponent = OwnerCharacter ? OwnerCharacter->ActionComponent : nullptr;
	if (!ActionComponent || !ActionComponent->IsActionRunning())
	{
		return INDEX_NONE;
	}

	if (ActionComponent->GetActiveActionDomain() != EMVActionDomain::Dodge)
	{
		return INDEX_NONE;
	}

	return ActionComponent->GetActiveActionInstanceId();
}

bool UMVCombatComponent::IsDodgeAttackContext() const
{
	return GetDodgeAttackContextInstanceId() != INDEX_NONE;
}

bool UMVCombatComponent::IsSprintAttackContext() const
{
	const AMVCharacterBase* OwnerCharacter = Cast<AMVCharacterBase>(GetOwner());
	const UMVActionComponent* ActionComponent = OwnerCharacter ? OwnerCharacter->ActionComponent : nullptr;
	if (!OwnerCharacter || (ActionComponent && ActionComponent->IsActionRunning()))
	{
		return false;
	}

	return OwnerCharacter->Gait == EGait::Sprinting
		&& HasReachedSprintAttackSpeed();
}

bool UMVCombatComponent::ShouldSuppressChargeAttackInputForSprint() const
{
	const AMVCharacterBase* OwnerCharacter = Cast<AMVCharacterBase>(GetOwner());
	if (!OwnerCharacter || !OwnerCharacter->CharacterInputState.WantsToSprint)
	{
		return false;
	}

	return OwnerCharacter->Gait == EGait::Sprinting
		|| bSprintContextualBasicAttackConsumed;
}

bool UMVCombatComponent::HasReachedSprintAttackSpeed() const
{
	if (!StatComponent)
	{
		return false;
	}

	return StatComponent->HasReachedSprintSpeedRatio(SprintAttackMinSpeedRatio);
}
