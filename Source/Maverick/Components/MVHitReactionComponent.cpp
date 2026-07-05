#include "Components/MVHitReactionComponent.h"

#include "Character/MVCharacterBase.h"
#include "Chooser.h"
#include "Components/MVActionComponent.h"
#include "Engine/DataTable.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Tables/MVCharacterTableTypes.h"
#include "Tables/MVTableManager.h"
#include "Tags/MVGameplayTags.h"

DEFINE_LOG_CATEGORY_STATIC(LogMVHitReactionComponent, Log, All);

namespace
{
const FName MVHitReactionGetupRecoveryType(TEXT("Getup"));
const FName MVHitReactionEscapeDodgeRecoveryType(TEXT("EscapeDodge"));

FString MVHitReactionBuildAvailableRowNameLog(const UDataTable& DataTable)
{
	TArray<FString> RowKeys;
	for (const FName RowName : DataTable.GetRowNames())
	{
		RowKeys.Add(RowName.ToString());
	}

	RowKeys.Sort();
	if (RowKeys.IsEmpty())
	{
		return TEXT("<empty>");
	}

	const int32 MaxLoggedRowCount = 16;
	TArray<FString> LoggedRowKeys;
	const int32 LoggedCount = FMath::Min(RowKeys.Num(), MaxLoggedRowCount);
	for (int32 Index = 0; Index < LoggedCount; ++Index)
	{
		LoggedRowKeys.Add(RowKeys[Index]);
	}

	FString Result = FString::Join(LoggedRowKeys, TEXT(", "));
	if (RowKeys.Num() > MaxLoggedRowCount)
	{
		Result += FString::Printf(TEXT(", ... (+%d more)"), RowKeys.Num() - MaxLoggedRowCount);
	}
	return Result;
}
}

UMVHitReactionComponent::UMVHitReactionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	GroggyTriggerHitReactionTypes =
	{
		EMVActionHitReactionType::LargeHit,
		EMVActionHitReactionType::KnockDown,
		EMVActionHitReactionType::Airborne
	};
}

void UMVHitReactionComponent::BeginPlay()
{
	Super::BeginPlay();

	CacheOwnerReferences();
	BindInputManagerHandlers();
	BindActionComponentHandlers();
}

void UMVHitReactionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (CachedInputManager)
	{
		CachedInputManager->UnregisterActionInputHandler(this);
	}

	if (CachedActionComponent)
	{
		CachedActionComponent->OnActionEnded.RemoveDynamic(
			this,
			&UMVHitReactionComponent::HandleActionEnded);
	}

	Super::EndPlay(EndPlayReason);
}

void UMVHitReactionComponent::HandleDamaged(const FMVResolvedHitData& HitData)
{
	if (HitData.HitReactionType == EMVActionHitReactionType::None)
	{
		return;
	}

	if (!OwnerCharacter || !CachedActionComponent || !CachedStatComponent)
	{
		CacheOwnerReferences();
	}

	if (!OwnerCharacter || !CachedActionComponent)
	{
		return;
	}

	if (CachedStatComponent && CachedStatComponent->IsDead())
	{
		return;
	}

	if (HitData.VictimCharacterIndexCode.IsValid() && HitData.VictimCharacterIndexCode != OwnerCharacter->GetCharacterIndexCode())
	{
		return;
	}

	if (OwnerCharacter->IsInvincible())
	{
		return;
	}

	const bool bLethalHit = CachedStatComponent && CachedStatComponent->WouldDieFromHit(HitData);

	if (bLethalHit && !MVActionHitReactions::IsKnockDownOrAirborne(HitData.HitReactionType))
	{
		return;
	}

	FMVHitReactionActionData ActionData;
	if (!GetActionData(HitData, ActionData))
	{
		UE_LOG(
			LogMVHitReactionComponent,
			Warning,
			TEXT("HitReaction action data was not resolved. CharacterIndexCode=%s, HitReactionType=%d."),
			*OwnerCharacter->GetCharacterIndexCode().ToString(),
			static_cast<int32>(HitData.HitReactionType));
		return;
	}

	if (CachedActionComponent->IsActionRunning())
	{
		const bool bActiveActionIsHitReaction = CachedActionComponent->GetActiveActionTableName()
			.ToString()
			.StartsWith(TEXT("HR_"));
		if (bActiveActionIsHitReaction && !CachedInputManager->IsRecoveryEscapeWindowOpen())
		{
			return;
		}

		if (!CachedActionComponent->CanInterruptActiveAction())
		{
			return;
		}

		if (bCancelActiveActionBeforeReaction)
		{
			CachedActionComponent->CancelActiveAction(CancelActiveActionBlendOutTime);
		}
	}

	const bool bStarted = CachedActionComponent->TryStartActionFromRowHandle(
		ActionData.ActionRowHandle,
		ActionData.StartSection);
	if (!bStarted)
	{
		UE_LOG(
			LogMVHitReactionComponent,
			Warning,
			TEXT("ActionComponent failed to start hit reaction. DataTable=%s, RowName=%s, Section=%s."),
			*GetNameSafe(ActionData.ActionRowHandle.DataTable),
			*ActionData.ActionRowHandle.RowName.ToString(),
			*ActionData.StartSection.ToString());
	}
	if (bStarted)
	{
		ResetAirborneLandDetector();
		ActiveHitReactionActionRowName = ActionData.ActionRowHandle.RowName;
		ActiveHitReactionType = HitData.HitReactionType;
		ActiveHitReactionDirection = ActionData.Direction;
		bActiveHitReactionActionIsRecoveryAction = false;
		ApplyHitReactionLaunch(HitData, ActionData.ActionRow);
		TryConsumeBufferedRecoveryInput();
	}
}

