// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/MVCombatComponent.h"

#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "Combat/MVAbilityBase.h"
#include "Combat/MVProjectileExecutionTracker.h"
#include "Combat/Projectile/MVProjectileAbility.h"
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

bool MVCombatUsesSharedLightChargeChain(const EMVCombatActionTypes ActionType)
{
	return ActionType == EMVCombatActionTypes::LightAttack || ActionType == EMVCombatActionTypes::ChargeAttack;
}

bool MVCombatIsActiveSkillSlot(const EMVSkillSlot SkillSlot)
{
	return SkillSlot == EMVSkillSlot::QSkill
		|| SkillSlot == EMVSkillSlot::RSkill
		|| SkillSlot == EMVSkillSlot::WSkill
		|| SkillSlot == EMVSkillSlot::ESkill;
}

EMVSkillSlot MVCombatLegacySkillIndexToSlot(const int32 SkillIndex)
{
	switch (SkillIndex)
	{
	case MVCombatSkillSlots::Q: return EMVSkillSlot::QSkill;
	case MVCombatSkillSlots::R: return EMVSkillSlot::RSkill;
	case MVCombatSkillSlots::W: return EMVSkillSlot::WSkill;
	case MVCombatSkillSlots::E: return EMVSkillSlot::ESkill;
	default: return EMVSkillSlot::NONE;
	}
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
	
	ProjectileExecutionTracker = NewObject<UMVProjectileExecutionTracker>(this);
	ProjectileExecutionTracker->OnMissed.BindUObject(this, &UMVCombatComponent::HandleTrackedProjectileMiss);

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

	if (ProjectileExecutionTracker)
	{
		ProjectileExecutionTracker->Reset();
		ProjectileExecutionTracker = nullptr;
	}

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
	if (++NextPassiveActivationId == 0)
	{
		++NextPassiveActivationId;
	}
	const uint64 NewActivationId = NextPassiveActivationId;
	ActivePassiveActivationId = NewActivationId;

	IMVAbilityInterface::Execute_StartAbility(NewPassiveAbility, INDEX_NONE);
	if (!NewPassiveAbility->bAbilityActive
		|| ActivePassiveAbilityInstance.Get() != NewPassiveAbility
		|| ActivePassiveActivationId != NewActivationId)
	{
		if (ActivePassiveAbilityInstance.Get() == NewPassiveAbility)
		{
			ActivePassiveAbilityInstance = nullptr;
			ActivePassiveSkillRowHandle = FDataTableRowHandle();
			ActivePassiveActivationId = 0;
		}
		IMVAbilityInterface::Execute_EndAbility(NewPassiveAbility);

		UE_LOG(
			LogMVCombatComponent,
			Warning,
			TEXT("ApplyPassiveSkill: Passive ability did not remain active. Check the Blueprint parent StartAbility and effect definition. Row=%s"),
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
	ActivePassiveActivationId = 0;

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
	if (ChargePreparation.ActionInstanceId != INDEX_NONE)
	{
		const AMVCharacterBase* Owner = Cast<AMVCharacterBase>(GetOwner());
		if (!FindPreparingChargeAction() || (Owner && Owner->InputManagerComponent
			&& Owner->InputManagerComponent->IsGameplayInputBlocked()))
		{
			CancelChargePreparation();
		}
	}
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

bool UMVCombatComponent::ReduceSkillMainCooldown(
	int32 SkillIndex,
	float ReductionFraction,
	int32 TriggerCount,
	float& OutBeforeSeconds,
	float& OutAfterSeconds)
{
	OutBeforeSeconds = 0.0f;
	OutAfterSeconds = 0.0f;

	UWorld* World = GetWorld();
	if (!World
		|| SkillIndex < 0
		|| SkillIndex >= MVCombatSkillSlots::Count
		|| (SkillIndex == MVCombatSkillSlots::R && bUseRSkillGauge)
		|| !FMath::IsFinite(ReductionFraction)
		|| ReductionFraction <= 0.0f
		|| TriggerCount <= 0)
	{
		return false;
	}

	FMVSkillEntry* SkillEntry = SkillMap.Find(MVCombatLegacySkillIndexToSlot(SkillIndex));
	if (!SkillEntry
		|| SkillEntry->MainCooldownDuration <= 0.0f
		|| SkillEntry->MainCooldownEndTime < 0.0f
		|| SkillEntry->bChainActive
		|| (CurrentAbilityInstance
			&& SkillEntry->ContainsAbility(CurrentAbilityInstance.Get())))
	{
		return false;
	}

	const float CurrentTime = World->GetTimeSeconds();
	OutBeforeSeconds = SkillEntry->GetMainCooldownRemaining(CurrentTime);
	if (OutBeforeSeconds <= KINDA_SMALL_NUMBER)
	{
		return false;
	}

	const float Fraction = FMath::Clamp(ReductionFraction, 0.0f, 1.0f);
	const float RemainingMultiplier = FMath::Pow(1.0f - Fraction, TriggerCount);
	OutAfterSeconds = FMath::Clamp(
		OutBeforeSeconds * RemainingMultiplier,
		0.0f,
		OutBeforeSeconds);
	SkillEntry->MainCooldownEndTime = CurrentTime + OutAfterSeconds;
	return true;
}

int32 UMVCombatComponent::ReduceOngoingSkillMainCooldowns(
	float ReductionFraction,
	int32 TriggerCount)
{
	int32 ReducedSkillCount = 0;
	for (int32 SkillIndex = 0; SkillIndex < MVCombatSkillSlots::Count; ++SkillIndex)
	{
		float BeforeSeconds = 0.0f;
		float AfterSeconds = 0.0f;
		if (ReduceSkillMainCooldown(
			SkillIndex,
			ReductionFraction,
			TriggerCount,
			BeforeSeconds,
			AfterSeconds))
		{
			++ReducedSkillCount;
		}
	}
	return ReducedSkillCount;
}

bool UMVCombatComponent::RefundSkillMainCooldownForExecution(
	int32 SkillIndex,
	const UMVAbilityBase* SourceAbility)
{
	const int32 AttackInstanceId = IsValid(SourceAbility)
		? SourceAbility->GetAttackInstanceId() : INDEX_NONE;

	UWorld* World = GetWorld();
	const AMVCharacterBase* OwnerCharacter = Cast<AMVCharacterBase>(GetOwner());
	const UMVWeaponComponent* Weapon = IsValid(OwnerCharacter)
		? OwnerCharacter->WeaponComponent.Get() : nullptr;
	if (!World || !IsValid(SourceAbility) || !IsValid(StatComponent.Get())
		|| StatComponent->IsDead() || !IsValid(Weapon)
		|| SkillIndex < 0 || SkillIndex >= MVCombatSkillSlots::Count
		|| (SkillIndex == MVCombatSkillSlots::R && bUseRSkillGauge)
		|| AttackInstanceId == INDEX_NONE)
	{
		return false;
	}

	const EMVSkillSlot SkillSlot = MVCombatLegacySkillIndexToSlot(SkillIndex);
	FMVSkillEntry* SkillEntry = SkillMap.Find(SkillSlot);
	if (!SkillEntry || SkillEntry->LoadoutRevision == 0
		|| SkillEntry->CurrentUseId == 0)
	{
		return false;
	}

	const FMVProjectileExecutionRecord* ProjectileRecord =
		FindProjectileExecution(AttackInstanceId);
	if (ProjectileRecord)
	{
		if (ProjectileRecord->ExecutionAbility.Get() != SourceAbility
			|| ProjectileRecord->SourceSkillSlot != SkillSlot
			|| ProjectileRecord->SourceSlotRevision != SkillEntry->LoadoutRevision
			|| ProjectileRecord->SourceSkillUseId != SkillEntry->CurrentUseId
			|| ProjectileRecord->SourceLifeId != CurrentLifeId
			|| !IsValid(ProjectileRecord->SourceAbility.Get())
			|| !SkillEntry->ContainsAbility(ProjectileRecord->SourceAbility.Get())
			|| Weapon->GetEquippedWeaponState().ItemTag != ProjectileRecord->SourceWeaponItemTag
			|| SourceAbility->GetSourceWeaponItemTag() != ProjectileRecord->SourceWeaponItemTag)
		{
			return false;
		}
	}
	else if (CurrentAbilityInstance.Get() != SourceAbility
		|| !bCurrentAbilityAwaitingCompletion
		|| CurrentAttackInstanceId != AttackInstanceId
		|| CurrentAttackLifeId != CurrentLifeId
		|| CurrentRefundedAttackInstanceId == AttackInstanceId
		|| CurrentPreparedSkillSlot != SkillSlot
		|| CurrentPreparedSlotRevision != SkillEntry->LoadoutRevision
		|| CurrentPreparedSkillUseId != SkillEntry->CurrentUseId
		|| !SkillEntry->ContainsAbility(SourceAbility)
		|| Weapon->GetEquippedWeaponState().ItemTag != SourceAbility->GetSourceWeaponItemTag())
	{
		return false;
	}

	// 환원 권한을 먼저 소비해 같은 공격의 관통·다발 적중과 재진입을 막는다.
	if (ProjectileRecord)
	{
		if (!ProjectileExecutionTracker->TryConsumeRefund(AttackInstanceId))
		{
			return false;
		}
	}
	else
	{
		CurrentRefundedAttackInstanceId = AttackInstanceId;
	}
	const float CurrentTime = World->GetTimeSeconds();
	SkillEntry->bMainCooldownRefundedForCurrentUse = true;
	// 이미 시작한 주 쿨타임은 끝내고, 아직 시작 전이면 종료 처리에서 면제한다.
	if (SkillEntry->MainCooldownEndTime >= 0.0f)
	{
		SkillEntry->MainCooldownEndTime = CurrentTime;
	}
	return true;
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

bool UMVCombatComponent::TryStartCombatActionFromRowHandle(
	const FDataTableRowHandle& RowHandle,
	FName StartSection)
{
	const AMVCharacterBase* Character = Cast<AMVCharacterBase>(GetOwner());
	if (!Character || !Character->ActionComponent || Character->ActionComponent->IsActionRunning()
		|| !IsValidSkillActionRowHandle(RowHandle, TEXT("TryStartCombatActionFromRowHandle")))
	{
		return false;
	}

	const FMVSkillDataTableColumn* Row = RowHandle.DataTable->FindRow<FMVSkillDataTableColumn>(
		RowHandle.RowName, TEXT("TryStartCombatActionFromRowHandle"));
	if (!Row || !Row->bEnabled || !CanConsumeActionCost(Row))
	{
		return false;
	}

	FMVSkillEntry Entry;
	if (!BuildSkillEntryFromRowHandle(RowHandle, Entry))
	{
		return false;
	}

	DirectActionEntry = MoveTemp(Entry);
	return TryStartActionWithAbility(DirectActionEntry, RowHandle, StartSection);
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
	if (ActionInputTag != MVGameplayTags::Action_Input_LightAttack)
	{
		return false;
	}

	switch (Phase)
	{
	case EMVActionInputPhase::Started:
		return TryBeginLightOrChargeAttack();
	case EMVActionInputPhase::Triggered:
		return TryCommitChargeAttack(HeldSeconds);
	case EMVActionInputPhase::Completed:
		return TryReleaseChargePreparation(HeldSeconds, false);
	case EMVActionInputPhase::Canceled:
		return TryReleaseChargePreparation(HeldSeconds, true);
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
	if (ActionInputTag == MVGameplayTags::Action_Input_LightAttack)
	{
		return TryBasicCombatAction(EMVCombatActionTypes::LightAttack);
	}

	return false;
}

bool UMVCombatComponent::IsCombatActionInputTag(const FGameplayTag ActionInputTag) const
{
	return ActionInputTag == MVGameplayTags::Action_Input_LightAttack
		|| ActionInputTag.MatchesTag(MVGameplayTags::Action_Input_Skill);
}

bool UMVCombatComponent::TryBasicAttack(
	const EMVCombatActionTypes ActionType, const FName StartSection,
	const bool bForceTransition, const float BlendOutTime)
{
	FMVSkillEntry* Entry = BasicAttackMap.Find(ActionType);
	if (!Entry)
	{
		return false;
	}

	const double CurrentTime = GetWorld()->GetTimeSeconds();
	if (!bForceTransition)
	{
		ResetExpiredBasicAttackChain(CurrentTime);
	}

	const int32 OriginalStageIndex = Entry->CurrentChainStageIndex;
	const int32 StageIndex = SelectBasicAttackStage(ActionType, *Entry);
	Entry->CurrentChainStageIndex = StageIndex;
	const FDataTableRowHandle RowHandle = Entry->GetCurrentActionRowHandle();

	if (!CanConsumeActionCost(Entry->GetCurrentSkillData())
		|| !TryStartActionWithAbility(*Entry, RowHandle, StartSection, bForceTransition, BlendOutTime))
	{
		Entry->CurrentChainStageIndex = OriginalStageIndex;
		return false;
	}

	MarkBasicAttackChainStarted(*Entry, ActionType, StageIndex, CurrentTime);
	UpdateLastBasicAttackSwingDirection(RowHandle);
	UE_LOG(LogMVCombatComponent, Log, TEXT("TryBasicAttack: Action=%s Row=%s ChainStage=%d"),
		*MVCombatActionTypeToString(ActionType), *MVCombatRowHandleToString(RowHandle), StageIndex);
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
	const auto TryStartSkillStage = [this, ActionEntry, StartSection](const bool bNewUse)
	{
		const uint64 PreviousUseId = ActionEntry->CurrentUseId;
		const bool bPreviousRefunded = ActionEntry->bMainCooldownRefundedForCurrentUse;
		if (bNewUse)
		{
			ActionEntry->CurrentUseId = ++NextSkillUseId;
			ActionEntry->bMainCooldownRefundedForCurrentUse = false;
		}
		const bool bStarted = TryStartActionWithAbility(
			*ActionEntry, ActionEntry->GetCurrentActionRowHandle(), StartSection);
		if (!bStarted)
		{
			ActionEntry->CurrentUseId = PreviousUseId;
			ActionEntry->bMainCooldownRefundedForCurrentUse = bPreviousRefunded;
		}
		return bStarted;
	};

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

			if (TryStartSkillStage(true))
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

			if (TryStartSkillStage(false))
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

		if (TryStartSkillStage(true))
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
	if (!ProjectileExecutionTracker || !IsValid(Projectile) ||
		!IsValid(SourceAbility) ||
		CurrentAbilityInstance.Get() != SourceAbility ||
		!SourceAbility->bAbilityActive ||
		CurrentAttackInstanceId == INDEX_NONE ||
		SourceAbility->GetAttackInstanceId() != CurrentAttackInstanceId ||
		Projectile->GetAttackInstanceId() != CurrentAttackInstanceId)
	{
		return false;
	}
	if (!IsValid(StatComponent.Get()) || StatComponent->IsDead()
		|| CurrentAttackLifeId != CurrentLifeId)
	{
		return false;
	}

	FMVProjectileExecutionRecord Source;
	Source.SourceAbility = SourceAbility;
	Source.SourceRowHandle = CurrentAbilityRowHandle;
	Source.SourceSkillSlot = CurrentPreparedSkillSlot;
	Source.SourceChainStageIndex = CurrentPreparedChainStageIndex;
	Source.SourceSlotRevision = CurrentPreparedSlotRevision;
	Source.SourceSkillUseId = CurrentPreparedSkillUseId;
	Source.SourceLifeId = CurrentLifeId;
	Source.SourceWeaponItemTag = SourceAbility->GetSourceWeaponItemTag();
	Source.PassiveActivationId = CurrentAttackPassiveActivationId;
	Source.bWasBasicAttack = IsBasicAttackAbility(SourceAbility);
	Source.bWasRSkill = IsCurrentAbilityRSkill();
	return ProjectileExecutionTracker->Register(Projectile, Source);
}

const FMVProjectileExecutionRecord* UMVCombatComponent::FindProjectileExecution(int32 AttackInstanceId) const
{
	return ProjectileExecutionTracker ? ProjectileExecutionTracker->Find(AttackInstanceId) : nullptr;
}

void UMVCombatComponent::CloseProjectileExecution(int32 AttackInstanceId)
{
	if (ProjectileExecutionTracker)
	{
		ProjectileExecutionTracker->CloseAction(AttackInstanceId);
	}
}

void UMVCombatComponent::HandleTrackedProjectileMiss(const FMVProjectileExecutionRecord& Record)
{
	UMVAbilityBase* MissedAbility = Record.ExecutionAbility.Get();
	const AMVCharacterBase* OwnerCharacter = Cast<AMVCharacterBase>(GetOwner());
	const UMVWeaponComponent* Weapon = IsValid(OwnerCharacter)
		? OwnerCharacter->WeaponComponent.Get()
		: nullptr;
	const FGameplayTag EquippedWeaponItemTag = IsValid(Weapon)
		? Weapon->GetEquippedWeaponState().ItemTag
		: FGameplayTag();

	if (IsValid(MissedAbility) &&
		Record.SourceLifeId == CurrentLifeId &&
		IsValid(StatComponent.Get()) && !StatComponent->IsDead() &&
		EquippedWeaponItemTag == Record.SourceWeaponItemTag)
	{
		OnAttackMissed.Broadcast(MissedAbility);
	}
}

void UMVCombatComponent::NotifyProjectileAbilityStarted(const UMVAbilityBase* StartedAbility)
{
	if (IsValid(StartedAbility) &&
		CurrentAbilityInstance.Get() == StartedAbility &&
		IsValid(StatComponent.Get()) && !StatComponent->IsDead() &&
		CurrentAttackLifeId == CurrentLifeId &&
		CurrentAttackInstanceId != INDEX_NONE &&
		CurrentAttackInstanceId == StartedAbility->GetAttackInstanceId())
	{
		bCurrentAbilityAwaitingCompletion = true;
	}
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
	if (!bHandledAbility &&
		DirectActionEntry.ContainsAbility(EndedAbility) &&
		Cast<UMVProjectileAbility>(EndedAbility))
	{
		bHandledAbility = true;
	}

	if (bHandledAbility && CurrentAbilityInstance.Get() == EndedAbility)
	{
		if (bCurrentAbilityAwaitingCompletion
			&& EndedAbility->bAbilityCostConsumed
			&& !bCurrentAbilityHitConfirmed
			&& CurrentAttackLifeId == CurrentLifeId
			&& IsValid(StatComponent.Get()) && !StatComponent->IsDead())
		{
			const bool bProjectileMissQueued = ProjectileExecutionTracker &&
				ProjectileExecutionTracker->QueueMiss(EndedAbility->GetAttackInstanceId(), EndedAbility);
			if (!bProjectileMissQueued && !Cast<UMVProjectileAbility>(EndedAbility))
			{
				OnAttackMissed.Broadcast(const_cast<UMVAbilityBase*>(EndedAbility));
			}
		}

		bCurrentAbilityAwaitingCompletion = false;
	}
}

void UMVCombatComponent::HandleHitResolved(const FMVResolvedHitData& HitData)
{
	if (HitData.Origin != EMVResolvedHitOrigin::AttackCollision)
	{
		return;
	}

	AMVCharacterBase* OwnerCharacter = Cast<AMVCharacterBase>(GetOwner());
	const bool bCurrentAttackHit = IsCurrentAttackHit(HitData);

	const FMVProjectileExecutionRecord* ProjectileRecord =
		FindProjectileExecution(HitData.AttackInstanceId);

	if (!bCurrentAttackHit && !ProjectileRecord)
	{
		return;
	}
	if (!IsValid(StatComponent.Get()) || StatComponent->IsDead()
		|| (ProjectileRecord && ProjectileRecord->SourceLifeId != CurrentLifeId))
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
	const EMVSkillSlot ProjectileSourceSkillSlot = bProjectileExecution
		? ProjectileRecord->SourceSkillSlot
		: EMVSkillSlot::NONE;
	const int32 ProjectileSourceChainStageIndex = bProjectileExecution
		? ProjectileRecord->SourceChainStageIndex
		: INDEX_NONE;
	const FGameplayTag ProjectileSourceWeaponItemTag = bProjectileExecution
		? ProjectileRecord->SourceWeaponItemTag
		: FGameplayTag();
	const uint64 HitLifeId = bProjectileExecution
		? ProjectileRecord->SourceLifeId
		: CurrentAttackLifeId;

	const bool bWasRSkill = ProjectileRecord
		? ProjectileRecord->bWasRSkill
		: IsCurrentAbilityRSkill();

	const bool bPublishPassiveEvents =
		!ProjectileRecord ||
		IsProjectileSourceWeaponEquipped(*ProjectileRecord, HitData);

	// 이벤트가 다른 액션을 시작하더라도 이 공격의 적중 여부를 먼저 고정한다.
	if (ProjectileRecord)
	{
		ProjectileExecutionTracker->MarkHit(HitData.AttackInstanceId);
	}
	if (bCurrentAttackHit)
	{
		bCurrentAbilityHitConfirmed = true;
	}

	if (bPublishPassiveEvents)
	{
		OnValidatedAttackHit.Broadcast(HitData, EventAbility);
	}
	if (!IsValid(StatComponent.Get()) || StatComponent->IsDead()
		|| HitLifeId != CurrentLifeId)
	{
		return;
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

	const float CurrentTime = GetWorld()->GetTimeSeconds();
	FMVSkillEntry* HitSkillEntry = nullptr;
	if (bProjectileExecution)
	{
		if (HitData.AttackInstanceId != CurrentAttackInstanceId ||
			!MVCombatIsActiveSkillSlot(ProjectileSourceSkillSlot) ||
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

		HitSkillEntry = SkillMap.Find(ProjectileSourceSkillSlot);
		if (!HitSkillEntry ||
			HitSkillEntry->CurrentChainStageIndex != ProjectileSourceChainStageIndex ||
			HitSkillEntry->GetCurrentAbility() != ProjectileSourceAbility.Get() ||
			!HitSkillEntry->IsInputWindowValid(CurrentTime))
		{
			return;
		}
	}
	else if (IsCurrentAttackHit(HitData))
	{
		for (TPair<EMVSkillSlot, FMVSkillEntry>& Pair : SkillMap)
		{
			if (Pair.Value.GetCurrentAbility() == CurrentAbilityInstance)
			{
				HitSkillEntry = &Pair.Value;
				break;
			}
		}
	}

	if (!HitSkillEntry)
	{
		return;
	}

	const FMVSkillDataTableColumn* CurrentSkillData = HitSkillEntry->GetCurrentSkillData();
	if (CurrentSkillData &&
		CurrentSkillData->ChainAdvancePolicy == EMVChainAdvancePolicy::OnHitConfirmed &&
		!HitSkillEntry->bCurrentStageHitConfirmed)
	{
		HitSkillEntry->bCurrentStageHitConfirmed = true;
		HitSkillEntry->TryAdvanceChainStage(CurrentTime);
	}
}

void UMVCombatComponent::HandleHitDeliveryFinished(
	const FMVResolvedHitData& HitData)
{
	if (HitData.Origin != EMVResolvedHitOrigin::AttackCollision
		|| !IsValid(StatComponent.Get()) || StatComponent->IsDead())
	{
		return;
	}

	const FMVProjectileExecutionRecord* ProjectileRecord = FindProjectileExecution(HitData.AttackInstanceId);

	if (ProjectileRecord)
	{
		if (ProjectileRecord->SourceLifeId == CurrentLifeId &&
			IsValid(ProjectileRecord->ExecutionAbility.Get()) &&
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
		&& IsValid(StatComponent.Get()) && !StatComponent->IsDead()
		&& bCurrentAbilityAwaitingCompletion
		&& CurrentAttackLifeId == CurrentLifeId
		&& IsValid(CurrentAbilityInstance.Get())
		&& CurrentAbilityInstance->bAbilityActive
		&& HitData.AttackInstanceId != INDEX_NONE
		&& HitData.AttackInstanceId == CurrentAttackInstanceId;
}

bool UMVCombatComponent::IsAttackFromCurrentPassiveActivation(
	const FMVResolvedHitData& HitData) const
{
	if (ActivePassiveActivationId == 0
		|| !IsValid(ActivePassiveAbilityInstance.Get())
		|| !ActivePassiveAbilityInstance->bAbilityActive
		|| !IsValid(StatComponent.Get()) || StatComponent->IsDead()
		|| HitData.Origin != EMVResolvedHitOrigin::AttackCollision)
	{
		return false;
	}

	const FMVProjectileExecutionRecord* ProjectileRecord =
		FindProjectileExecution(HitData.AttackInstanceId);
	if (ProjectileRecord)
	{
		return ProjectileRecord->SourceLifeId == CurrentLifeId
			&& ProjectileRecord->PassiveActivationId == ActivePassiveActivationId
			&& IsValid(ProjectileRecord->ExecutionAbility.Get())
			&& HitData.Attacker.Get() == GetOwner()
			&& IsProjectileSourceWeaponEquipped(*ProjectileRecord, HitData);
	}

	return CurrentAttackPassiveActivationId == ActivePassiveActivationId
		&& IsCurrentAttackHit(HitData);
}

bool UMVCombatComponent::IsAttackFromCurrentPassiveActivation(
	const UMVAbilityBase* Ability) const
{
	if (!IsValid(Ability)
		|| ActivePassiveActivationId == 0
		|| !IsValid(ActivePassiveAbilityInstance.Get())
		|| !ActivePassiveAbilityInstance->bAbilityActive
		|| !IsValid(StatComponent.Get()) || StatComponent->IsDead()
		|| Ability->GetAttackInstanceId() == INDEX_NONE)
	{
		return false;
	}

	const FMVProjectileExecutionRecord* ProjectileRecord =
		FindProjectileExecution(Ability->GetAttackInstanceId());
	if (ProjectileRecord)
	{
		const AMVCharacterBase* OwnerCharacter = Cast<AMVCharacterBase>(GetOwner());
		const UMVWeaponComponent* Weapon = IsValid(OwnerCharacter)
			? OwnerCharacter->WeaponComponent.Get()
			: nullptr;
		return ProjectileRecord->SourceLifeId == CurrentLifeId
			&& ProjectileRecord->ExecutionAbility.Get() == Ability
			&& ProjectileRecord->PassiveActivationId == ActivePassiveActivationId
			&& IsValid(Weapon)
			&& Weapon->GetEquippedWeaponState().ItemTag == ProjectileRecord->SourceWeaponItemTag;
	}

	return CurrentAbilityInstance.Get() == Ability
		&& bCurrentAbilityAwaitingCompletion
		&& CurrentAttackLifeId == CurrentLifeId
		&& CurrentAttackInstanceId == Ability->GetAttackInstanceId()
		&& CurrentAttackPassiveActivationId == ActivePassiveActivationId;
}

bool UMVCombatComponent::IsBasicAttackAbility(const UMVAbilityBase* Ability) const
{
	if (!IsValid(Ability))
	{
		return false;
	}

	const FMVProjectileExecutionRecord* ProjectileRecord =
		FindProjectileExecution(Ability->GetAttackInstanceId());
	if (ProjectileRecord &&
		ProjectileRecord->ExecutionAbility.Get() == Ability)
	{
		return ProjectileRecord->bWasBasicAttack;
	}

	for (const TPair<EMVCombatActionTypes, FMVSkillEntry>& Pair : BasicAttackMap)
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

bool UMVCombatComponent::CanApplyOutgoingAttackModifiers(const FMVResolvedHitData& HitData) const
{
	if (HitData.Origin != EMVResolvedHitOrigin::AttackCollision
		|| !IsValid(StatComponent.Get()) || StatComponent->IsDead())
	{
		return false;
	}

	const FMVProjectileExecutionRecord* ProjectileRecord =
		FindProjectileExecution(HitData.AttackInstanceId);

	if (ProjectileRecord)
	{
		if (ProjectileRecord->SourceLifeId != CurrentLifeId ||
			!IsValid(ProjectileRecord->ExecutionAbility.Get()) ||
			HitData.Attacker.Get() != GetOwner() ||
			!IsProjectileSourceWeaponEquipped(*ProjectileRecord, HitData))
		{
			return false;
		}
	}
	else if (!IsCurrentAttackHit(HitData))
	{
		return false;
	}
	return true;
}

void UMVCombatComponent::ApplyOutgoingCriticalChanceModifiers(
	const FMVResolvedHitData& HitData,
	float& InOutChance)
{
	if (!CanApplyOutgoingAttackModifiers(HitData) || !OnModifyOutgoingCriticalChance.IsBound())
	{
		return;
	}

	float ModifiedChance = InOutChance;
	OnModifyOutgoingCriticalChance.Broadcast(HitData, ModifiedChance);
	if (FMath::IsFinite(ModifiedChance))
	{
		InOutChance = FMath::Clamp(ModifiedChance, 0.0f, 1.0f);
	}
}

void UMVCombatComponent::ApplyOutgoingAttackDamageModifiers(FMVResolvedHitData& HitData)
{
	if (!CanApplyOutgoingAttackModifiers(HitData))
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
	CurrentAbilityRowHandle = FDataTableRowHandle();
	CurrentPreparedSkillSlot = EMVSkillSlot::NONE;
	CurrentPreparedChainStageIndex = INDEX_NONE;
	CurrentPreparedSlotRevision = 0;
	CurrentPreparedSkillUseId = 0;
	CurrentAttackLifeId = 0;
	CurrentAttackPassiveActivationId = 0;
	bCurrentAbilityAwaitingCompletion = false;
}

void UMVCombatComponent::RefreshActionMaps()
{
	ResetBasicAttackMap();
	ResetSkillMap();
}

void UMVCombatComponent::ResetBasicAttackMap()
{
	CancelChargePreparation();
	BasicAttackMap.Reset();
	NextLightChargeStageIndex = 0;
	LastBasicAttackedTime = 0.0;
	bSprintContextualBasicAttackConsumed = false;
	bWasSprintAttackContextActive = false;
	ConsumedDodgeContextActionInstanceId = INDEX_NONE;
	PendingDodgeContextActionInstanceId = INDEX_NONE;
	ClearLastBasicAttackSwingDirection();

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
			|| !WeaponComponent->GetEquippedWeaponPassiveRow(SkillRowHandle))
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

	SkillEntry.LoadoutRevision = ++NextSkillSlotRevision;
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
	if (!SpeedRow || !SpeedRow->bEnabled)
	{
		return false;
	}
	const bool bUseAttackSpeed = SpeedRow->bUseAttackSpeed;

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
		CurrentAbilityRowHandle = RowHandle;
		CurrentPreparedSkillSlot = EMVSkillSlot::NONE;
		CurrentPreparedChainStageIndex = INDEX_NONE;
		CurrentPreparedSlotRevision = 0;
		CurrentPreparedSkillUseId = 0;
		if (CurrentAbilityInstance)
		{
			int32 SourceSkillIndex = INDEX_NONE;
			for (int32 SkillIndex = 0; SkillIndex < MVCombatSkillSlots::Count; ++SkillIndex)
			{
				if (SkillMap.Find(MVCombatLegacySkillIndexToSlot(SkillIndex)) == &ActionEntry)
				{
					SourceSkillIndex = SkillIndex;
					CurrentPreparedSkillSlot = MVCombatLegacySkillIndexToSlot(SkillIndex);
					CurrentPreparedChainStageIndex = ActionEntry.CurrentChainStageIndex;
					CurrentPreparedSlotRevision = ActionEntry.LoadoutRevision;
					CurrentPreparedSkillUseId = ActionEntry.CurrentUseId;
					break;
				}
			}

			const FGameplayTag SourceWeaponItemTag =
				IsValid(Owner->WeaponComponent.Get())
					? Owner->WeaponComponent->GetEquippedWeaponState().ItemTag
					: FGameplayTag();

			CurrentAttackInstanceId = ++NextAttackInstanceId;
			CurrentAttackLifeId = CurrentLifeId;
			CurrentAttackPassiveActivationId = ActivePassiveActivationId;
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
				CurrentAbilityRowHandle = FDataTableRowHandle();
				CurrentPreparedSkillSlot = EMVSkillSlot::NONE;
				CurrentPreparedChainStageIndex = INDEX_NONE;
				CurrentPreparedSlotRevision = 0;
				CurrentPreparedSkillUseId = 0;
				CurrentAttackInstanceId = INDEX_NONE;
				CurrentAttackLifeId = 0;
				CurrentAttackPassiveActivationId = 0;
				bCurrentAbilityAwaitingCompletion = false;
			}
		};

	// A same-row replacement must finish the old ability before preparing the new one.
	if (bIsActionRunning)
	{
		if (bForceTransition || (bRecoveryOpen && bCanInterrupt))
		{
			if (ActionComponent->GetActiveActionTableName() == MVCombatActionTableNameFromDataTable(RowHandle.DataTable)
				&& ActionComponent->GetActiveActionRowName() == RowHandle.RowName)
			{
				ActionComponent->CancelActiveAction(TransitionBlendOutTime);
			}
			const UMVAbilityBase* PreparedAbility = PrepareCurrentAbility();
			const float PlayRateMultiplier = GetPlayRateMultiplier();
			const bool bStarted = ActionComponent->TryTransitionActionFromRowHandleWithPlayRate(
				RowHandle,
				StartSection,
				FMath::Max(0.0f, TransitionBlendOutTime),
				EMVActionDomain::Attack,
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
		const bool bStarted = ActionComponent->TryStartActionFromRowHandleWithPlayRate(
			RowHandle,
			StartSection,
			0.1f,
			EMVActionDomain::Attack,
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
	CancelChargePreparation();
	if (++CurrentLifeId == 0)
	{
		++CurrentLifeId;
	}
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
	return MVCombatUsesSharedLightChargeChain(ActionType)
		|| ActionType == EMVCombatActionTypes::SprintLightAttack
		|| ActionType == EMVCombatActionTypes::DodgeLightAttack;
}

void UMVCombatComponent::ResetExpiredBasicAttackChain(const double CurrentTime)
{
	if (CurrentTime - LastBasicAttackedTime > ResetBasicAttackTime)
	{
		ResetOtherBasicAttackChains(EMVCombatActionTypes::None);
		NextLightChargeStageIndex = 0;
		ClearLastBasicAttackSwingDirection();
	}
}

int32 UMVCombatComponent::SelectBasicAttackStage(
	const EMVCombatActionTypes ActionType, const FMVSkillEntry& Entry) const
{
	if (MVCombatUsesSharedLightChargeChain(ActionType))
	{
		return Entry.SkillRowNames.IsValidIndex(NextLightChargeStageIndex) ? NextLightChargeStageIndex : 0;
	}

	return SelectBasicAttackChainStageForSwing(Entry);
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

	if (MVCombatUsesSharedLightChargeChain(ActionType))
	{
		const FMVSkillEntry* Light = BasicAttackMap.Find(EMVCombatActionTypes::LightAttack);
		const FMVSkillEntry* Charge = BasicAttackMap.Find(EMVCombatActionTypes::ChargeAttack);
		const int32 StageCount = FMath::Max(Light ? Light->SkillRowNames.Num() : 0, Charge ? Charge->SkillRowNames.Num() : 0);
		NextLightChargeStageIndex = (NextLightChargeStageIndex + 1) % FMath::Max(1, StageCount);
	}
	else
	{
		NextLightChargeStageIndex = 0;
	}
}

bool UMVCombatComponent::TryBeginLightOrChargeAttack()
{
	if (FindPreparingChargeAction())
	{
		return true;
	}

	UpdateContextualBasicAttackResets();
	const EMVCombatActionTypes LightType = EMVCombatActionTypes::LightAttack;
	if (ResolveContextualBasicAttackActionType(LightType) != LightType)
	{
		return TryBasicCombatAction(LightType);
	}

	return TryBeginChargePreparation() || TryBasicCombatAction(LightType);
}

bool UMVCombatComponent::TryBeginChargePreparation()
{
	if (!CanStartCombatAction())
	{
		return false;
	}

	const EMVCombatActionTypes ChargeType = EMVCombatActionTypes::ChargeAttack;
	FMVSkillEntry* Charge = BasicAttackMap.Find(ChargeType);
	const AMVCharacterBase* Owner = Cast<AMVCharacterBase>(GetOwner());
	UAnimInstance* AnimInstance = Owner && Owner->GetMesh() ? Owner->GetMesh()->GetAnimInstance() : nullptr;
	if (!Charge || !AnimInstance)
	{
		return false;
	}

	const double CurrentTime = GetWorld()->GetTimeSeconds();
	ResetExpiredBasicAttackChain(CurrentTime);
	const int32 StageIndex = SelectBasicAttackStage(ChargeType, *Charge);
	const FMVSkillDataTableColumn* Row = GetBasicAttackSkillDataAtStage(*Charge, StageIndex);
	if (!CanConsumeActionCost(Row) || !Row->bEnabled || !StatComponent->HasStamina(Row->StaminaCost))
	{
		return false;
	}

	UAnimMontage* Montage = Cast<UAnimMontage>(Row->Montage.TryLoad());
	const int32 LeadInIndex = Montage ? Montage->GetSectionIndex(ChargePreparationSection) : INDEX_NONE;
	if (LeadInIndex == INDEX_NONE)
	{
		return false;
	}

	const FName AttackSection = Montage->GetAnimCompositeSection(LeadInIndex).NextSectionName;
	const float LeadInLength = Montage->GetSectionLength(LeadInIndex);
	if (AttackSection == ChargePreparationSection || !Montage->IsValidSectionName(AttackSection)
		|| LeadInLength <= 0.0f || Montage->RateScale <= 0.0f)
	{
		return false;
	}

	const int32 OriginalStageIndex = Charge->CurrentChainStageIndex;
	Charge->CurrentChainStageIndex = StageIndex;
	const bool bStarted = TryStartActionWithAbility(*Charge, Charge->GetCurrentActionRowHandle(), ChargePreparationSection);
	Charge->CurrentChainStageIndex = OriginalStageIndex;
	if (!bStarted)
	{
		return false;
	}

	bCurrentAbilityAwaitingCompletion = false;
	ChargePreparation.ActionInstanceId = Owner->ActionComponent->GetActiveActionInstanceId();
	ChargePreparation.ChargeStageIndex = StageIndex;
	ChargePreparation.StartedWorldTime = CurrentTime;
	ChargePreparation.CommitTime = Row->ChargeCommitTime > 0.0f ? Row->ChargeCommitTime : DefaultChargeCommitTime;
	ChargePreparation.AttackPlayRate = AnimInstance->Montage_GetPlayRate(Montage);
	ChargePreparation.AttackSection = AttackSection;

	// Keep the montage in anticipation until the charge input commits.
	AnimInstance->Montage_SetNextSection(ChargePreparationSection, ChargePreparationSection, Montage);
	if (ChargePreparation.CommitTime > 0.0f)
	{
		const float LeadInPlayRate = LeadInLength / (ChargePreparation.CommitTime * Montage->RateScale);
		AnimInstance->Montage_SetPlayRate(Montage, LeadInPlayRate);
	}
	TryCommitChargeAttack(0.0f);
	return true;
}

bool UMVCombatComponent::TryCommitChargeAttack(const float HeldSeconds)
{
	UMVActionComponent* Action = FindPreparingChargeAction();
	if (!Action)
	{
		return false;
	}

	const double CurrentTime = GetWorld()->GetTimeSeconds();
	if (FMath::Max(static_cast<double>(HeldSeconds), CurrentTime - ChargePreparation.StartedWorldTime)
		< ChargePreparation.CommitTime)
	{
		return true;
	}

	const EMVCombatActionTypes ChargeType = EMVCombatActionTypes::ChargeAttack;
	FMVSkillEntry* Charge = BasicAttackMap.Find(ChargeType);
	const int32 StageIndex = ChargePreparation.ChargeStageIndex;
	const FMVSkillDataTableColumn* Row = Charge ? GetBasicAttackSkillDataAtStage(*Charge, StageIndex) : nullptr;
	if (!CanConsumeActionCost(Row) || !Row->bEnabled || !StatComponent->HasStamina(Row->StaminaCost))
	{
		TransitionChargePreparationToLight();
		return true;
	}

	const AMVCharacterBase* Owner = CastChecked<AMVCharacterBase>(GetOwner());
	UAnimInstance* AnimInstance = Owner->GetMesh()->GetAnimInstance();
	if (!AnimInstance || !Action->TryJumpActiveActionSection(ChargePreparation.AttackSection))
	{
		CancelChargePreparation();
		return true;
	}

	UAnimMontage* Montage = Action->GetActiveActionMontage();
	AnimInstance->Montage_SetNextSection(ChargePreparationSection, ChargePreparation.AttackSection, Montage);
	AnimInstance->Montage_SetPlayRate(Montage, ChargePreparation.AttackPlayRate);
	ChargePreparation = {};
	bCurrentAbilityAwaitingCompletion = CurrentAbilityInstance != nullptr;
	Charge->CurrentChainStageIndex = StageIndex;
	UpdateLastBasicAttackSwingDirection(Charge->GetCurrentActionRowHandle());
	MarkBasicAttackChainStarted(*Charge, ChargeType, StageIndex, CurrentTime);
	UE_LOG(LogMVCombatComponent, Log, TEXT("Charge committed: Row=%s ChainStage=%d"),
		*Charge->SkillRowNames[StageIndex].ToString(), StageIndex);
	BroadcastCombatActionStarted(ChargeType, EMVSkillSlot::NONE);
	return true;
}

bool UMVCombatComponent::TryReleaseChargePreparation(const float HeldSeconds, const bool bCanceled)
{
	if (!FindPreparingChargeAction())
	{
		return false;
	}

	if (bCanceled)
	{
		CancelChargePreparation();
	}
	else
	{
		TryCommitChargeAttack(HeldSeconds);
		if (FindPreparingChargeAction())
		{
			TransitionChargePreparationToLight();
		}
	}
	return true;
}

void UMVCombatComponent::TransitionChargePreparationToLight()
{
	const FMVSkillEntry* Charge = BasicAttackMap.Find(EMVCombatActionTypes::ChargeAttack);
	const FMVSkillDataTableColumn* ChargeRow = Charge
		? GetBasicAttackSkillDataAtStage(*Charge, ChargePreparation.ChargeStageIndex) : nullptr;
	const EMVCombatActionTypes LightType = EMVCombatActionTypes::LightAttack;
	const FMVSkillEntry* Light = BasicAttackMap.Find(LightType);
	const FMVSkillDataTableColumn* LightRow = Light
		? GetBasicAttackSkillDataAtStage(*Light, SelectBasicAttackStage(LightType, *Light)) : nullptr;
	const UAnimMontage* LightMontage = LightRow ? Cast<UAnimMontage>(LightRow->Montage.TryLoad()) : nullptr;
	const FName RequestedSection = ChargeRow ? ChargeRow->EarlyReleaseStartSection : NAME_None;
	const FName StartSection = LightMontage && LightMontage->IsValidSectionName(RequestedSection) ? RequestedSection : NAME_None;
	const float BlendOutTime = ChargeRow ? ChargeRow->EarlyReleaseBlendOutTime : 0.1f;
	if (TryBasicAttack(LightType, StartSection, true, BlendOutTime))
	{
		ChargePreparation = {};
		BroadcastCombatActionStarted(LightType, EMVSkillSlot::NONE);
	}
	else
	{
		CancelChargePreparation();
	}
}

UMVActionComponent* UMVCombatComponent::FindPreparingChargeAction() const
{
	const AMVCharacterBase* Owner = Cast<AMVCharacterBase>(GetOwner());
	UMVActionComponent* Action = Owner ? Owner->ActionComponent.Get() : nullptr;
	return Action && Action->IsActionRunning()
		&& Action->GetActiveActionInstanceId() == ChargePreparation.ActionInstanceId ? Action : nullptr;
}

void UMVCombatComponent::CancelChargePreparation()
{
	UMVActionComponent* Action = FindPreparingChargeAction();
	ChargePreparation = {};
	if (Action)
	{
		Action->CancelActiveAction();
	}
}

EMVCombatActionTypes UMVCombatComponent::ResolveContextualBasicAttackActionType(
	const EMVCombatActionTypes RequestedActionType)
{
	PendingDodgeContextActionInstanceId = INDEX_NONE;

	if (RequestedActionType != EMVCombatActionTypes::LightAttack)
	{
		return RequestedActionType;
	}

	const int32 DodgeContextActionInstanceId = GetDodgeAttackContextInstanceId();
	const bool bSprintAttackContext = IsSprintAttackContext();
	if (DodgeContextActionInstanceId != INDEX_NONE
		&& DodgeContextActionInstanceId != ConsumedDodgeContextActionInstanceId)
	{
		PendingDodgeContextActionInstanceId = DodgeContextActionInstanceId;
		return EMVCombatActionTypes::DodgeLightAttack;
	}

	if (bSprintAttackContext && !bSprintContextualBasicAttackConsumed)
	{
		return EMVCombatActionTypes::SprintLightAttack;
	}

	return RequestedActionType;
}

void UMVCombatComponent::MarkContextualBasicAttackStarted(const EMVCombatActionTypes StartedActionType)
{
	if (StartedActionType == EMVCombatActionTypes::SprintLightAttack)
	{
		bSprintContextualBasicAttackConsumed = true;
		return;
	}

	if (StartedActionType == EMVCombatActionTypes::DodgeLightAttack
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

bool UMVCombatComponent::HasReachedSprintAttackSpeed() const
{
	if (!StatComponent)
	{
		return false;
	}

	return StatComponent->HasReachedSprintSpeedRatio(SprintAttackMinSpeedRatio);
}
