#include "Components/MVActionComponent.h"

#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Chooser.h"
#include "Components/MVStatComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Tables/MVTableManager.h"

UMVActionComponent::UMVActionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UMVActionComponent::BeginPlay()
{
	Super::BeginPlay();

	CacheOwnerReferences();
}

void UMVActionComponent::SetCharacterIndexId(const int32 NewCharacterIndexId)
{
	CharacterIndexId = FMath::Max(0, NewCharacterIndexId);
}

int32 UMVActionComponent::GetCharacterIndexId() const
{
	return CharacterIndexId;
}

int32 UMVActionComponent::GetActionProfileId() const
{
	return ResolveActionProfileId();
}

bool UMVActionComponent::TryStartAction(
	const EMVActionId ActionId,
	const float PlayRate,
	const FName StartSection)
{
	return TryStartActionById(MVActionIds::ToRawActionId(ActionId), PlayRate, StartSection);
}

bool UMVActionComponent::TryStartActionById(
	const int32 ActionId,
	float PlayRate,
	FName StartSection)
{
	if (IsActionRunning())
	{
		return TryBufferActionById(ActionId);
	}

	if (PlayRate <= 0.0f)
	{
		return false;
	}

	const FMVActionIndexRow* ActionIndex = FindActionIndexRow(ActionId);
	if (!ActionIndex)
	{
		return false;
	}

	const FMVActionStatRow* ActionStat = FindActionStatRow(ActionId);
	if (!ActionStat || !CanConsumeActionStartCost(*ActionStat))
	{
		return false;
	}

	OnActionPreparing.Broadcast(ActionId);

	UAnimMontage* ActionMontage = ResolveActionMontage(ActionId, *ActionIndex);
	if (!ActionMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("ActionId %d has no resolved montage."), ActionId);
		return false;
	}

	UAnimInstance* AnimInstance = GetOwnerAnimInstance();
	if (!AnimInstance)
	{
		return false;
	}

	const float MontageDuration = AnimInstance->Montage_Play(ActionMontage, PlayRate);
	if (MontageDuration <= 0.0f)
	{
		return false;
	}

	if (!StartSection.IsNone())
	{
		AnimInstance->Montage_JumpToSection(StartSection, ActionMontage);
	}

	ActiveActionId = ActionId;
	ActiveActionMontage = ActionMontage;

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &UMVActionComponent::HandleActionMontageEnded);
	AnimInstance->Montage_SetEndDelegate(EndDelegate, ActionMontage);

	BeginActionStatRecoveryPause();
	if (!ConsumeActionStartCost(*ActionStat, ActionId))
	{
		FinishActiveAction(true);
		AnimInstance->Montage_Stop(0.0f, ActionMontage);
		return false;
	}

	OnActionStarted.Broadcast(ActionId);
	return true;
}

void UMVActionComponent::FinishActiveAction(bool bInterrupted)
{
	if (!IsActionRunning())
	{
		return;
	}

	const int32 FinishedActionId = ActiveActionId;
	const int32 PendingBufferedActionId = !bInterrupted && HasBufferedAction()
		? BufferedActionId
		: INDEX_NONE;

	ActiveActionId = INDEX_NONE;
	ActiveActionMontage = nullptr;
	ResetActionNotifyState();
	ClearBufferedAction();

	EndActionStatRecoveryPause();
	OnActionEnded.Broadcast(FinishedActionId, bInterrupted);

	if (PendingBufferedActionId != INDEX_NONE)
	{
		TryStartActionById(PendingBufferedActionId);
	}
}

void UMVActionComponent::BeginActionStatRecoveryPause()
{
	const bool bWasPaused = ActionStatRecoveryPauseCount > 0;
	++ActionStatRecoveryPauseCount;
	if (!bWasPaused)
	{
		OnActionStatRecoveryPauseChanged.Broadcast(true);
	}
}

void UMVActionComponent::EndActionStatRecoveryPause()
{
	if (ActionStatRecoveryPauseCount <= 0)
	{
		ActionStatRecoveryPauseCount = 0;
		return;
	}

	--ActionStatRecoveryPauseCount;
	if (ActionStatRecoveryPauseCount <= 0)
	{
		ActionStatRecoveryPauseCount = 0;
		OnActionStatRecoveryPauseChanged.Broadcast(false);
	}
}

void UMVActionComponent::ResetActionNotifyState()
{
	InputBufferWindowCount = 0;
}

bool UMVActionComponent::IsActionStatRecoveryPaused() const
{
	return ActionStatRecoveryPauseCount > 0;
}

bool UMVActionComponent::IsActionRunning() const
{
	return ActiveActionId != INDEX_NONE && ActiveActionMontage;
}

int32 UMVActionComponent::GetActiveActionId() const
{
	return ActiveActionId;
}

UAnimMontage* UMVActionComponent::GetActiveActionMontage() const
{
	return ActiveActionMontage;
}

bool UMVActionComponent::TryBufferAction(const EMVActionId ActionId)
{
	return TryBufferActionById(MVActionIds::ToRawActionId(ActionId));
}