bool UMVHitReactionComponent::CanTriggerGroggy(const FMVResolvedHitData& HitData) const
{
	const AActor* Owner = GetOwner();
	const UMVStatComponent* StatComponent = CachedStatComponent.Get();
	if (!StatComponent && Owner)
	{
		StatComponent = Owner->FindComponentByClass<UMVStatComponent>();
	}

	if (!StatComponent
		|| StatComponent->IsDead()
		|| StatComponent->IsGroggy()
		|| StatComponent->MaxGroggy <= 0.0f)
	{
		return false;
	}

	if (HitData.Victim.Get() != Owner)
	{
		return false;
	}

	if (!CanTriggerGroggyByHitReactionType(HitData.HitReactionType))
	{
		return false;
	}

	if (!CanTriggerGroggyByActionTag(HitData.ActionTag))
	{
		return false;
	}

	const float PredictedGroggy = StatComponent->CurrentGroggy + FMath::Max(0.0f, HitData.GroggyDamage);
	return PredictedGroggy >= StatComponent->MaxGroggy;
}

EMVHitReactionDirection UMVHitReactionComponent::ResolveHitReactionDirection(const FMVResolvedHitData& HitData) const
{
	const AMVCharacterBase* Character = OwnerCharacter.Get();
	if (!Character)
	{
		Character = Cast<AMVCharacterBase>(GetOwner());
	}

	const FVector HitDirection2D(HitData.HitDirection.X, HitData.HitDirection.Y, 0.0f);
	if (!Character || HitDirection2D.IsNearlyZero())
	{
		return EMVHitReactionDirection::Front;
	}

	// HitResolver의 기본 HitDirection은 공격자에서 피격자로 향하는 힘 방향이므로, 리액션 선택은 공격 원점 방향으로 본다.
	const FVector IncomingDirection = -HitDirection2D.GetSafeNormal2D();
	const FVector Forward = Character->GetActorForwardVector().GetSafeNormal2D();
	const FVector Right = Character->GetActorRightVector().GetSafeNormal2D();

	const float ForwardDot = FVector::DotProduct(IncomingDirection, Forward);
	const float RightDot = FVector::DotProduct(IncomingDirection, Right);

	if (FMath::Abs(RightDot) > FMath::Abs(ForwardDot))
	{
		return RightDot >= 0.0f
			? EMVHitReactionDirection::Right
			: EMVHitReactionDirection::Left;
	}

	return ForwardDot >= 0.0f
		? EMVHitReactionDirection::Front
		: EMVHitReactionDirection::Back;
}

void UMVHitReactionComponent::CacheOwnerReferences()
{
	OwnerCharacter = Cast<AMVCharacterBase>(GetOwner());
	CachedActionComponent = GetOwner()
		? GetOwner()->FindComponentByClass<UMVActionComponent>()
		: nullptr;
	CachedInputManager = GetOwner()
		? GetOwner()->FindComponentByClass<UMVInputManagerComponent>()
		: nullptr;
	CachedStatComponent = GetOwner()
		? GetOwner()->FindComponentByClass<UMVStatComponent>()
		: nullptr;
}

void UMVHitReactionComponent::BindInputManagerHandlers()
{
	if (!CachedInputManager)
	{
		CacheOwnerReferences();
	}

	if (CachedInputManager)
	{
		CachedInputManager->RegisterActionInputHandler(this, MVActionInputHandlerPriorities::HitReaction);
	}
}

void UMVHitReactionComponent::BindActionComponentHandlers()
{
	if (!CachedActionComponent)
	{
		CacheOwnerReferences();
	}

	if (CachedActionComponent)
	{
		CachedActionComponent->OnActionEnded.RemoveDynamic(
			this,
			&UMVHitReactionComponent::HandleActionEnded);
		CachedActionComponent->OnActionEnded.AddUniqueDynamic(
			this,
			&UMVHitReactionComponent::HandleActionEnded);
	}
}

bool UMVHitReactionComponent::GetActionData(const FMVResolvedHitData& HitData, FMVHitReactionActionData& OutActionData)
{
	if (!OwnerCharacter)
	{
		CacheOwnerReferences();
	}

	if (!OwnerCharacter)
	{
		return false;
	}

	const EMVHitReactionDirection Direction = ResolveSupportedHitReactionDirection(
		HitData.HitReactionType,
		ResolveHitReactionDirection(HitData));
	ChooserCharacterIndexCode = ResolveCharacterIndexCode();
	ChooserCharacterIndexCodeTags.Reset();
	if (!ChooserCharacterIndexCode.IsValid())
	{
		return false;
	}
	ChooserCharacterIndexCodeTags.AddTag(ChooserCharacterIndexCode);

	ChooserEquippedStyle = OwnerCharacter->GetEquippedStyle();
	ChooserHitReactionType = HitData.HitReactionType;
	ChooserHitReactionDirection = Direction;

	FMVHitReactionActionRowHandle ActionRowHandle;
	if (!ResolveHitReactionActionRowHandle(HitData.HitReactionType, Direction, ActionRowHandle))
	{
		UE_LOG(
			LogMVHitReactionComponent,
			Warning,
			TEXT("HitReaction action row handle was not resolved. CharacterIndexCode=%s, HitReactionType=%d, Direction=%d."),
			*ChooserCharacterIndexCode.ToString(),
			static_cast<int32>(HitData.HitReactionType),
			static_cast<int32>(Direction));
		return false;
	}

	const FMVHitReactionActionRow* ActionRow = FindHitReactionActionRow(ActionRowHandle.ActionRow);
	if (!ActionRow)
	{
		UE_LOG(
			LogMVHitReactionComponent,
			Warning,
			TEXT("HitReaction row was not resolved. DataTable=%s, RowName=%s."),
			*GetNameSafe(ActionRowHandle.ActionRow.DataTable),
			*ActionRowHandle.ActionRow.RowName.ToString());
		return false;
	}

	OutActionData.ActionRowHandle = ActionRowHandle.ActionRow;
	OutActionData.StartSection = ActionRowHandle.StartSection.IsNone()
		? ActionRow->DefaultStartSection
		: ActionRowHandle.StartSection;
	OutActionData.Direction = Direction;
	OutActionData.ActionRow = *ActionRow;
	return true;
}

