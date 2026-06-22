#include "Components/MVActionComponent.h"

#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Chooser.h"
#include "Components/MVStatComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Tables/MVTableManager.h"

namespace
{
const TCHAR* DebugBoolText(const bool bValue)
{
	return bValue ? TEXT("true") : TEXT("false");
}

FVector NormalizeBufferedMovementInput(const FVector& MovementInputDirection)
{
	const FVector MovementInput2D(MovementInputDirection.X, MovementInputDirection.Y, 0.0f);
	return MovementInput2D.IsNearlyZero()
		? FVector::ZeroVector
		: MovementInput2D.GetSafeNormal2D();
}

}

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
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[MVActionBuffer] RedirectToBuffer Owner=%s Requested=%d Active=%d Buffered=%d InputWindow=%d RecoveryWindow=%d MovementBlock=%d Consuming=%s"),
			*GetNameSafe(GetOwner()),
			ActionId,
			ActiveActionId,
			BufferedActionId,
			InputBufferWindowCount,
			RecoveryEscapeWindowCount,
			MovementInputBlockCount,
			DebugBoolText(bConsumingBufferedAction));
		return TryBufferActionById(ActionId);
	}

	if (PlayRate <= 0.0f)
	{
		return false;
	}

	if (!bConsumingBufferedAction && IsMovementInputBlocked())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[MVActionBuffer] StartRejected MovementBlocked Owner=%s Requested=%d MovementBlock=%d"),
			*GetNameSafe(GetOwner()),
			ActionId,
			MovementInputBlockCount);
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
	ActiveActionInstanceId = ++NextActionInstanceId;

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &UMVActionComponent::HandleActionMontageEnded, ActiveActionInstanceId);
	AnimInstance->Montage_SetEndDelegate(EndDelegate, ActionMontage);

	BeginActionStatRecoveryPause();
	if (!ConsumeActionStartCost(*ActionStat, ActionId))
	{
		FinishActiveAction(true);
		AnimInstance->Montage_Stop(0.0f, ActionMontage);
		return false;
	}

	OnActionStarted.Broadcast(ActionId);
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[MVActionBuffer] ActionStarted Owner=%s Action=%d Montage=%s"),
		*GetNameSafe(GetOwner()),
		ActionId,
		*GetNameSafe(ActionMontage));
	return true;
}

void UMVActionComponent::FinishActiveAction(bool bInterrupted)
{
	if (!IsActionRunning())
	{
		return;
	}

	const int32 FinishedActionId = ActiveActionId;
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[MVActionBuffer] FinishActiveAction Owner=%s Finished=%d Interrupted=%s Buffered=%d HasBufferedInput=%s BufferedDir=%s InputWindow=%d RecoveryWindow=%d MovementBlock=%d"),
		*GetNameSafe(GetOwner()),
		FinishedActionId,
		DebugBoolText(bInterrupted),
		BufferedActionId,
		DebugBoolText(bBufferedActionHasMovementInput),
		*BufferedActionMovementInputDirection.ToCompactString(),
		InputBufferWindowCount,
		RecoveryEscapeWindowCount,
		MovementInputBlockCount);

	ActiveActionId = INDEX_NONE;
	ActiveActionMontage = nullptr;
	ActiveActionInstanceId = INDEX_NONE;
	ResetActionNotifyState();
	ClearBufferedAction();

	EndActionStatRecoveryPause();
	OnActionEnded.Broadcast(FinishedActionId, bInterrupted);
}

void UMVActionComponent::CompleteActiveAction()
{
	FinishActiveAction(false);
}

