#include "Components/MVHitReactionComponent.h"

#include "Character/MVCharacterBase.h"
#include "Chooser.h"
#include "Components/MVActionComponent.h"
#include "Engine/DataTable.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Tables/MVCharacterTableTypes.h"
#include "Tables/MVTableManager.h"

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
}

void UMVHitReactionComponent::BeginPlay()
{
	Super::BeginPlay();

	CacheOwnerReferences();
	BindInputManagerHandlers();
	BindActionComponentHandlers();
}

void UMVHitReactionComponent::HandleDamaged(const FMVResolvedHitData& HitData)
{
	if (HitData.HitReactionType == EMVActionHitReactionType::None)
	{
		return;
	}

	if (!OwnerCharacter || !CachedActionComponent)
	{
		CacheOwnerReferences();
	}

	if (!OwnerCharacter || !CachedActionComponent)
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
		if (bActiveActionIsHitReaction && !CachedActionComponent->IsRecoveryEscapeWindowOpen())
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

	const bool bStarted = CachedActionComponent->TryStartActionFromTable(
		ActionData.ActionTableName,
		ActionData.ActionRowName,
		ActionData.StartSection);
	if (!bStarted)
	{
		UE_LOG(
			LogMVHitReactionComponent,
			Warning,
			TEXT("ActionComponent failed to start hit reaction. Table=%s, RowName=%s, Section=%s."),
			*ActionData.ActionTableName.ToString(),
			*ActionData.ActionRowName.ToString(),
			*ActionData.StartSection.ToString());
	}
	if (bStarted)
	{
		ResetAirborneLandDetector();
		ActiveHitReactionActionRowName = ActionData.ActionRowName;
		ActiveHitReactionType = HitData.HitReactionType;
		ActiveHitReactionDirection = ActionData.Direction;
		ApplyHitReactionLaunch(HitData, ActionData.ActionRow);
		TryConsumeBufferedRecoveryInput();
	}
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
}

void UMVHitReactionComponent::BindInputManagerHandlers()
{
	if (!CachedInputManager)
	{
		CacheOwnerReferences();
	}

	if (CachedInputManager)
	{
		CachedInputManager->OnActionInputSubmitted.RemoveDynamic(
			this,
			&UMVHitReactionComponent::HandleActionInputSubmitted);
		CachedInputManager->OnActionInputSubmitted.AddUniqueDynamic(
			this,
			&UMVHitReactionComponent::HandleActionInputSubmitted);
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
		CachedActionComponent->OnRecoveryEscapeWindowChanged.RemoveDynamic(
			this,
			&UMVHitReactionComponent::HandleRecoveryEscapeWindowChanged);
		CachedActionComponent->OnRecoveryEscapeWindowChanged.AddUniqueDynamic(
			this,
			&UMVHitReactionComponent::HandleRecoveryEscapeWindowChanged);
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

	const FName ActionTableName = ResolveHitReactionActionTableName(HitData.HitReactionType, Direction);
	if (ActionTableName.IsNone())
	{
		UE_LOG(
			LogMVHitReactionComponent,
			Warning,
			TEXT("HitReaction table was not resolved. CharacterIndexCode=%s, HitReactionType=%d, Direction=%d."),
			*ChooserCharacterIndexCode.ToString(),
			static_cast<int32>(HitData.HitReactionType),
			static_cast<int32>(Direction));
		return false;
	}

	const FName ActionRowName = MakeHitReactionActionRowName(
		ChooserCharacterIndexCode,
		HitData.HitReactionType,
		Direction,
		DefaultHitReactionRowIndex);
	const FMVHitReactionActionRow* ActionRow = FindHitReactionActionRow(ActionTableName, ActionRowName);
	if (!ActionRow)
	{
		UE_LOG(
			LogMVHitReactionComponent,
			Warning,
			TEXT("HitReaction row was not resolved. Table=%s, RowName=%s."),
			*ActionTableName.ToString(),
			*ActionRowName.ToString());
		return false;
	}

	OutActionData.ActionTableName = ActionTableName;
	OutActionData.ActionRowName = ActionRowName;
	OutActionData.StartSection = ActionRow->DefaultStartSection;
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

	int32 ActionId = INDEX_NONE;
	FVector2D ControllerSpaceInput = FVector2D::ZeroVector;
	bool bHasMovementInput = false;
	if (!CachedInputManager->TryGetBufferedActionInput(
		ActionId,
		ControllerSpaceInput,
		bHasMovementInput))
	{
		return false;
	}

	if (!TryConsumeRecoveryInput(ActionId, ControllerSpaceInput, bHasMovementInput))
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
	const int32 ActionId,
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
		|| !CachedActionComponent->IsRecoveryEscapeWindowOpen())
	{
		return false;
	}

	if (ShouldCancelRecoveryInputDirectly())
	{
		return TryCancelActiveRecoveryAction();
	}

	if (ActionId == MVActionIds::Dodge)
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
		|| !CachedActionComponent->IsRecoveryEscapeWindowOpen())
	{
		return false;
	}

	const EMVActionInputDirection Direction = CachedInputManager->ResolveActionInputDirection(ControllerSpaceInput);
	if (Direction == EMVActionInputDirection::None)
	{
		return false;
	}

	if (ShouldCancelRecoveryInputDirectly())
	{
		return TryCancelActiveRecoveryAction();
	}

	return TryStartDefaultRecoveryAction();
}