void UMVHitReactionComponent::ApplyHitReactionLaunch(
	const FMVResolvedHitData& HitData,
	const FMVHitReactionActionRow& ActionRow)
{
	if (!ActionRow.bUseLaunch)
	{
		return;
	}

	if (!OwnerCharacter)
	{
		CacheOwnerReferences();
	}

	if (!OwnerCharacter)
	{
		return;
	}

	FVector HorizontalDirection(HitData.HitDirection.X, HitData.HitDirection.Y, 0.0f);
	if (HorizontalDirection.IsNearlyZero())
	{
		HorizontalDirection = -OwnerCharacter->GetActorForwardVector();
	}
	HorizontalDirection = HorizontalDirection.GetSafeNormal2D();

	const float HorizontalSpeed = ActionRow.LaunchDuration > KINDA_SMALL_NUMBER
		? FMath::Max(0.0f, ActionRow.LaunchDistance) / ActionRow.LaunchDuration
		: 0.0f;
	FVector LaunchVelocity = HorizontalDirection * HorizontalSpeed;
	LaunchVelocity.Z = ActionRow.LaunchVerticalSpeed;

	if (LaunchVelocity.IsNearlyZero())
	{
		return;
	}

	OwnerCharacter->LaunchCharacter(LaunchVelocity, true, true);
}

bool UMVHitReactionComponent::TryConsumeBufferedRecoveryInput()
{
	if (!CachedInputManager)
	{
		CacheOwnerReferences();
	}

	if (!CachedInputManager)
	{
		return false;
	}

	FGameplayTag ActionInputTag;
	FVector2D ControllerSpaceInput = FVector2D::ZeroVector;
	bool bHasMovementInput = false;
	if (!CachedInputManager->TryGetBufferedActionInput(
		ActionInputTag,
		ControllerSpaceInput,
		bHasMovementInput))
	{
		return false;
	}

	if (!TryConsumeRecoveryInput(ActionInputTag, ControllerSpaceInput, bHasMovementInput))
	{
		return false;
	}

	CachedInputManager->ClearBufferedActionInput();
	return true;
}

bool UMVHitReactionComponent::TryConsumeBufferedRecoveryMovementInput()
{
	if (!CachedInputManager)
	{
		CacheOwnerReferences();
	}

	if (!CachedInputManager)
	{
		return false;
	}

	// Recovery window는 입력을 받는 구간이 아니라, 이미 저장된 이동 의도를 일반 탈출로 소비할 수 있는 구간이다.
	FVector2D ControllerSpaceInput = FVector2D::ZeroVector;
	if (!CachedInputManager->TryGetRecentActionMovementInput(ControllerSpaceInput))
	{
		return false;
	}

	return TryConsumeRecoveryMovementInput(ControllerSpaceInput, true);
}

bool UMVHitReactionComponent::TryConsumeRecoveryInput(
	const FGameplayTag ActionInputTag,
	const FVector2D ControllerSpaceInput,
	const bool bHasMovementInput)
{
	if (!CachedActionComponent || !CachedInputManager)
	{
		CacheOwnerReferences();
		BindInputManagerHandlers();
		BindActionComponentHandlers();
	}

	if (!CachedActionComponent
		|| !CachedInputManager
		|| ActiveHitReactionActionRowName.IsNone()
		|| CachedActionComponent->GetActiveActionRowName() != ActiveHitReactionActionRowName
		|| !CachedInputManager->IsRecoveryEscapeWindowOpen())
	{
		return false;
	}

	if (CachedStatComponent && CachedStatComponent->IsDead())
	{
		return false;
	}

	if (bActiveHitReactionActionIsRecoveryAction)
	{
		return false;
	}

	if (ShouldCancelRecoveryInputDirectly())
	{
		return TryCancelActiveRecoveryAction();
	}

	if (ActionInputTag.MatchesTagExact(MVGameplayTags::Action_Input_Dodge))
	{
		const EMVActionInputDirection Direction = bHasMovementInput
			? CachedInputManager->ResolveActionInputDirection(ControllerSpaceInput)
			: DefaultEscapeDodgeDirection;
		return TryStartEscapeDodgeRecoveryAction(Direction);
	}

	return TryStartDefaultRecoveryAction();
}