void UMVActionComponent::CancelActiveAction(const float BlendOutTime)
{
	UAnimInstance* AnimInstance = GetOwnerAnimInstance();
	UAnimMontage* MontageToStop = ActiveActionMontage;

	FinishActiveAction(true);

	if (AnimInstance && MontageToStop)
	{
		AnimInstance->Montage_Stop(FMath::Max(0.0f, BlendOutTime), MontageToStop);
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
	MovementInputBlockCount = 0;
	RecoveryEscapeWindowCount = 0;
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
	if (!IsActionRunning() || ActionId <= 0)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[MVActionBuffer] BufferRejected InvalidState Owner=%s Requested=%d Active=%d IsRunning=%s"),
			*GetNameSafe(GetOwner()),
			ActionId,
			ActiveActionId,
			DebugBoolText(IsActionRunning()));
		return false;
	}

	if (!IsInputBufferOpen() && !IsRecoveryEscapeWindowOpen())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[MVActionBuffer] BufferRejected WindowClosed Owner=%s Requested=%d Active=%d InputWindow=%d RecoveryWindow=%d MovementBlock=%d"),
			*GetNameSafe(GetOwner()),
			ActionId,
			ActiveActionId,
			InputBufferWindowCount,
			RecoveryEscapeWindowCount,
			MovementInputBlockCount);
		return false;
	}

	if (!FindActionIndexRow(ActionId))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[MVActionBuffer] BufferRejected MissingActionRow Owner=%s Requested=%d Active=%d"),
			*GetNameSafe(GetOwner()),
			ActionId,
			ActiveActionId);
		return false;
	}

	const FVector MovementInputDirection = ResolveBufferedActionMovementInput.IsBound()
		? ResolveBufferedActionMovementInput.Execute(ActionId)
		: FVector::ZeroVector;
	BufferAction(ActionId, MovementInputDirection);
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[MVActionBuffer] Buffered Owner=%s Action=%d Active=%d HasInput=%s Dir=%s InputWindow=%d RecoveryWindow=%d MovementBlock=%d"),
		*GetNameSafe(GetOwner()),
		ActionId,
		ActiveActionId,
		DebugBoolText(bBufferedActionHasMovementInput),
		*BufferedActionMovementInputDirection.ToCompactString(),
		InputBufferWindowCount,
		RecoveryEscapeWindowCount,
		MovementInputBlockCount);
	if (IsRecoveryEscapeWindowOpen())
	{
		const bool bStartedBufferedAction = TryStartBufferedAction();
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[MVActionBuffer] BufferedImmediateConsumeAttempt Owner=%s Action=%d Started=%s"),
			*GetNameSafe(GetOwner()),
			ActionId,
			DebugBoolText(bStartedBufferedAction));
	}
	return true;
}

void UMVActionComponent::ClearBufferedAction()
{
	if (HasBufferedAction())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[MVActionBuffer] ClearBufferedAction Owner=%s Buffered=%d HasInput=%s Dir=%s"),
			*GetNameSafe(GetOwner()),
			BufferedActionId,
			DebugBoolText(bBufferedActionHasMovementInput),
			*BufferedActionMovementInputDirection.ToCompactString());
	}
	BufferedActionId = INDEX_NONE;
	BufferedActionMovementInputDirection = FVector::ZeroVector;
	bBufferedActionHasMovementInput = false;
}

void UMVActionComponent::UpdateBufferedActionMovementInput(const FVector& MovementInputDirection)
{
	if (!HasBufferedAction())
	{
		return;
	}

	const FVector NormalizedMovementInput = NormalizeBufferedMovementInput(MovementInputDirection);
	if (NormalizedMovementInput.IsNearlyZero())
	{
		return;
	}

	const bool bPreviousHasInput = bBufferedActionHasMovementInput;
	const FVector PreviousDirection = BufferedActionMovementInputDirection;
	BufferedActionMovementInputDirection = NormalizedMovementInput;
	bBufferedActionHasMovementInput = !NormalizedMovementInput.IsNearlyZero();
	if (bPreviousHasInput != bBufferedActionHasMovementInput
		|| !PreviousDirection.Equals(BufferedActionMovementInputDirection, KINDA_SMALL_NUMBER))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[MVActionBuffer] BufferedInputUpdated Owner=%s Buffered=%d HasInput=%s Dir=%s RawDir=%s"),
			*GetNameSafe(GetOwner()),
			BufferedActionId,
			DebugBoolText(bBufferedActionHasMovementInput),
			*BufferedActionMovementInputDirection.ToCompactString(),
			*MovementInputDirection.ToCompactString());
	}
}