bool UMVHitReactionComponent::TryStartDefaultRecoveryAction()
{
	if (!CachedActionComponent)
	{
		CacheOwnerReferences();
		BindActionComponentHandlers();
	}

	if (!CachedActionComponent
		|| ActiveHitReactionActionRowName.IsNone()
		|| CachedActionComponent->GetActiveActionRowName() != ActiveHitReactionActionRowName
		|| !CachedActionComponent->IsRecoveryEscapeWindowOpen()
		|| ShouldCancelRecoveryInputDirectly())
	{
		return false;
	}

	const FGameplayTag CharacterIndexCode = ResolveCharacterIndexCode();
	return TryTransitionRecoveryAction(
		ResolveGetupRecoveryActionTableName(),
		MakeGetupRecoveryActionRowName(
			CharacterIndexCode,
			ActiveHitReactionDirection,
			DefaultRecoveryRowIndex),
		TEXT("DefaultRecoveryGetup"));
}

bool UMVHitReactionComponent::TryStartEscapeDodgeRecoveryAction(const EMVActionInputDirection Direction)
{
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
	return TryTransitionRecoveryAction(
		ResolveEscapeDodgeRecoveryActionTableName(),
		MakeEscapeDodgeRecoveryActionRowName(
			CharacterIndexCode,
			ActiveHitReactionDirection,
			EscapeDirection,
			DefaultRecoveryRowIndex),
		TEXT("RecoveryEscapeDodge"));
}