bool UMVHitReactionComponent::TryConsumeRecoveryMovementInput(
	const FVector2D ControllerSpaceInput,
	const bool bHasMovementInput)
{
	if (!bHasMovementInput)
	{
		return false;
	}

	if (!CachedActionComponent || !CachedInputManager)
	{
		CacheOwnerReferences();
		BindInputManagerHandlers();
		BindActionComponentHandlers();
	}

	if (!CachedActionComponent
		|| !CachedInputManager
		|| ActiveHitReactionActionRowName.IsNone()
		|| CachedActionComponent->GetActiveActionRowName() != ActiveHitReactionActionRowName
		|| !CachedInputManager->IsRecoveryEscapeWindowOpen())
	{
		return false;
	}

	if (CachedStatComponent && CachedStatComponent->IsDead())
	{
		return false;
	}

	const EMVActionInputDirection Direction = CachedInputManager->ResolveActionInputDirection(ControllerSpaceInput);
	if (Direction == EMVActionInputDirection::None)
	{
		return false;
	}

	if (bActiveHitReactionActionIsRecoveryAction)
	{
		return TryCancelActiveRecoveryAction();
	}

	if (ShouldCancelRecoveryInputDirectly())
	{
		return TryCancelActiveRecoveryAction();
	}

	return TryStartEscapeDodgeRecoveryAction(Direction);
}

bool UMVHitReactionComponent::TryStartDefaultRecoveryAction(const bool bRequireRecoveryWindow)
{
	if (!CachedActionComponent)
	{
		CacheOwnerReferences();
		BindActionComponentHandlers();
	}

	if (!CachedActionComponent
		|| ActiveHitReactionActionRowName.IsNone()
		|| bActiveHitReactionActionIsRecoveryAction
		|| ShouldCancelRecoveryInputDirectly())
	{
		return false;
	}

	if (CachedStatComponent && CachedStatComponent->IsDead())
	{

		return false;
	}

	if (CachedActionComponent->GetActiveActionRowName() != ActiveHitReactionActionRowName)
	{
		return false;
	}

	if (bRequireRecoveryWindow && !CachedInputManager->IsRecoveryEscapeWindowOpen())
	{
		return false;
	}

	const FGameplayTag CharacterIndexCode = ResolveCharacterIndexCode();
	FDataTableRowHandle RecoveryActionRowHandle;
	if (!ResolveRecoveryActionRowHandle(
		MakeGetupRecoveryActionRowName(
			CharacterIndexCode,
			ActiveHitReactionDirection,
			DefaultRecoveryRowIndex),
		RecoveryActionRowHandle))
	{
		return false;
	}

	return TryStartRecoveryAction(
		RecoveryActionRowHandle,
		bRequireRecoveryWindow ? TEXT("DefaultRecoveryGetup") : TEXT("DefaultRecoveryNotify"),
		bRequireRecoveryWindow);
}

bool UMVHitReactionComponent::TryStartEscapeDodgeRecoveryAction(const EMVActionInputDirection Direction)
{
	if (CachedStatComponent && CachedStatComponent->IsDead())
	{
		return false;
	}

	EMVActionInputDirection EscapeDirection = Direction;
	if (EscapeDirection == EMVActionInputDirection::None)
	{
		EscapeDirection = DefaultEscapeDodgeDirection;
	}

	if (EscapeDirection == EMVActionInputDirection::None)
	{
		return false;
	}

	const FGameplayTag CharacterIndexCode = ResolveCharacterIndexCode();
	FDataTableRowHandle RecoveryActionRowHandle;
	if (!ResolveRecoveryActionRowHandle(
		MakeEscapeDodgeRecoveryActionRowName(
			CharacterIndexCode,
			ActiveHitReactionDirection,
			EscapeDirection,
			DefaultRecoveryRowIndex),
		RecoveryActionRowHandle))
	{
		return false;
	}

	AlignOwnerToControllerForEscapeDodge();
	return TryStartRecoveryAction(RecoveryActionRowHandle, TEXT("RecoveryEscapeDodge"), true);
}

void UMVHitReactionComponent::AlignOwnerToControllerForEscapeDodge() const
{
	AMVCharacterBase* Character = OwnerCharacter.Get();
	if (!Character)
	{
		Character = Cast<AMVCharacterBase>(GetOwner());
	}

	if (!Character)
	{
		return;
	}

	Character->SetActorRotation(Character->ResolveMovementInputReferenceRotation());
	if (UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement())
	{
		MovementComponent->bUseControllerDesiredRotation = false;
		MovementComponent->bOrientRotationToMovement = false;
	}
}

bool UMVHitReactionComponent::TryStartRecoveryAction(
	const FDataTableRowHandle ActionRowHandle,
	const TCHAR* Source,
	const bool bRequireRecoveryWindow)
{
	if (!CachedActionComponent)
	{
		CacheOwnerReferences();
		BindActionComponentHandlers();
	}

	if (!CachedActionComponent
		|| ActiveHitReactionActionRowName.IsNone()
		|| !ActionRowHandle.DataTable
		|| ActionRowHandle.RowName.IsNone())
	{
		return false;
	}

	if (CachedActionComponent->GetActiveActionRowName() != ActiveHitReactionActionRowName)
	{
		return false;
	}

	if (bRequireRecoveryWindow && !CachedInputManager->IsRecoveryEscapeWindowOpen())
	{
		return false;
	}

	const FMVActionRow* RecoveryActionRow = FindRecoveryActionRow(ActionRowHandle);
	if (!RecoveryActionRow)
	{
		UE_LOG(
			LogMVHitReactionComponent,
			Warning,
			TEXT("Recovery action row was not resolved. Source=%s, DataTable=%s, RowName=%s."),
			Source,
			*GetNameSafe(ActionRowHandle.DataTable),
			*ActionRowHandle.RowName.ToString());
		return false;
	}

	const bool bStarted = CachedActionComponent->TryTransitionActionFromRowHandle(
		ActionRowHandle,
		RecoveryActionRow->DefaultStartSection,
		RecoveryActionTransitionBlendOutTime);
	if (!bStarted)
	{
		UE_LOG(
			LogMVHitReactionComponent,
			Warning,
			TEXT("Recovery action start failed. Source=%s, DataTable=%s, RowName=%s."),
			Source,
			*GetNameSafe(ActionRowHandle.DataTable),
			*ActionRowHandle.RowName.ToString());
		return false;
	}

	ActiveHitReactionActionRowName = ActionRowHandle.RowName;
	bActiveHitReactionActionIsRecoveryAction = true;
	ResetAirborneLandDetector();
	return true;
}