bool UMVActionComponent::HasBufferedAction() const
{
	return BufferedActionId != INDEX_NONE;
}

int32 UMVActionComponent::GetBufferedActionId() const
{
	return BufferedActionId;
}

bool UMVActionComponent::HasBufferedActionMovementInput() const
{
	return bBufferedActionHasMovementInput;
}

FVector UMVActionComponent::GetBufferedActionMovementInputDirection() const
{
	return BufferedActionMovementInputDirection;
}

bool UMVActionComponent::IsConsumingBufferedAction() const
{
	return bConsumingBufferedAction;
}

int32 UMVActionComponent::GetConsumingBufferedActionId() const
{
	return ConsumingBufferedActionId;
}

bool UMVActionComponent::HasConsumingBufferedActionMovementInput() const
{
	return bConsumingBufferedActionHasMovementInput;
}

FVector UMVActionComponent::GetConsumingBufferedActionMovementInputDirection() const
{
	return ConsumingBufferedActionMovementInputDirection;
}

bool UMVActionComponent::IsInputBufferOpen() const
{
	return InputBufferWindowCount > 0;
}

bool UMVActionComponent::TryStartBufferedAction()
{
	if (bConsumingBufferedAction)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MVActionBuffer] ConsumeBlocked AlreadyConsuming Owner=%s"), *GetNameSafe(GetOwner()));
		return false;
	}

	if (!HasBufferedAction())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[MVActionBuffer] ConsumeBlocked NoBufferedAction Owner=%s Active=%d InputWindow=%d RecoveryWindow=%d MovementBlock=%d"),
			*GetNameSafe(GetOwner()),
			ActiveActionId,
			InputBufferWindowCount,
			RecoveryEscapeWindowCount,
			MovementInputBlockCount);
		return false;
	}

	if (!IsRecoveryEscapeWindowOpen())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[MVActionBuffer] ConsumeBlocked RecoveryWindowClosed Owner=%s Buffered=%d InputWindow=%d RecoveryWindow=%d MovementBlock=%d"),
			*GetNameSafe(GetOwner()),
			BufferedActionId,
			InputBufferWindowCount,
			RecoveryEscapeWindowCount,
			MovementInputBlockCount);
		return false;
	}

	if (IsMovementInputBlocked())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[MVActionBuffer] ConsumeBlocked MovementBlocked Owner=%s Buffered=%d InputWindow=%d RecoveryWindow=%d MovementBlock=%d"),
			*GetNameSafe(GetOwner()),
			BufferedActionId,
			InputBufferWindowCount,
			RecoveryEscapeWindowCount,
			MovementInputBlockCount);
		return false;
	}

	const int32 ActionIdToStart = BufferedActionId;
	const FVector MovementInputDirectionToStart = BufferedActionMovementInputDirection;
	const bool bHasMovementInputToStart = bBufferedActionHasMovementInput;
	if (CanConsumeBufferedAction.IsBound()
		&& !CanConsumeBufferedAction.Execute(
			ActionIdToStart,
			MovementInputDirectionToStart,
			bHasMovementInputToStart))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[MVActionBuffer] ConsumeRejectedByDelegate Owner=%s Buffered=%d HasInput=%s Dir=%s Active=%d"),
			*GetNameSafe(GetOwner()),
			ActionIdToStart,
			DebugBoolText(bHasMovementInputToStart),
			*MovementInputDirectionToStart.ToCompactString(),
			ActiveActionId);
		ClearBufferedAction();
		return false;
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[MVActionBuffer] ConsumeStart Owner=%s Buffered=%d HasInput=%s Dir=%s Active=%d InputWindow=%d RecoveryWindow=%d MovementBlock=%d"),
		*GetNameSafe(GetOwner()),
		ActionIdToStart,
		DebugBoolText(bHasMovementInputToStart),
		*MovementInputDirectionToStart.ToCompactString(),
		ActiveActionId,
		InputBufferWindowCount,
		RecoveryEscapeWindowCount,
		MovementInputBlockCount);
	ClearBufferedAction();

	bConsumingBufferedAction = true;
	ConsumingBufferedActionId = ActionIdToStart;
	ConsumingBufferedActionMovementInputDirection = MovementInputDirectionToStart;
	bConsumingBufferedActionHasMovementInput = bHasMovementInputToStart;

	if (IsActionRunning())
	{
		CancelActiveAction(0.1f);
	}

	const bool bStarted = TryStartActionById(ActionIdToStart);
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[MVActionBuffer] ConsumeEnd Owner=%s Buffered=%d Started=%s ActiveNow=%d"),
		*GetNameSafe(GetOwner()),
		ActionIdToStart,
		DebugBoolText(bStarted),
		ActiveActionId);

	bConsumingBufferedAction = false;
	ConsumingBufferedActionId = INDEX_NONE;
	ConsumingBufferedActionMovementInputDirection = FVector::ZeroVector;
	bConsumingBufferedActionHasMovementInput = false;

	return bStarted;
}