bool UMVActionComponent::TryBufferActionById(const int32 ActionId)
{
	if (!IsActionRunning() || !IsInputBufferOpen() || ActionId <= 0)
	{
		return false;
	}

	if (!FindActionIndexRow(ActionId))
	{
		return false;
	}

	BufferedActionId = ActionId;
	return true;
}

void UMVActionComponent::ClearBufferedAction()
{
	BufferedActionId = INDEX_NONE;
}

bool UMVActionComponent::HasBufferedAction() const
{
	return BufferedActionId != INDEX_NONE;
}

int32 UMVActionComponent::GetBufferedActionId() const
{
	return BufferedActionId;
}

bool UMVActionComponent::IsInputBufferOpen() const
{
	return InputBufferWindowCount > 0;
}

void UMVActionComponent::BeginInputBufferWindow()
{
	++InputBufferWindowCount;
}

void UMVActionComponent::EndInputBufferWindow()
{
	InputBufferWindowCount = FMath::Max(0, InputBufferWindowCount - 1);
}

const FMVActionIndexRow* UMVActionComponent::FindActionIndexRow(
	const int32 ActionId,
	const EMVActionType ExpectedActionType) const
{
	const int32 ResolvedActionProfileId = ResolveActionProfileId();
	if (ResolvedActionProfileId <= 0)
	{
		return nullptr;
	}

	const FString ActionIndexKey = MakeActionIndexRowKey(ResolvedActionProfileId, ActionId);
	const UMVTableManager* TableManager = UMVTableManager::Get(this);
	const FMVActionIndexRow* ActionIndex = TableManager && !ActionIndexTableName.IsNone() && ActionId > 0
		? TableManager->FindRow<FMVActionIndexRow>(ActionIndexTableName, ActionIndexKey)
		: nullptr;
	if (!ActionIndex || !ActionIndex->bEnabled)
	{
		return nullptr;
	}

	if (ActionIndex->ActionProfileId != ResolvedActionProfileId || ActionIndex->ActionId != ActionId)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("ActionIndex row '%s' mismatches requested ActionProfileId %d and ActionId %d."),
			*ActionIndexKey,
			ResolvedActionProfileId,
			ActionId);
		return nullptr;
	}

	if (ExpectedActionType != EMVActionType::None && ActionIndex->ActionType != ExpectedActionType)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("ActionIndex row '%s' has unexpected ActionType."),
			*ActionIndexKey);
		return nullptr;
	}

	return ActionIndex;
}

const FMVActionStatRow* UMVActionComponent::FindActionStatRow(
	int32 ActionId,
	EMVActionType ExpectedActionType) const
{
	const FMVActionIndexRow* ActionIndex = FindActionIndexRow(ActionId, ExpectedActionType);
	if (!ActionIndex)
	{
		return nullptr;
	}

	const int32 ActionStatId = ActionIndex->ActionStatId > 0
		? ActionIndex->ActionStatId
		: ActionIndex->ActionId;
	const UMVTableManager* TableManager = UMVTableManager::Get(this);
	return TableManager && !ActionStatTableName.IsNone() && ActionStatId > 0
		? TableManager->FindRow<FMVActionStatRow>(ActionStatTableName, FString::FromInt(ActionStatId))
		: nullptr;
}

void UMVActionComponent::CacheOwnerReferences()
{
	CachedStatComponent = GetOwner()
		? GetOwner()->FindComponentByClass<UMVStatComponent>()
		: nullptr;
}

const FMVCharacterIndexRow* UMVActionComponent::FindCharacterIndexRow() const
{
	const UMVTableManager* TableManager = UMVTableManager::Get(this);
	const FMVCharacterIndexRow* CharacterIndex = TableManager && !CharacterIndexTableName.IsNone() && CharacterIndexId > 0
		? TableManager->FindRow<FMVCharacterIndexRow>(CharacterIndexTableName, FString::FromInt(CharacterIndexId))
		: nullptr;
	if (!CharacterIndex || !CharacterIndex->bEnabled)
	{
		return nullptr;
	}

	if (CharacterIndex->CharacterIndexId != CharacterIndexId)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("CharacterIndex row '%d' mismatches requested CharacterIndexId %d."),
			CharacterIndex->CharacterIndexId,
			CharacterIndexId);
		return nullptr;
	}

	return CharacterIndex;
}

int32 UMVActionComponent::ResolveActionProfileId() const
{
	const FMVCharacterIndexRow* CharacterIndex = FindCharacterIndexRow();
	if (!CharacterIndex)
	{
		UE_LOG(LogTemp, Warning, TEXT("CharacterIndexId %d has no enabled CharacterIndex row."), CharacterIndexId);
		return INDEX_NONE;
	}

	if (CharacterIndex->ActionProfileId <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("CharacterIndexId %d has invalid ActionProfileId."), CharacterIndexId);
		return INDEX_NONE;
	}

	return CharacterIndex->ActionProfileId;
}

FString UMVActionComponent::MakeActionIndexRowKey(const int32 InActionProfileId, const int32 InActionId)
{
	return FString::Printf(TEXT("%05d_%d"), InActionProfileId, InActionId);
}