bool UMVHitReactionComponent::HasBufferedRecoveryActionInput() const
{
	if (!CachedInputManager)
	{
		return false;
	}

	FGameplayTag ActionInputTag;
	FVector2D ControllerSpaceInput = FVector2D::ZeroVector;
	bool bHasMovementInput = false;
	return CachedInputManager->TryGetBufferedActionInput(
		ActionInputTag,
		ControllerSpaceInput,
		bHasMovementInput);
}

bool UMVHitReactionComponent::ShouldCancelRecoveryInputDirectly() const
{
	return ActiveHitReactionType == EMVActionHitReactionType::SmallHit
		|| ActiveHitReactionType == EMVActionHitReactionType::LargeHit;
}

void UMVHitReactionComponent::ClearActiveHitReactionState()
{
	ActiveHitReactionActionRowName = NAME_None;
	ActiveHitReactionType = EMVActionHitReactionType::None;
	ActiveHitReactionDirection = EMVHitReactionDirection::Front;
	bActiveHitReactionActionIsRecoveryAction = false;
	ResetAirborneLandDetector();
}

bool UMVHitReactionComponent::TryCancelActiveRecoveryAction()
{
	if (!CachedActionComponent
		|| ActiveHitReactionActionRowName.IsNone()
		|| CachedActionComponent->GetActiveActionRowName() != ActiveHitReactionActionRowName
		|| !CachedInputManager->IsRecoveryEscapeWindowOpen())
	{
		return false;
	}

	CachedActionComponent->CancelActiveAction(RecoveryEscapeCancelBlendOutTime);
	return true;
}

void UMVHitReactionComponent::BeginAirborneLandDetector()
{
	if (!OwnerCharacter || !CachedActionComponent)
	{
		CacheOwnerReferences();
		BindActionComponentHandlers();
	}

	++AirborneLandDetectorCount;
	bAirborneLandJumpRequested = false;

	if (const AMVCharacterBase* Character = OwnerCharacter.Get())
	{
		if (const UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement())
		{
			bAirborneLandDetectorSawFalling = bAirborneLandDetectorSawFalling || MovementComponent->IsFalling();
		}
	}

	BindAirborneMovementModeChanged();
	TryJumpAirborneLandSection();
}

void UMVHitReactionComponent::EndAirborneLandDetector()
{
	AirborneLandDetectorCount = FMath::Max(0, AirborneLandDetectorCount - 1);
	if (!IsAirborneLandDetectorActive())
	{
		UnbindAirborneMovementModeChanged();
		bAirborneLandDetectorSawFalling = false;
		bAirborneLandJumpRequested = false;
	}
}

bool UMVHitReactionComponent::RequestDefaultRecoveryAction()
{
	if (!CachedStatComponent)
	{
		CacheOwnerReferences();
	}

	const bool bDead = CachedStatComponent && CachedStatComponent->IsDead();

	if (bDead)
	{
		return false;
	}

	const bool bStarted = TryStartDefaultRecoveryAction(false);

	return bStarted;
}

bool UMVHitReactionComponent::IsAirborneLandDetectorActive() const
{
	return AirborneLandDetectorCount > 0;
}

void UMVHitReactionComponent::BindAirborneMovementModeChanged()
{
	if (!OwnerCharacter)
	{
		CacheOwnerReferences();
	}

	if (OwnerCharacter && !bAirborneMovementModeDelegateBound)
	{
		OwnerCharacter->MovementModeChangedDelegate.RemoveDynamic(
			this,
			&UMVHitReactionComponent::HandleOwnerMovementModeChanged);
		OwnerCharacter->MovementModeChangedDelegate.AddUniqueDynamic(
			this,
			&UMVHitReactionComponent::HandleOwnerMovementModeChanged);
		bAirborneMovementModeDelegateBound = true;
	}
}

void UMVHitReactionComponent::UnbindAirborneMovementModeChanged()
{
	if (OwnerCharacter && bAirborneMovementModeDelegateBound)
	{
		OwnerCharacter->MovementModeChangedDelegate.RemoveDynamic(
			this,
			&UMVHitReactionComponent::HandleOwnerMovementModeChanged);
	}
	bAirborneMovementModeDelegateBound = false;
}

void UMVHitReactionComponent::ResetAirborneLandDetector()
{
	UnbindAirborneMovementModeChanged();
	AirborneLandDetectorCount = 0;
	bAirborneLandDetectorSawFalling = false;
	bAirborneLandJumpRequested = false;
}

void UMVHitReactionComponent::TryJumpAirborneLandSection()
{
	if (!IsAirborneLandDetectorActive()
		|| bAirborneLandJumpRequested
		|| AirborneLandSectionName.IsNone())
	{
		return;
	}

	if (!OwnerCharacter || !CachedActionComponent)
	{
		CacheOwnerReferences();
		BindActionComponentHandlers();
	}

	if (!OwnerCharacter
		|| !CachedActionComponent
		|| ActiveHitReactionActionRowName.IsNone()
		|| CachedActionComponent->GetActiveActionRowName() != ActiveHitReactionActionRowName)
	{
		return;
	}

	const UCharacterMovementComponent* MovementComponent = OwnerCharacter->GetCharacterMovement();
	if (!MovementComponent)
	{
		return;
	}

	if (MovementComponent->IsFalling())
	{
		bAirborneLandDetectorSawFalling = true;
		return;
	}

	if (!MovementComponent->IsMovingOnGround())
	{
		return;
	}

	if (bRequireFallingBeforeAirborneLand && !bAirborneLandDetectorSawFalling)
	{
		return;
	}

	if (CachedActionComponent->TryJumpActiveActionSection(AirborneLandSectionName))
	{
		bAirborneLandJumpRequested = true;
	}
}