void UMVActionComponent::BeginInputBufferWindow()
{
	++InputBufferWindowCount;
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[MVActionBuffer] InputWindowBegin Owner=%s Count=%d Active=%d Buffered=%d RecoveryWindow=%d MovementBlock=%d"),
		*GetNameSafe(GetOwner()),
		InputBufferWindowCount,
		ActiveActionId,
		BufferedActionId,
		RecoveryEscapeWindowCount,
		MovementInputBlockCount);
}

void UMVActionComponent::EndInputBufferWindow()
{
	InputBufferWindowCount = FMath::Max(0, InputBufferWindowCount - 1);
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[MVActionBuffer] InputWindowEnd Owner=%s Count=%d Active=%d Buffered=%d RecoveryWindow=%d MovementBlock=%d"),
		*GetNameSafe(GetOwner()),
		InputBufferWindowCount,
		ActiveActionId,
		BufferedActionId,
		RecoveryEscapeWindowCount,
		MovementInputBlockCount);
	if (!IsInputBufferOpen() && IsRecoveryEscapeWindowOpen())
	{
		const bool bStartedBufferedAction = TryStartBufferedAction();
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[MVActionBuffer] InputWindowEndConsumeAttempt Owner=%s Started=%s"),
			*GetNameSafe(GetOwner()),
			DebugBoolText(bStartedBufferedAction));
	}
}

void UMVActionComponent::BeginMovementInputBlock()
{
	++MovementInputBlockCount;
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[MVActionBuffer] MovementBlockBegin Owner=%s Count=%d Active=%d Buffered=%d InputWindow=%d RecoveryWindow=%d"),
		*GetNameSafe(GetOwner()),
		MovementInputBlockCount,
		ActiveActionId,
		BufferedActionId,
		InputBufferWindowCount,
		RecoveryEscapeWindowCount);
}

void UMVActionComponent::EndMovementInputBlock()
{
	const bool bWasBlocked = MovementInputBlockCount > 0;
	MovementInputBlockCount = FMath::Max(0, MovementInputBlockCount - 1);
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[MVActionBuffer] MovementBlockEnd Owner=%s Count=%d WasBlocked=%s Active=%d Buffered=%d InputWindow=%d RecoveryWindow=%d"),
		*GetNameSafe(GetOwner()),
		MovementInputBlockCount,
		DebugBoolText(bWasBlocked),
		ActiveActionId,
		BufferedActionId,
		InputBufferWindowCount,
		RecoveryEscapeWindowCount);
	if (bWasBlocked && !IsMovementInputBlocked() && IsRecoveryEscapeWindowOpen())
	{
		const bool bStartedBufferedAction = TryStartBufferedAction();
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[MVActionBuffer] MovementBlockEndConsumeAttempt Owner=%s Started=%s"),
			*GetNameSafe(GetOwner()),
			DebugBoolText(bStartedBufferedAction));
	}
}