bool UMVHitReactionComponent::TryTransitionRecoveryAction(
	const FName ActionTableName,
	const FName ActionRowName,
	const TCHAR* Source)
{
	if (!CachedActionComponent)
	{
		CacheOwnerReferences();
		BindActionComponentHandlers();
	}

	if (!CachedActionComponent
		|| ActiveHitReactionActionRowName.IsNone()
		|| CachedActionComponent->GetActiveActionRowName() != ActiveHitReactionActionRowName
		|| !CachedActionComponent->IsRecoveryEscapeWindowOpen()
		|| ActionTableName.IsNone()
		|| ActionRowName.IsNone())
	{
		return false;
	}

	const FMVActionRow* RecoveryActionRow = FindRecoveryActionRow(ActionTableName, ActionRowName);
	if (!RecoveryActionRow)
	{
		UE_LOG(
			LogMVHitReactionComponent,
			Warning,
			TEXT("Recovery action row was not resolved. Source=%s, Table=%s, RowName=%s."),
			Source,
			*ActionTableName.ToString(),
			*ActionRowName.ToString());
		return false;
	}

	const bool bStarted = CachedActionComponent->TryTransitionActionFromTable(
		ActionTableName,
		ActionRowName,
		RecoveryActionRow->DefaultStartSection,
		RecoveryActionTransitionBlendOutTime);
	if (!bStarted)
	{
		UE_LOG(
			LogMVHitReactionComponent,
			Warning,
			TEXT("Recovery action transition failed. Source=%s, Table=%s, RowName=%s."),
			Source,
			*ActionTableName.ToString(),
			*ActionRowName.ToString());
		return false;
	}

	ActiveHitReactionActionRowName = NAME_None;
	ActiveHitReactionType = EMVActionHitReactionType::None;
	ActiveHitReactionDirection = EMVHitReactionDirection::Front;
	return true;
}

bool UMVHitReactionComponent::ShouldCancelRecoveryInputDirectly() const
{
	return ActiveHitReactionType == EMVActionHitReactionType::SmallHit
		|| ActiveHitReactionType == EMVActionHitReactionType::LargeHit;
}