bool UMVHitReactionComponent::ResolveHitReactionActionRowHandle(
	const EMVActionHitReactionType HitReactionType,
	const EMVHitReactionDirection Direction,
	FMVHitReactionActionRowHandle& OutActionRowHandle)
{
	OutActionRowHandle.Reset();
	if (EvaluateHitReactionChooserActionRowHandle(OutActionRowHandle))
	{
		return true;
	}

	if (!bUseNamingConventionWhenChooserUnavailable || !OwnerCharacter)
	{
		return false;
	}

	const FGameplayTag CharacterIndexCode = ResolveCharacterIndexCode();
	return MakeHitReactionActionRowHandleFromNames(
		ResolveHitReactionActionTableName(),
		MakeHitReactionActionRowName(
			CharacterIndexCode,
			HitReactionType,
			Direction,
			DefaultHitReactionRowIndex),
		OutActionRowHandle);
}

bool UMVHitReactionComponent::CanTriggerGroggyByHitReactionType(
	const EMVActionHitReactionType HitReactionType) const
{
	return GroggyTriggerHitReactionTypes.Contains(HitReactionType);
}

bool UMVHitReactionComponent::CanTriggerGroggyByActionTag(const FName ActionTag) const
{
	static_cast<void>(ActionTag);
	// Todo: Add ActionTag-based groggy trigger filtering when attack tags are finalized.
	return true;
}

FName UMVHitReactionComponent::ResolveHitReactionActionTableName() const
{
	if (!HitReactionActionTableName.IsNone())
	{
		return HitReactionActionTableName;
	}

	return bUseNamingConventionWhenChooserUnavailable && OwnerCharacter
		? MakeHitReactionActionTableName(ResolveCharacterIndexCode())
		: NAME_None;
}

EMVHitReactionDirection UMVHitReactionComponent::ResolveSupportedHitReactionDirection(
	const EMVActionHitReactionType HitReactionType,
	const EMVHitReactionDirection Direction) const
{
	switch (HitReactionType)
	{
	case EMVActionHitReactionType::LargeHit:
	case EMVActionHitReactionType::Groggy:
		return EMVHitReactionDirection::Front;
	case EMVActionHitReactionType::KnockDown:
	case EMVActionHitReactionType::Airborne:
		return Direction == EMVHitReactionDirection::Back
			? EMVHitReactionDirection::Back
			: EMVHitReactionDirection::Front;
	case EMVActionHitReactionType::SmallHit:
	case EMVActionHitReactionType::None:
	default:
		return Direction;
	}
}

bool UMVHitReactionComponent::EvaluateHitReactionChooserActionRowHandle(FMVHitReactionActionRowHandle& OutActionRowHandle)
{
	OutActionRowHandle.Reset();
	ChooserHitReactionActionRowHandle.Reset();

	if (!HitReactionChooserTable.IsValid())
	{
		return false;
	}

	UChooserTable* ChooserTable = Cast<UChooserTable>(HitReactionChooserTable.TryLoad());
	if (!ChooserTable)
	{
		return false;
	}

	FChooserEvaluationContext Context;
	Context.AddObjectParam(this);
	Context.AddStructParam(ChooserHitReactionActionRowHandle);

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

	if (ChooserHitReactionActionRowHandle.IsValid())
	{
		OutActionRowHandle = ChooserHitReactionActionRowHandle;
		return true;
	}

	UObject* ResolvedObject = SelectedObject.IsValid()
		? SelectedObject.Get()
		: SelectedObject.LoadSynchronous();
	UDataTable* SelectedDataTable = Cast<UDataTable>(ResolvedObject);
	if (!SelectedDataTable)
	{
		return false;
	}

	OutActionRowHandle.ActionRow.DataTable = SelectedDataTable;
	OutActionRowHandle.ActionRow.RowName = MakeHitReactionActionRowName(
		ResolveCharacterIndexCode(),
		ChooserHitReactionType,
		ChooserHitReactionDirection,
		DefaultHitReactionRowIndex);
	return OutActionRowHandle.IsValid();
}

bool UMVHitReactionComponent::ResolveRecoveryActionRowHandle(
	const FName ActionRowName,
	FDataTableRowHandle& OutActionRowHandle) const
{
	FMVHitReactionActionRowHandle HitReactionActionRowHandle;
	if (!MakeHitReactionActionRowHandleFromNames(
		ResolveHitReactionActionTableName(),
		ActionRowName,
		HitReactionActionRowHandle))
	{
		OutActionRowHandle.DataTable = nullptr;
		OutActionRowHandle.RowName = NAME_None;
		return false;
	}

	OutActionRowHandle = HitReactionActionRowHandle.ActionRow;
	return OutActionRowHandle.DataTable && !OutActionRowHandle.RowName.IsNone();
}

FName UMVHitReactionComponent::MakeHitReactionActionTableName(
	const FGameplayTag CharacterIndexCode) const
{
	const FString CharacterIndexCodeToken = CharacterIndexCodeToTableToken(CharacterIndexCode);
	if (CharacterIndexCodeToken.IsEmpty())
	{
		return NAME_None;
	}

	return FName(*FString::Printf(
		TEXT("HR_%s"),
		*CharacterIndexCodeToken));
}