bool UMVActionComponent::IsMovementInputBlocked() const
{
	return MovementInputBlockCount > 0;
}

void UMVActionComponent::BeginRecoveryEscapeWindow()
{
	++RecoveryEscapeWindowCount;
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[MVActionBuffer] RecoveryWindowBegin Owner=%s Count=%d Active=%d Buffered=%d HasInput=%s Dir=%s InputWindow=%d MovementBlock=%d"),
		*GetNameSafe(GetOwner()),
		RecoveryEscapeWindowCount,
		ActiveActionId,
		BufferedActionId,
		DebugBoolText(bBufferedActionHasMovementInput),
		*BufferedActionMovementInputDirection.ToCompactString(),
		InputBufferWindowCount,
		MovementInputBlockCount);
	const bool bStartedBufferedAction = TryStartBufferedAction();
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[MVActionBuffer] RecoveryWindowBeginConsumeAttempt Owner=%s Started=%s"),
		*GetNameSafe(GetOwner()),
		DebugBoolText(bStartedBufferedAction));
}

void UMVActionComponent::EndRecoveryEscapeWindow()
{
	if (RecoveryEscapeWindowCount <= 0)
	{
		RecoveryEscapeWindowCount = 0;
		UE_LOG(LogTemp, Warning, TEXT("[MVActionBuffer] RecoveryWindowEndIgnored Owner=%s CountAlreadyZero"), *GetNameSafe(GetOwner()));
		return;
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[MVActionBuffer] RecoveryWindowEnd Owner=%s CountBefore=%d Active=%d Buffered=%d HasInput=%s Dir=%s InputWindow=%d MovementBlock=%d"),
		*GetNameSafe(GetOwner()),
		RecoveryEscapeWindowCount,
		ActiveActionId,
		BufferedActionId,
		DebugBoolText(bBufferedActionHasMovementInput),
		*BufferedActionMovementInputDirection.ToCompactString(),
		InputBufferWindowCount,
		MovementInputBlockCount);
	const bool bStartedBufferedAction = TryStartBufferedAction();
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[MVActionBuffer] RecoveryWindowEndConsumeAttempt Owner=%s Started=%s"),
		*GetNameSafe(GetOwner()),
		DebugBoolText(bStartedBufferedAction));
	RecoveryEscapeWindowCount = FMath::Max(0, RecoveryEscapeWindowCount - 1);
}

bool UMVActionComponent::IsRecoveryEscapeWindowOpen() const
{
	return RecoveryEscapeWindowCount > 0;
}

void UMVActionComponent::BufferAction(const int32 ActionId, const FVector& MovementInputDirection)
{
	const FVector NormalizedMovementInput = NormalizeBufferedMovementInput(MovementInputDirection);
	BufferedActionId = ActionId;
	BufferedActionMovementInputDirection = NormalizedMovementInput;
	bBufferedActionHasMovementInput = !NormalizedMovementInput.IsNearlyZero();
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

void UMVActionComponent::HandleActionMontageEnded(
	UAnimMontage* Montage,
	bool bInterrupted,
	const int32 ActionInstanceId)
{
	if (ActionInstanceId != ActiveActionInstanceId || Montage != ActiveActionMontage)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[MVActionBuffer] IgnoreStaleMontageEnded Owner=%s Montage=%s Interrupted=%s EndInstance=%d ActiveInstance=%d Active=%d ActiveMontage=%s"),
			*GetNameSafe(GetOwner()),
			*GetNameSafe(Montage),
			DebugBoolText(bInterrupted),
			ActionInstanceId,
			ActiveActionInstanceId,
			ActiveActionId,
			*GetNameSafe(ActiveActionMontage));
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