bool UMVHitReactionComponent::TryCancelActiveRecoveryAction()
{
	if (!CachedActionComponent
		|| ActiveHitReactionActionRowName.IsNone()
		|| CachedActionComponent->GetActiveActionRowName() != ActiveHitReactionActionRowName
		|| !CachedActionComponent->IsRecoveryEscapeWindowOpen())
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

FName UMVHitReactionComponent::ResolveHitReactionActionTableName(
	const EMVActionHitReactionType HitReactionType,
	const EMVHitReactionDirection Direction) const
{
	const FName ChooserTableName = EvaluateHitReactionChooserTable();
	if (!ChooserTableName.IsNone())
	{
		return ChooserTableName;
	}

	return bUseNamingConventionWhenChooserUnavailable && OwnerCharacter
		? MakeHitReactionActionTableName(ResolveCharacterIndexCode(), HitReactionType, Direction)
		: NAME_None;
}

EMVHitReactionDirection UMVHitReactionComponent::ResolveSupportedHitReactionDirection(
	const EMVActionHitReactionType HitReactionType,
	const EMVHitReactionDirection Direction) const
{
	switch (HitReactionType)
	{
	case EMVActionHitReactionType::LargeHit:
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

FName UMVHitReactionComponent::EvaluateHitReactionChooserTable() const
{
	if (!HitReactionChooserTable.IsValid())
	{
		return NAME_None;
	}

	UChooserTable* ChooserTable = Cast<UChooserTable>(HitReactionChooserTable.TryLoad());
	if (!ChooserTable)
	{
		return NAME_None;
	}

	FChooserEvaluationContext Context;
	Context.AddObjectParam(const_cast<UMVHitReactionComponent*>(this));
	if (UObject* OwnerObject = GetOwner())
	{
		Context.AddObjectParam(OwnerObject);
	}

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
	const UDataTable* SelectedDataTable = Cast<UDataTable>(ResolvedObject);
	if (!SelectedDataTable)
	{
		return NAME_None;
	}

	FString TableName = SelectedDataTable->GetName();
	TableName.RemoveFromStart(TEXT("DT_"));
	return FName(*TableName);
}

FName UMVHitReactionComponent::ResolveGetupRecoveryActionTableName() const
{
	if (!GetupRecoveryActionTableName.IsNone())
	{
		return GetupRecoveryActionTableName;
	}

	return MakeHitReactionRecoveryActionTableName(
		ResolveCharacterIndexCode(),
		MVHitReactionGetupRecoveryType);
}

FName UMVHitReactionComponent::ResolveEscapeDodgeRecoveryActionTableName() const
{
	if (!EscapeDodgeRecoveryActionTableName.IsNone())
	{
		return EscapeDodgeRecoveryActionTableName;
	}

	return MakeHitReactionRecoveryActionTableName(
		ResolveCharacterIndexCode(),
		MVHitReactionEscapeDodgeRecoveryType);
}

FName UMVHitReactionComponent::MakeHitReactionActionTableName(
	const FGameplayTag CharacterIndexCode,
	const EMVActionHitReactionType HitReactionType,
	const EMVHitReactionDirection Direction) const
{
	const FString CharacterIndexCodeToken = CharacterIndexCodeToTableToken(CharacterIndexCode);
	if (CharacterIndexCodeToken.IsEmpty())
	{
		return NAME_None;
	}

	return FName(*FString::Printf(
		TEXT("HR_%s_%s_%s"),
		*CharacterIndexCodeToken,
		*HitReactionTypeToTableToken(HitReactionType),
		*HitReactionDirectionToTableToken(Direction)));
}

FName UMVHitReactionComponent::MakeHitReactionRecoveryActionTableName(
	const FGameplayTag CharacterIndexCode,
	const FName RecoveryType) const
{
	const FString CharacterIndexCodeToken = CharacterIndexCodeToTableToken(CharacterIndexCode);
	if (CharacterIndexCodeToken.IsEmpty() || RecoveryType.IsNone())
	{
		return NAME_None;
	}

	return FName(*FString::Printf(
		TEXT("HR_%s_%s"),
		*CharacterIndexCodeToken,
		*RecoveryType.ToString()));
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

const FMVHitReactionActionRow* UMVHitReactionComponent::FindHitReactionActionRow(
	const FName ActionTableName,
	const FName ActionRowName) const
{
	UMVTableManager* TableManager = UMVTableManager::Get(this);
	if (!TableManager)
	{
		UE_LOG(LogMVHitReactionComponent, Warning, TEXT("TableManager is not available."));
		return nullptr;
	}

	if (ActionTableName.IsNone() || ActionRowName.IsNone())
	{
		return nullptr;
	}

	const UDataTable* DataTable = TableManager->FindDataTable(ActionTableName);
	if (!DataTable)
	{
		UE_LOG(
			LogMVHitReactionComponent,
			Warning,
			TEXT("HitReaction table is not loaded in manifest: %s."),
			*ActionTableName.ToString());
		return nullptr;
	}

	if (!DataTable->GetRowStruct() || !DataTable->GetRowStruct()->IsChildOf(FMVHitReactionActionRow::StaticStruct()))
	{
		UE_LOG(
			LogMVHitReactionComponent,
			Warning,
			TEXT("HitReaction table '%s' row struct is '%s', expected MVHitReactionActionRow."),
			*ActionTableName.ToString(),
			DataTable->GetRowStruct() ? *DataTable->GetRowStruct()->GetName() : TEXT("None"));
		return nullptr;
	}

	const FMVHitReactionActionRow* Row = TableManager->FindRow<FMVHitReactionActionRow>(
		ActionTableName,
		ActionRowName.ToString());
	if (!Row)
	{
		UE_LOG(
			LogMVHitReactionComponent,
			Warning,
			TEXT("HitReaction row '%s' was not found in table '%s'."),
			*ActionRowName.ToString(),
			*ActionTableName.ToString());
	}

	return Row;
}

const FMVActionRow* UMVHitReactionComponent::FindRecoveryActionRow(
	const FName ActionTableName,
	const FName ActionRowName) const
{
	UMVTableManager* TableManager = UMVTableManager::Get(this);
	if (!TableManager)
	{
		UE_LOG(LogMVHitReactionComponent, Warning, TEXT("TableManager is not available."));
		return nullptr;
	}

	if (ActionTableName.IsNone() || ActionRowName.IsNone())
	{
		return nullptr;
	}

	const UDataTable* DataTable = TableManager->FindDataTable(ActionTableName);
	if (!DataTable)
	{
		UE_LOG(
			LogMVHitReactionComponent,
			Warning,
			TEXT("Recovery action table is not loaded in manifest: %s."),
			*ActionTableName.ToString());
		return nullptr;
	}

	if (!DataTable->GetRowStruct() || !DataTable->GetRowStruct()->IsChildOf(FMVActionRow::StaticStruct()))
	{
		UE_LOG(
			LogMVHitReactionComponent,
			Warning,
			TEXT("Recovery action table '%s' row struct is '%s', expected MVActionRow or child."),
			*ActionTableName.ToString(),
			DataTable->GetRowStruct() ? *DataTable->GetRowStruct()->GetName() : TEXT("None"));
		return nullptr;
	}

	const FMVActionRow* Row = TableManager->FindRow<FMVActionRow>(ActionTableName, ActionRowName.ToString());
	if (!Row)
	{
		UE_LOG(
			LogMVHitReactionComponent,
			Warning,
			TEXT("Recovery action row '%s' was not found in table '%s'. RowStruct=%s, RowCount=%d, AvailableRows=%s. Reloading tables once and retrying."),
			*ActionRowName.ToString(),
			*ActionTableName.ToString(),
			DataTable->GetRowStruct() ? *DataTable->GetRowStruct()->GetName() : TEXT("None"),
			DataTable->GetRowMap().Num(),
			*MVHitReactionBuildAvailableRowNameLog(*DataTable));

		TableManager->ReloadAllTables();
		const UDataTable* ReloadedDataTable = TableManager->FindDataTable(ActionTableName);
		Row = TableManager->FindRow<FMVActionRow>(ActionTableName, ActionRowName.ToString());
		if (Row)
		{
			return Row;
		}

		UE_LOG(
			LogMVHitReactionComponent,
			Warning,
			TEXT("Recovery action row '%s' still missing after reload. Table=%s, RowStruct=%s, RowCount=%d, AvailableRows=%s."),
			*ActionRowName.ToString(),
			*ActionTableName.ToString(),
			ReloadedDataTable && ReloadedDataTable->GetRowStruct()
				? *ReloadedDataTable->GetRowStruct()->GetName()
				: TEXT("None"),
			ReloadedDataTable ? ReloadedDataTable->GetRowMap().Num() : 0,
			ReloadedDataTable ? *MVHitReactionBuildAvailableRowNameLog(*ReloadedDataTable) : TEXT("<table missing>"));
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

void UMVHitReactionComponent::HandleActionInputSubmitted(
	const int32 ActionId,
	const FVector2D ControllerSpaceInput,
	const bool bHasMovementInput)
{
	if (TryConsumeRecoveryInput(ActionId, ControllerSpaceInput, bHasMovementInput) && CachedInputManager)
	{
		CachedInputManager->ClearBufferedActionInput();
	}
}

void UMVHitReactionComponent::HandleActionEnded(
	const FName ActionTableName,
	const FName ActionRowName,
	bool bInterrupted)
{
	if (!ActiveHitReactionActionRowName.IsNone() && ActionRowName == ActiveHitReactionActionRowName)
	{
		ActiveHitReactionActionRowName = NAME_None;
		ActiveHitReactionType = EMVActionHitReactionType::None;
		ActiveHitReactionDirection = EMVHitReactionDirection::Front;
		ResetAirborneLandDetector();
	}
}

void UMVHitReactionComponent::HandleRecoveryEscapeWindowChanged(const bool bOpen)
{
	if (bOpen)
	{
		// window가 열리는 순간 이전에 저장된 액션 입력을 우선 소비하고, 없으면 최근 이동 입력으로 일반 탈출을 시도한다.
		if (!TryConsumeBufferedRecoveryInput())
		{
			if (!TryConsumeBufferedRecoveryMovementInput())
			{
				TryStartDefaultRecoveryAction();
			}
		}
	}
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