FName UMVHitReactionComponent::MakeHitReactionActionRowName(
	const FGameplayTag CharacterIndexCode,
	const EMVActionHitReactionType HitReactionType,
	const EMVHitReactionDirection Direction,
	const int32 Index) const
{
	const FString CharacterIndexCodeToken = CharacterIndexCodeToTableToken(CharacterIndexCode);
	if (CharacterIndexCodeToken.IsEmpty())
	{
		return NAME_None;
	}

	return FName(*FString::Printf(
		TEXT("HR_%s_%s_%s_%02d"),
		*CharacterIndexCodeToken,
		*HitReactionTypeToTableToken(HitReactionType),
		*HitReactionDirectionToTableToken(Direction),
		FMath::Max(1, Index)));
}

FName UMVHitReactionComponent::MakeGetupRecoveryActionRowName(
	const FGameplayTag CharacterIndexCode,
	const EMVHitReactionDirection Direction,
	const int32 Index) const
{
	const FString CharacterIndexCodeToken = CharacterIndexCodeToTableToken(CharacterIndexCode);
	if (CharacterIndexCodeToken.IsEmpty())
	{
		return NAME_None;
	}

	return FName(*FString::Printf(
		TEXT("HR_%s_%s_%s_%02d"),
		*CharacterIndexCodeToken,
		*MVHitReactionGetupRecoveryType.ToString(),
		*HitReactionDirectionToTableToken(Direction),
		FMath::Max(1, Index)));
}

FName UMVHitReactionComponent::MakeEscapeDodgeRecoveryActionRowName(
	const FGameplayTag CharacterIndexCode,
	const EMVHitReactionDirection FallDirection,
	const EMVActionInputDirection EscapeDirection,
	const int32 Index) const
{
	const FString CharacterIndexCodeToken = CharacterIndexCodeToTableToken(CharacterIndexCode);
	if (CharacterIndexCodeToken.IsEmpty())
	{
		return NAME_None;
	}

	return FName(*FString::Printf(
		TEXT("HR_%s_%s_%s_%s_%02d"),
		*CharacterIndexCodeToken,
		*MVHitReactionEscapeDodgeRecoveryType.ToString(),
		*HitReactionDirectionToTableToken(FallDirection),
		*ActionInputDirectionToTableToken(EscapeDirection),
		FMath::Max(1, Index)));
}

FGameplayTag UMVHitReactionComponent::ResolveCharacterIndexCode() const
{
	const AMVCharacterBase* Character = OwnerCharacter.Get();
	if (!Character)
	{
		Character = Cast<AMVCharacterBase>(GetOwner());
	}

	if (!Character)
	{
		return FGameplayTag();
	}

	const FGameplayTag CharacterIndexCode = Character->GetCharacterIndexCode();
	if (CharacterIndexCode.IsValid())
	{
		return CharacterIndexCode;
	}

	return FGameplayTag();
}

bool UMVHitReactionComponent::MakeHitReactionActionRowHandleFromNames(
	const FName ActionTableName,
	const FName ActionRowName,
	FMVHitReactionActionRowHandle& OutActionRowHandle) const
{
	OutActionRowHandle.Reset();
	const UMVTableManager* TableManager = UMVTableManager::Get(this);
	if (!TableManager)
	{
		UE_LOG(LogMVHitReactionComponent, Warning, TEXT("TableManager is not available."));
		return false;
	}

	if (ActionTableName.IsNone() || ActionRowName.IsNone())
	{
		return false;
	}

	UDataTable* DataTable = const_cast<UDataTable*>(TableManager->FindDataTable(ActionTableName));
	if (!DataTable)
	{
		UE_LOG(
			LogMVHitReactionComponent,
			Warning,
			TEXT("HitReaction table is not loaded in manifest: %s."),
			*ActionTableName.ToString());
		return false;
	}

	OutActionRowHandle.ActionRow.DataTable = DataTable;
	OutActionRowHandle.ActionRow.RowName = ActionRowName;
	return OutActionRowHandle.IsValid();
}

const FMVHitReactionActionRow* UMVHitReactionComponent::FindHitReactionActionRow(
	const FDataTableRowHandle ActionRowHandle) const
{
	if (!ActionRowHandle.DataTable || ActionRowHandle.RowName.IsNone())
	{
		return nullptr;
	}

	if (!ActionRowHandle.DataTable->GetRowStruct()
		|| !ActionRowHandle.DataTable->GetRowStruct()->IsChildOf(FMVHitReactionActionRow::StaticStruct()))
	{
		UE_LOG(
			LogMVHitReactionComponent,
			Warning,
			TEXT("HitReaction row handle has invalid row struct. DataTable=%s RowStruct=%s Expected=MVHitReactionActionRow."),
			*GetNameSafe(ActionRowHandle.DataTable),
			ActionRowHandle.DataTable->GetRowStruct()
				? *ActionRowHandle.DataTable->GetRowStruct()->GetName()
				: TEXT("None"));
		return nullptr;
	}

	const FMVHitReactionActionRow* Row = ActionRowHandle.DataTable->FindRow<FMVHitReactionActionRow>(
		ActionRowHandle.RowName,
		TEXT("MVHitReactionComponent"),
		false);
	if (!Row)
	{
		UE_LOG(
			LogMVHitReactionComponent,
			Warning,
			TEXT("HitReaction row '%s' was not found in table '%s'. AvailableRows=%s."),
			*ActionRowHandle.RowName.ToString(),
			*GetNameSafe(ActionRowHandle.DataTable),
			*MVHitReactionBuildAvailableRowNameLog(*ActionRowHandle.DataTable));
	}

	return Row;
}