UAnimInstance* UMVActionComponent::GetOwnerAnimInstance() const
{
	const ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	const USkeletalMeshComponent* MeshComponent = OwnerCharacter ? OwnerCharacter->GetMesh() : nullptr;
	return MeshComponent ? MeshComponent->GetAnimInstance() : nullptr;
}

UAnimMontage* UMVActionComponent::ResolveActionMontage(int32 ActionId, const FMVActionIndexRow& ActionIndex) const
{
	UChooserTable* ChooserTable = LoadActionChooserTable(ActionId, ActionIndex);
	if (!ChooserTable)
	{
		return nullptr;
	}

	FChooserEvaluationContext Context;
	if (UObject* OwnerObject = GetOwner())
	{
		Context.AddObjectParam(OwnerObject);
	}
	Context.AddObjectParam(const_cast<UMVActionComponent*>(this));

	TSoftObjectPtr<UObject> SelectedObject;
	UChooserTable::EvaluateChooser(
		Context,
		ChooserTable,
		FObjectChooserBase::FObjectChooserSoftObjectIteratorCallback::CreateLambda(
			[&SelectedObject](const TSoftObjectPtr<UObject>& InResult)
			{
				SelectedObject = InResult;
				return FObjectChooserBase::EIteratorStatus::Stop;
			}));

	UObject* ResolvedObject = SelectedObject.IsValid()
		? SelectedObject.Get()
		: SelectedObject.LoadSynchronous();
	UAnimMontage* Montage = Cast<UAnimMontage>(ResolvedObject);
	if (!Montage)
	{
		UE_LOG(LogTemp, Warning, TEXT("ActionId %d chooser did not resolve an AnimMontage."), ActionId);
	}

	return Montage;
}

UChooserTable* UMVActionComponent::LoadActionChooserTable(int32 ActionId, const FMVActionIndexRow& ActionIndex) const
{
	if (!ActionIndex.AnimationChooserTable.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("ActionId %d has no AnimationChooserTable."), ActionId);
		return nullptr;
	}

	UObject* ChooserObject = ActionIndex.AnimationChooserTable.TryLoad();
	UChooserTable* ChooserTable = Cast<UChooserTable>(ChooserObject);
	if (!ChooserTable)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("ActionId %d AnimationChooserTable path is not a ChooserTable: %s"),
			ActionId,
			*ActionIndex.AnimationChooserTable.ToString());
	}

	return ChooserTable;
}

void UMVActionComponent::HandleActionMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != ActiveActionMontage)
	{
		return;
	}

	FinishActiveAction(bInterrupted);
}

float UMVActionComponent::GetActionStartStaminaCost(const FMVActionStatRow& ActionStat) const
{
	return ActionStat.StaminaCostType == EMVActionResourceCostType::Instant
		? FMath::Max(0.0f, ActionStat.StaminaCost)
		: 0.0f;
}

float UMVActionComponent::GetActionStartMPCost(const FMVActionStatRow& ActionStat) const
{
	return ActionStat.MPCostType == EMVActionResourceCostType::Instant
		? FMath::Max(0.0f, ActionStat.MPCost)
		: 0.0f;
}

bool UMVActionComponent::CanConsumeActionStartCost(const FMVActionStatRow& ActionStat) const
{
	if (!CachedStatComponent)
	{
		const_cast<UMVActionComponent*>(this)->CacheOwnerReferences();
	}

	if (!CachedStatComponent)
	{
		return true;
	}

	const float RequiredStamina = FMath::Max(
		FMath::Max(0.0f, ActionStat.MinRequiredStamina),
		GetActionStartStaminaCost(ActionStat));
	const float RequiredMP = FMath::Max(
		FMath::Max(0.0f, ActionStat.MinRequiredMP),
		GetActionStartMPCost(ActionStat));

	return CachedStatComponent->HasStamina(RequiredStamina)
		&& CachedStatComponent->HasMP(RequiredMP);
}

bool UMVActionComponent::ConsumeActionStartCost(const FMVActionStatRow& ActionStat, int32 ActionId)
{
	if (!CachedStatComponent)
	{
		CacheOwnerReferences();
	}

	if (!CachedStatComponent)
	{
		return true;
	}

	const float StaminaCost = GetActionStartStaminaCost(ActionStat);
	const float MPCost = GetActionStartMPCost(ActionStat);
	if (!CanConsumeActionStartCost(ActionStat))
	{
		return false;
	}

	bool bConsumedAnyCost = false;
	if (StaminaCost > 0.0f)
	{
		if (!CachedStatComponent->ConsumeStamina(StaminaCost))
		{
			return false;
		}
		bConsumedAnyCost = true;
	}

	if (MPCost > 0.0f)
	{
		if (!CachedStatComponent->ConsumeMP(MPCost))
		{
			return false;
		}
		bConsumedAnyCost = true;
	}

	if (bConsumedAnyCost)
	{
		OnActionCostConsumed.Broadcast(ActionId);
	}

	return true;
}