const FMVActionRow* UMVHitReactionComponent::FindRecoveryActionRow(
	const FDataTableRowHandle ActionRowHandle) const
{
	if (!ActionRowHandle.DataTable || ActionRowHandle.RowName.IsNone())
	{
		return nullptr;
	}

	if (!ActionRowHandle.DataTable->GetRowStruct()
		|| !ActionRowHandle.DataTable->GetRowStruct()->IsChildOf(FMVActionRow::StaticStruct()))
	{
		UE_LOG(
			LogMVHitReactionComponent,
			Warning,
			TEXT("Recovery action row handle has invalid row struct. DataTable=%s RowStruct=%s Expected=MVActionRow or child."),
			*GetNameSafe(ActionRowHandle.DataTable),
			ActionRowHandle.DataTable->GetRowStruct()
				? *ActionRowHandle.DataTable->GetRowStruct()->GetName()
				: TEXT("None"));
		return nullptr;
	}

	const FMVActionRow* Row = ActionRowHandle.DataTable->FindRow<FMVActionRow>(
		ActionRowHandle.RowName,
		TEXT("MVHitReactionComponent"),
		false);
	if (!Row)
	{
		UE_LOG(
			LogMVHitReactionComponent,
			Warning,
			TEXT("Recovery action row '%s' was not found in table '%s'. AvailableRows=%s."),
			*ActionRowHandle.RowName.ToString(),
			*GetNameSafe(ActionRowHandle.DataTable),
			*MVHitReactionBuildAvailableRowNameLog(*ActionRowHandle.DataTable));
	}

	return Row;
}

FString UMVHitReactionComponent::CharacterIndexCodeToTableToken(const FGameplayTag CharacterIndexCode)
{
	if (!CharacterIndexCode.IsValid())
	{
		return FString();
	}

	const FString TagString = CharacterIndexCode.ToString();
	FString TagPrefix;
	FString TagLeaf;
	return TagString.Split(TEXT("."), &TagPrefix, &TagLeaf, ESearchCase::CaseSensitive, ESearchDir::FromEnd)
		? TagLeaf
		: TagString;
}

FString UMVHitReactionComponent::HitReactionTypeToTableToken(const EMVActionHitReactionType HitReactionType)
{
	switch (HitReactionType)
	{
	case EMVActionHitReactionType::SmallHit:
		return TEXT("SH");
	case EMVActionHitReactionType::LargeHit:
		return TEXT("LH");
	case EMVActionHitReactionType::KnockDown:
		return TEXT("KD");
	case EMVActionHitReactionType::Airborne:
		return TEXT("AB");
	case EMVActionHitReactionType::Groggy:
		return TEXT("GR");
	case EMVActionHitReactionType::None:
	default:
		return TEXT("NO");
	}
}

FString UMVHitReactionComponent::HitReactionDirectionToTableToken(const EMVHitReactionDirection Direction)
{
	switch (Direction)
	{
	case EMVHitReactionDirection::Left:
		return TEXT("L");
	case EMVHitReactionDirection::Right:
		return TEXT("R");
	case EMVHitReactionDirection::Back:
		return TEXT("B");
	case EMVHitReactionDirection::Front:
	default:
		return TEXT("F");
	}
}

FString UMVHitReactionComponent::ActionInputDirectionToTableToken(const EMVActionInputDirection Direction)
{
	switch (Direction)
	{
	case EMVActionInputDirection::Left:
		return TEXT("L");
	case EMVActionInputDirection::Right:
		return TEXT("R");
	case EMVActionInputDirection::Back:
		return TEXT("B");
	case EMVActionInputDirection::Forward:
	default:
		return TEXT("F");
	}
}

bool UMVHitReactionComponent::TryHandleActionInput(
	const FGameplayTag ActionInputTag,
	const FVector2D ControllerSpaceInput,
	const bool bHasMovementInput)
{
	return TryConsumeRecoveryInput(ActionInputTag, ControllerSpaceInput, bHasMovementInput);
}

void UMVHitReactionComponent::HandleActionEnded(
	const FName ActionTableName,
	const FName ActionRowName,
	bool bInterrupted)
{
	if (!ActiveHitReactionActionRowName.IsNone() && ActionRowName == ActiveHitReactionActionRowName)
	{
		ClearActiveHitReactionState();
	}
}

bool UMVHitReactionComponent::TryHandleRecoveryWindowOpened()
{
	if (bActiveHitReactionActionIsRecoveryAction && HasBufferedRecoveryActionInput())
	{
		return false;
	}

	// window는 기본 Getup 시작점이 아니라, 이전에 저장된 이동 의도를 KD/AB 탈출로 소비할 수 있는 구간이다.
	return TryConsumeBufferedRecoveryMovementInput();
}

void UMVHitReactionComponent::HandleOwnerMovementModeChanged(
	ACharacter* Character,
	const EMovementMode PrevMovementMode,
	const uint8 PreviousCustomMode)
{
	if (!OwnerCharacter || Character != OwnerCharacter.Get())
	{
		return;
	}

	if (const UCharacterMovementComponent* MovementComponent = OwnerCharacter->GetCharacterMovement())
	{
		bAirborneLandDetectorSawFalling = bAirborneLandDetectorSawFalling
			|| PrevMovementMode == MOVE_Falling
			|| MovementComponent->IsFalling();
	}

	TryJumpAirborneLandSection();
}
