#include "Character/PC/Dodge/MVPlayerDodge.h"

#include "Character/MVCharacterBase.h"
#include "Character/PC/MVPlayerCharacter.h"
#include "Chooser.h"
#include "Components/MVActionComponent.h"
#include "Components/MVInputManagerComponent.h"
#include "Components/MVStatComponent.h"
#include "Engine/DataTable.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Tables/MVTableManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogMVPlayerDodge, Log, All);

namespace
{
FString MVDodgeBuildAvailableRowNameLog(const UDataTable& DataTable)
{
	TArray<FString> RowNames;
	for (const FName RowName : DataTable.GetRowNames())
	{
		RowNames.Add(RowName.ToString());
	}

	RowNames.Sort();
	if (RowNames.IsEmpty())
	{
		return TEXT("<empty>");
	}

	constexpr int32 MaxLoggedRowCount = 16;
	TArray<FString> LoggedRowNames;
	const int32 LoggedCount = FMath::Min(RowNames.Num(), MaxLoggedRowCount);
	for (int32 Index = 0; Index < LoggedCount; ++Index)
	{
		LoggedRowNames.Add(RowNames[Index]);
	}

	FString Result = FString::Join(LoggedRowNames, TEXT(", "));
	if (RowNames.Num() > MaxLoggedRowCount)
	{
		Result += FString::Printf(TEXT(", ... (+%d more)"), RowNames.Num() - MaxLoggedRowCount);
	}
	return Result;
}

FString DodgeCharacterIndexCodeToTableToken(const FGameplayTag CharacterIndexCode)
{
	if (!CharacterIndexCode.IsValid())
	{
		return FString();
	}

	FString Token = CharacterIndexCode.GetTagName().ToString();
	int32 LastDotIndex = INDEX_NONE;
	if (Token.FindLastChar(TEXT('.'), LastDotIndex))
	{
		Token.RightChopInline(LastDotIndex + 1);
	}
	return Token;
}

bool MVDodgeIsHitReactionTerminalRecoveryAction(const FName ActionTableName, const FName ActionRowName)
{
	const FString TableName = ActionTableName.ToString();
	const FString RowName = ActionRowName.ToString();
	return TableName.StartsWith(TEXT("HR_"))
		&& (RowName.Contains(TEXT("_Getup_")) || RowName.Contains(TEXT("_EscapeDodge_")));
}

FVector2D DodgeClampControllerSpaceInput(const FVector2D& Input)
{
	const float SizeSquared = Input.SizeSquared();
	if (SizeSquared <= 1.0f)
	{
		return Input;
	}

	return Input / FMath::Sqrt(SizeSquared);
}

FRotator MakeYawRotationFromDirection(const FVector& Direction)
{
	return FRotator(0.0f, Direction.Rotation().Yaw, 0.0f);
}

FVector NormalizeMovementInputDirection(const FVector& Direction)
{
	const FVector Direction2D(Direction.X, Direction.Y, 0.0f);
	return Direction2D.IsNearlyZero()
		? FVector::ZeroVector
		: Direction2D.GetSafeNormal2D();
}

bool IsDiagonalDodgeDirection(const ELocomotionDirection Direction)
{
	return Direction == ELocomotionDirection::FL
		|| Direction == ELocomotionDirection::FR
		|| Direction == ELocomotionDirection::BL
		|| Direction == ELocomotionDirection::BR;
}

bool IsBackwardDiagonalDodgeDirection(const ELocomotionDirection Direction)
{
	return Direction == ELocomotionDirection::BL
		|| Direction == ELocomotionDirection::BR;
}

ELocomotionDirection ResolveDodgeEightWayDirection(const float MoveDirectionAngle)
{
	const float AbsAngle = FMath::Abs(MoveDirectionAngle);

	if (AbsAngle >= 157.5f)
	{
		return ELocomotionDirection::B;
	}

	if (AbsAngle >= 112.5f)
	{
		return MoveDirectionAngle >= 0.0f
			? ELocomotionDirection::BR
			: ELocomotionDirection::BL;
	}

	if (AbsAngle <= 22.5f)
	{
		return ELocomotionDirection::F;
	}

	if (AbsAngle <= 67.5f)
	{
		return MoveDirectionAngle >= 0.0f
			? ELocomotionDirection::FR
			: ELocomotionDirection::FL;
	}

	return MoveDirectionAngle >= 0.0f
		? ELocomotionDirection::R
		: ELocomotionDirection::L;
}

ELocomotionDirection DodgeResolveDirectionFromControllerSpaceInput(const FVector2D& ControllerSpaceInput)
{
	if (ControllerSpaceInput.IsNearlyZero())
	{
		return ELocomotionDirection::F;
	}

	const FVector2D NormalizedInput = ControllerSpaceInput.GetSafeNormal();
	const float DirectionAngle = FMath::RadiansToDegrees(FMath::Atan2(NormalizedInput.Y, NormalizedInput.X));
	return ResolveDodgeEightWayDirection(DirectionAngle);
}

FVector GetReferenceForwardVector(const FRotator& ReferenceRotation)
{
	return FRotationMatrix(ReferenceRotation).GetUnitAxis(EAxis::X).GetSafeNormal2D();
}

FVector GetReferenceRightVector(const FRotator& ReferenceRotation)
{
	return FRotationMatrix(ReferenceRotation).GetUnitAxis(EAxis::Y).GetSafeNormal2D();
}

FVector ResolveDirectionVectorFromReferenceRotation(
	const ELocomotionDirection Direction,
	const FRotator& ReferenceRotation)
{
	const FVector ForwardVector = GetReferenceForwardVector(ReferenceRotation);
	const FVector RightVector = GetReferenceRightVector(ReferenceRotation);

	switch (Direction)
	{
	case ELocomotionDirection::FR:
		return (ForwardVector + RightVector).GetSafeNormal2D();
	case ELocomotionDirection::R:
		return RightVector;
	case ELocomotionDirection::BR:
		return (-ForwardVector + RightVector).GetSafeNormal2D();
	case ELocomotionDirection::B:
		return -ForwardVector;
	case ELocomotionDirection::BL:
		return (-ForwardVector - RightVector).GetSafeNormal2D();
	case ELocomotionDirection::L:
		return -RightVector;
	case ELocomotionDirection::FL:
		return (ForwardVector - RightVector).GetSafeNormal2D();
	case ELocomotionDirection::F:
	default:
		return ForwardVector;
	}
}

FVector DodgeResolveWorldDirectionFromControllerSpaceInput(
	const FVector2D& ControllerSpaceInput,
	const FRotator& ReferenceRotation)
{
	if (ControllerSpaceInput.IsNearlyZero())
	{
		return FVector::ZeroVector;
	}

	const FVector ForwardVector = GetReferenceForwardVector(ReferenceRotation);
	const FVector RightVector = GetReferenceRightVector(ReferenceRotation);
	return (ForwardVector * ControllerSpaceInput.X + RightVector * ControllerSpaceInput.Y).GetSafeNormal2D();
}

FVector2D DodgeResolveControllerSpaceInputFromWorldDirection(
	const FVector& WorldDirection,
	const FRotator& ReferenceRotation)
{
	const FVector WorldDirection2D = NormalizeMovementInputDirection(WorldDirection);
	if (WorldDirection2D.IsNearlyZero())
	{
		return FVector2D::ZeroVector;
	}

	return DodgeClampControllerSpaceInput(FVector2D(
		FVector::DotProduct(WorldDirection2D, GetReferenceForwardVector(ReferenceRotation)),
		FVector::DotProduct(WorldDirection2D, GetReferenceRightVector(ReferenceRotation))));
}

FRotator ResolveStrafeReferenceRotation(const AMVCharacterBase& OwnerCharacter)
{
	return OwnerCharacter.ResolveMovementInputReferenceRotation();
}

FVector ResolveDodgeFacingDirection(
	const FVector& MovementInputDirection,
	const FVector& ReferenceForwardDirection,
	const bool bHasMovementInput,
	const bool bFreeDodge,
	const ELocomotionDirection InputDirection)
{
	if (!bHasMovementInput)
	{
		return FVector::ZeroVector;
	}

	if (bFreeDodge)
	{
		return MovementInputDirection;
	}

	if (IsBackwardDiagonalDodgeDirection(InputDirection))
	{
		return -MovementInputDirection;
	}

	if (IsDiagonalDodgeDirection(InputDirection))
	{
		return MovementInputDirection;
	}

	return ReferenceForwardDirection;
}

ELocomotionDirection ResolveDodgeChooserDirection(
	const bool bHasMovementInput,
	const ELocomotionDirection InputDirection)
{
	if (!bHasMovementInput)
	{
		return ELocomotionDirection::B;
	}

	switch (InputDirection)
	{
	case ELocomotionDirection::FL:
	case ELocomotionDirection::FR:
		return ELocomotionDirection::F;
	case ELocomotionDirection::BL:
	case ELocomotionDirection::BR:
		return ELocomotionDirection::B;
	default:
		return InputDirection;
	}
}
}

UMVPlayerDodge::UMVPlayerDodge()
{
}

UWorld* UMVPlayerDodge::GetWorld() const
{
	if (const AMVPlayerCharacter* PlayerCharacter = GetPlayerCharacter())
	{
		return PlayerCharacter->GetWorld();
	}

	return Super::GetWorld();
}

void UMVPlayerDodge::Initialize(AMVPlayerCharacter& InOwnerCharacter)
{
	if (OwnerPlayerCharacter.IsValid())
	{
		Deinitialize();
	}

	OwnerPlayerCharacter = &InOwnerCharacter;
	InOwnerCharacter.OnMovementInputReceived.AddUObject(this, &UMVPlayerDodge::HandleOwnerMovementInput);

	if (UMVInputManagerComponent* InputManager = InOwnerCharacter.InputManagerComponent)
	{
		InputManager->OnActionInputSubmitted.RemoveDynamic(
			this,
			&UMVPlayerDodge::HandleActionInputSubmitted);
		InputManager->OnActionInputSubmitted.AddUniqueDynamic(
			this,
			&UMVPlayerDodge::HandleActionInputSubmitted);
		InputManager->OnRecoveryEscapeWindowChanged.RemoveDynamic(
			this,
			&UMVPlayerDodge::HandleRecoveryEscapeWindowChanged);
		InputManager->OnRecoveryEscapeWindowChanged.AddUniqueDynamic(
			this,
			&UMVPlayerDodge::HandleRecoveryEscapeWindowChanged);
	}

	if (UMVActionComponent* ActionComponent = InOwnerCharacter.ActionComponent)
	{
		ActionComponent->OnActionEnded.RemoveDynamic(this, &UMVPlayerDodge::HandleActionEnded);
		ActionComponent->OnActionEnded.AddUniqueDynamic(this, &UMVPlayerDodge::HandleActionEnded);
	}
}

void UMVPlayerDodge::Deinitialize()
{
	AMVPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter)
	{
		return;
	}

	PlayerCharacter->OnMovementInputReceived.RemoveAll(this);

	if (UMVInputManagerComponent* InputManager = PlayerCharacter->InputManagerComponent)
	{
		InputManager->OnActionInputSubmitted.RemoveDynamic(
			this,
			&UMVPlayerDodge::HandleActionInputSubmitted);
		InputManager->OnRecoveryEscapeWindowChanged.RemoveDynamic(
			this,
			&UMVPlayerDodge::HandleRecoveryEscapeWindowChanged);
	}

	if (UMVActionComponent* ActionComponent = PlayerCharacter->ActionComponent)
	{
		ActionComponent->OnActionEnded.RemoveDynamic(this, &UMVPlayerDodge::HandleActionEnded);
	}

	EndLockOnPawnRotationSuppressionForDodge();
	ActiveDodgeActionTableName = NAME_None;
	ActiveDodgeActionRowName = NAME_None;
	OwnerPlayerCharacter.Reset();
}

AMVPlayerCharacter* UMVPlayerDodge::GetPlayerCharacter() const
{
	return OwnerPlayerCharacter.Get();
}

void UMVPlayerDodge::PrepareDodgeAction()
{
	AMVCharacterBase* OwnerCharacter = GetPlayerCharacter();
	if (!OwnerCharacter)
	{
		return;
	}

	const FVector2D ControllerSpaceMovementInput = CaptureControllerSpaceMovementInput(*OwnerCharacter);

	const bool bHasMovementInput = !ControllerSpaceMovementInput.IsNearlyZero();
	const bool bFreeDodge = bHasMovementInput
		&& !OwnerCharacter->CharacterInputState.WantsToStrafe
		&& !OwnerCharacter->CharacterInputState.WantsToAim;
	const bool bStrafeDodge = !bFreeDodge
		&& (OwnerCharacter->CharacterInputState.WantsToStrafe || OwnerCharacter->CharacterInputState.WantsToAim);
	const FRotator StrafeReferenceRotation = ResolveStrafeReferenceRotation(*OwnerCharacter);
	const FVector MovementInputDirection = DodgeResolveWorldDirectionFromControllerSpaceInput(
		ControllerSpaceMovementInput,
		StrafeReferenceRotation);

	const ELocomotionDirection InputDirection = bHasMovementInput
		? DodgeResolveDirectionFromControllerSpaceInput(ControllerSpaceMovementInput)
		: ELocomotionDirection::B;
	const ELocomotionDirection ChooserInputDirection = ResolveDodgeChooserDirection(
		bHasMovementInput,
		InputDirection);

	const FVector DodgeMovementDirection = bHasMovementInput && bStrafeDodge
		? ResolveDirectionVectorFromReferenceRotation(InputDirection, StrafeReferenceRotation)
		: MovementInputDirection;
	const FVector ReferenceForwardDirection = GetReferenceForwardVector(StrafeReferenceRotation);
	const FVector DodgeFacingDirection = ResolveDodgeFacingDirection(
		DodgeMovementDirection,
		ReferenceForwardDirection,
		bHasMovementInput,
		bFreeDodge,
		InputDirection);
	if (!DodgeFacingDirection.IsNearlyZero())
	{
		BeginLockOnPawnRotationSuppressionForDodge(*OwnerCharacter);
		OwnerCharacter->SetActorRotation(MakeYawRotationFromDirection(DodgeFacingDirection));
		if (UCharacterMovementComponent* MovementComponent = OwnerCharacter->GetCharacterMovement())
		{
			MovementComponent->bUseControllerDesiredRotation = false;
			MovementComponent->bOrientRotationToMovement = false;
		}
	}
	ApplyDodgeChooserSnapshot(*OwnerCharacter, bHasMovementInput, ChooserInputDirection, DodgeMovementDirection);

	CachedControllerSpaceMovementInput = FVector2D::ZeroVector;
	CachedControllerSpaceMovementInputFrame = 0;
}

void UMVPlayerDodge::UpdateBufferedDodgeMovementInput(const FVector& MovementInputDirection)
{
	HandleOwnerMovementInput(MovementInputDirection);
}

void UMVPlayerDodge::CacheControllerSpaceMovementInput(const FVector2D& ControllerSpaceMovementInput)
{
	CachedControllerSpaceMovementInput = DodgeClampControllerSpaceInput(ControllerSpaceMovementInput);
	CachedControllerSpaceMovementInputFrame = GFrameCounter;
}

void UMVPlayerDodge::HandleOwnerMovementInput(const FVector& MovementInputDirection)
{
	const AMVCharacterBase* OwnerCharacter = GetPlayerCharacter();
	FVector2D ControllerSpaceMovementInput = FVector2D::ZeroVector;
	if (OwnerCharacter)
	{
		if (!OwnerCharacter->TryGetControllerSpaceMovementInput(ControllerSpaceMovementInput, 0))
		{
			ControllerSpaceMovementInput = DodgeResolveControllerSpaceInputFromWorldDirection(
				MovementInputDirection,
				ResolveStrafeReferenceRotation(*OwnerCharacter));
		}
	}

	if (!ControllerSpaceMovementInput.IsNearlyZero())
	{
		CacheControllerSpaceMovementInput(ControllerSpaceMovementInput);
	}
}

void UMVPlayerDodge::BeginLockOnPawnRotationSuppressionForDodge(AMVCharacterBase& OwnerCharacter)
{
	if (bLockOnPawnRotationSuppressedForDodge)
	{
		return;
	}

	if (AMVPlayerCharacter* PlayerCharacter = Cast<AMVPlayerCharacter>(&OwnerCharacter))
	{
		PlayerCharacter->BeginLockOnPawnRotationSuppression();
		bLockOnPawnRotationSuppressedForDodge = true;
	}
}

void UMVPlayerDodge::EndLockOnPawnRotationSuppressionForDodge()
{
	if (!bLockOnPawnRotationSuppressedForDodge)
	{
		return;
	}

	if (AMVPlayerCharacter* PlayerCharacter = GetPlayerCharacter())
	{
		PlayerCharacter->EndLockOnPawnRotationSuppression();
	}
	bLockOnPawnRotationSuppressedForDodge = false;
}

FVector2D UMVPlayerDodge::CaptureControllerSpaceMovementInput(const AMVCharacterBase& OwnerCharacter) const
{
	if (CachedControllerSpaceMovementInputFrame == GFrameCounter
		&& !CachedControllerSpaceMovementInput.IsNearlyZero())
	{
		return CachedControllerSpaceMovementInput;
	}

	FVector2D CurrentControllerSpaceInput = FVector2D::ZeroVector;
	if (OwnerCharacter.TryGetControllerSpaceMovementInput(CurrentControllerSpaceInput, 0))
	{
		return DodgeClampControllerSpaceInput(CurrentControllerSpaceInput);
	}

	const FVector PendingInput2D(
		OwnerCharacter.GetPendingMovementInputVector().X,
		OwnerCharacter.GetPendingMovementInputVector().Y,
		0.0f);
	if (!PendingInput2D.IsNearlyZero())
	{
		return DodgeResolveControllerSpaceInputFromWorldDirection(
			PendingInput2D,
			ResolveStrafeReferenceRotation(OwnerCharacter));
	}

	return FVector2D::ZeroVector;
}

bool UMVPlayerDodge::TryStartDodgeAction()
{
	AMVCharacterBase* OwnerCharacter = GetPlayerCharacter();
	if (!OwnerCharacter)
	{
		UE_LOG(
			LogMVPlayerDodge,
			Warning,
			TEXT("TryStartDodgeAction failed because owner is not AMVCharacterBase. Owner=%s."),
			*GetNameSafe(GetPlayerCharacter()));
		return false;
	}

	UMVActionComponent* ActionComponent = OwnerCharacter
		? OwnerCharacter->ActionComponent
		: nullptr;
	if (!ActionComponent)
	{
		UE_LOG(
			LogMVPlayerDodge,
			Warning,
			TEXT("TryStartDodgeAction failed because ActionComponent is missing. Owner=%s."),
			*GetNameSafe(OwnerCharacter));
		return false;
	}
	UMVInputManagerComponent* InputManager = OwnerCharacter
		? OwnerCharacter->InputManagerComponent
		: nullptr;
	if (!InputManager)
	{
		UE_LOG(
			LogMVPlayerDodge,
			Warning,
			TEXT("TryStartDodgeAction failed because InputManagerComponent is missing. Owner=%s."),
			*GetNameSafe(OwnerCharacter));
		return false;
	}


	const bool bActionRunning = ActionComponent->IsActionRunning();
	const bool bCanTransitionActiveDodge = bActionRunning && CanTransitionActiveDodgeAction(*InputManager, *ActionComponent);
	if (bActionRunning && !bCanTransitionActiveDodge)
	{
		return false;
	}

	PrepareDodgeAction();

	FMVDodgeActionRowHandle ActionRowHandle;
	if (!ResolveDodgeActionRowHandle(ActionRowHandle))
	{
		UE_LOG(LogMVPlayerDodge, Warning, TEXT("TryStartDodgeAction failed because Dodge row handle was not resolved."));
		EndLockOnPawnRotationSuppressionForDodge();
		return false;
	}

	const FName ActionTableName = MakeDodgeActionTableName(ActionRowHandle.ActionRow.DataTable);
	const FName ActionRowName = ActionRowHandle.ActionRow.RowName;

	const FMVDodgeActionRow* DodgeActionRow = FindDodgeActionRow(ActionRowHandle.ActionRow);
	if (!DodgeActionRow)
	{
		EndLockOnPawnRotationSuppressionForDodge();
		return false;
	}

	if (!CanConsumeDodgeCost(*DodgeActionRow))
	{
		EndLockOnPawnRotationSuppressionForDodge();
		return false;
	}

	const FName StartSection = ActionRowHandle.StartSection.IsNone()
		? DodgeActionRow->DefaultStartSection
		: ActionRowHandle.StartSection;
	const bool bStarted = bCanTransitionActiveDodge
		? ActionComponent->TryTransitionActionFromRowHandle(ActionRowHandle.ActionRow, StartSection, RecoveryDodgeTransitionBlendOutTime)
		: ActionComponent->TryStartActionFromRowHandle(ActionRowHandle.ActionRow, StartSection);
	if (!bStarted)
	{
		UE_LOG(
			LogMVPlayerDodge,
			Warning,
			TEXT("TryStartDodgeAction failed because ActionComponent rejected %s. Table=%s Row=%s StartSection=%s."),
			bCanTransitionActiveDodge ? TEXT("transition") : TEXT("start"),
			*ActionTableName.ToString(),
			*ActionRowName.ToString(),
			*StartSection.ToString());
		EndLockOnPawnRotationSuppressionForDodge();
		return false;
	}

	BeginLockOnPawnRotationSuppressionForDodge(*OwnerCharacter);
	if (!ConsumeDodgeCost(*DodgeActionRow))
	{
		UE_LOG(
			LogMVPlayerDodge,
			Warning,
			TEXT("TryStartDodgeAction started but failed to consume stamina, cancelling. Table=%s Row=%s."),
			*ActionTableName.ToString(),
			*ActionRowName.ToString());
		EndLockOnPawnRotationSuppressionForDodge();
		ActionComponent->CancelActiveAction(0.0f);
		return false;
	}

	ActiveDodgeActionTableName = ActionTableName;
	ActiveDodgeActionRowName = ActionRowName;
	return true;
}

bool UMVPlayerDodge::TryConsumeBufferedDodgeInput()
{
	const AMVCharacterBase* OwnerCharacter = GetPlayerCharacter();
	UMVInputManagerComponent* InputManager = OwnerCharacter
		? OwnerCharacter->InputManagerComponent
		: nullptr;
	if (!InputManager)
	{
		return false;
	}

	int32 ActionId = INDEX_NONE;
	FVector2D ControllerSpaceInput = FVector2D::ZeroVector;
	bool bHasMovementInput = false;
	if (!InputManager->TryGetBufferedActionInput(
		ActionId,
		ControllerSpaceInput,
		bHasMovementInput)
		|| ActionId != MVActionIds::Dodge)
	{
		return false;
	}

	if (bHasMovementInput)
	{
		CacheControllerSpaceMovementInput(ControllerSpaceInput);
	}

	if (!TryStartDodgeAction())
	{
		return false;
	}

	InputManager->ClearBufferedActionInput();
	return true;
}

bool UMVPlayerDodge::ResolveDodgeActionRowHandle(FMVDodgeActionRowHandle& OutActionRowHandle)
{
	OutActionRowHandle.Reset();
	if (EvaluateDodgeChooserActionRowHandle(OutActionRowHandle))
	{
		return true;
	}

	if (!bUseNamingConventionWhenChooserUnavailable)
	{
		UE_LOG(LogMVPlayerDodge, Warning, TEXT("Dodge chooser did not resolve a row handle and naming fallback is disabled."));
		return false;
	}

	const FGameplayTag CharacterIndexCode = ResolveCharacterIndexCode();
	const FName ActionTableName = MakeDodgeActionTableName(CharacterIndexCode);
	const UMVTableManager* TableManager = UMVTableManager::Get(this);
	if (!TableManager)
	{
		UE_LOG(LogMVPlayerDodge, Warning, TEXT("Dodge naming fallback failed because TableManager is not available."));
		return false;
	}

	const UDataTable* ActionDataTable = TableManager
		? TableManager->FindDataTable(ActionTableName)
		: nullptr;
	if (!ActionDataTable)
	{
		UE_LOG(
			LogMVPlayerDodge,
			Warning,
			TEXT("Dodge naming fallback failed because table was not found. CharacterIndexCode=%s Table=%s."),
			*CharacterIndexCode.ToString(),
			*ActionTableName.ToString());
		return false;
	}

	OutActionRowHandle.ActionRow.DataTable = const_cast<UDataTable*>(ActionDataTable);
	OutActionRowHandle.ActionRow.RowName = MakeDodgeActionRowName(CharacterIndexCode, DefaultDodgeRowIndex);
	return OutActionRowHandle.IsValid();
}

bool UMVPlayerDodge::EvaluateDodgeChooserActionRowHandle(FMVDodgeActionRowHandle& OutActionRowHandle)
{
	OutActionRowHandle.Reset();
	ChooserDodgeActionRowHandle.Reset();

	if (!DodgeChooserTable.IsValid())
	{
		UE_LOG(LogMVPlayerDodge, Warning, TEXT("Dodge chooser path is invalid."));
		return false;
	}

	UChooserTable* ChooserTable = Cast<UChooserTable>(DodgeChooserTable.TryLoad());
	if (!ChooserTable)
	{
		UE_LOG(
			LogMVPlayerDodge,
			Warning,
			TEXT("Dodge chooser failed to load. Path=%s."),
			*DodgeChooserTable.ToString());
		return false;
	}

	FChooserEvaluationContext Context;
	if (UObject* OwnerObject = GetPlayerCharacter())
	{
		Context.AddObjectParam(OwnerObject);
	}
	else
	{
		return false;
	}
	Context.AddStructParam(ChooserDodgeActionRowHandle);

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

	if (ChooserDodgeActionRowHandle.IsValid())
	{
		OutActionRowHandle = ChooserDodgeActionRowHandle;
		return true;
	}

	UObject* ResolvedObject = SelectedObject.IsValid()
		? SelectedObject.Get()
		: SelectedObject.LoadSynchronous();
	UDataTable* SelectedDataTable = Cast<UDataTable>(ResolvedObject);
	if (!SelectedDataTable)
	{
		UE_LOG(
			LogMVPlayerDodge,
			Warning,
			TEXT("Dodge chooser did not return FMVDodgeActionRowHandle or UDataTable fallback. SelectedObject=%s."),
			*GetNameSafe(ResolvedObject));
		return false;
	}

	OutActionRowHandle.ActionRow.DataTable = SelectedDataTable;
	OutActionRowHandle.ActionRow.RowName = MakeDodgeActionRowName(
		ResolveCharacterIndexCode(),
		DefaultDodgeRowIndex);
	return OutActionRowHandle.IsValid();
}

FName UMVPlayerDodge::MakeDodgeActionTableName(const FGameplayTag CharacterIndexCode) const
{
	const FString CharacterIndexCodeToken = DodgeCharacterIndexCodeToTableToken(CharacterIndexCode);
	return CharacterIndexCodeToken.IsEmpty()
		? NAME_None
		: FName(*FString::Printf(TEXT("Dodge_%s"), *CharacterIndexCodeToken));
}

FName UMVPlayerDodge::MakeDodgeActionTableName(const UDataTable* ActionDataTable) const
{
	if (!ActionDataTable)
	{
		return NAME_None;
	}

	FString TableName = ActionDataTable->GetName();
	TableName.RemoveFromStart(TEXT("DT_"));
	return FName(*TableName);
}

FName UMVPlayerDodge::MakeDodgeActionRowName(
	const FGameplayTag CharacterIndexCode,
	const int32 Index) const
{
	const FString CharacterIndexCodeToken = DodgeCharacterIndexCodeToTableToken(CharacterIndexCode);
	return CharacterIndexCodeToken.IsEmpty()
		? NAME_None
		: FName(*FString::Printf(TEXT("Dodge_%s_%02d"), *CharacterIndexCodeToken, FMath::Max(1, Index)));
}

FGameplayTag UMVPlayerDodge::ResolveCharacterIndexCode() const
{
	const AMVCharacterBase* OwnerCharacter = GetPlayerCharacter();
	return OwnerCharacter ? OwnerCharacter->GetCharacterIndexCode() : FGameplayTag();
}

bool UMVPlayerDodge::CanTransitionActiveDodgeAction(const UMVInputManagerComponent& InputManager, const UMVActionComponent& ActionComponent) const
{
	if (!InputManager.IsRecoveryEscapeWindowOpen() || !ActionComponent.CanInterruptActiveAction())
	{
		return false;
	}

	const FName ActiveActionTableName = ActionComponent.GetActiveActionTableName();
	const FName ActiveActionRowName = ActionComponent.GetActiveActionRowName();
	const bool bActiveActionIsDodge = !ActiveDodgeActionTableName.IsNone()
		&& !ActiveDodgeActionRowName.IsNone()
		&& ActiveActionTableName == ActiveDodgeActionTableName
		&& ActiveActionRowName == ActiveDodgeActionRowName;
	return bActiveActionIsDodge
		|| MVDodgeIsHitReactionTerminalRecoveryAction(ActiveActionTableName, ActiveActionRowName);
}

const FMVDodgeActionRow* UMVPlayerDodge::FindDodgeActionRow(
	const FDataTableRowHandle ActionRowHandle) const
{
	if (!ActionRowHandle.DataTable || ActionRowHandle.RowName.IsNone())
	{
		UE_LOG(
			LogMVPlayerDodge,
			Warning,
			TEXT("FindDodgeActionRow failed because row handle is invalid. DataTable=%s Row=%s."),
			*GetNameSafe(ActionRowHandle.DataTable),
			*ActionRowHandle.RowName.ToString());
		return nullptr;
	}

	if (!ActionRowHandle.DataTable->GetRowStruct()
		|| !ActionRowHandle.DataTable->GetRowStruct()->IsChildOf(FMVDodgeActionRow::StaticStruct()))
	{
		UE_LOG(
			LogMVPlayerDodge,
			Warning,
			TEXT("FindDodgeActionRow failed because table row struct is invalid. DataTable=%s RowStruct=%s Expected=MVDodgeActionRow."),
			*GetNameSafe(ActionRowHandle.DataTable),
			ActionRowHandle.DataTable->GetRowStruct() ? *ActionRowHandle.DataTable->GetRowStruct()->GetName() : TEXT("None"));
		return nullptr;
	}

	const FMVDodgeActionRow* DodgeActionRow = ActionRowHandle.DataTable->FindRow<FMVDodgeActionRow>(
		ActionRowHandle.RowName,
		TEXT("MVPlayerDodge"),
		false);
	if (!DodgeActionRow)
	{
		UE_LOG(
			LogMVPlayerDodge,
			Warning,
			TEXT("FindDodgeActionRow failed because row was not found. DataTable=%s Row=%s AvailableRows=%s."),
			*GetNameSafe(ActionRowHandle.DataTable),
			*ActionRowHandle.RowName.ToString(),
			*MVDodgeBuildAvailableRowNameLog(*ActionRowHandle.DataTable));
		return nullptr;
	}

	if (!DodgeActionRow->bEnabled)
	{
		return nullptr;
	}

	return DodgeActionRow;
}

bool UMVPlayerDodge::CanConsumeDodgeCost(const FMVDodgeActionRow& DodgeActionRow) const
{
	const AMVCharacterBase* OwnerCharacter = GetPlayerCharacter();
	const UMVStatComponent* StatComponent = OwnerCharacter
		? OwnerCharacter->FindComponentByClass<UMVStatComponent>()
		: nullptr;
	if (!StatComponent)
	{
		return true;
	}

	const float InstantCost = DodgeActionRow.StaminaCostType == EMVActionResourceCostType::Instant
		? DodgeActionRow.StaminaCost
		: 0.0f;
	const float RequiredStamina = FMath::Max(
		FMath::Max(0.0f, DodgeActionRow.MinRequiredStamina),
		FMath::Max(0.0f, InstantCost));
	return StatComponent->HasStamina(RequiredStamina);
}

bool UMVPlayerDodge::ConsumeDodgeCost(const FMVDodgeActionRow& DodgeActionRow)
{
	AMVCharacterBase* OwnerCharacter = GetPlayerCharacter();
	UMVStatComponent* StatComponent = OwnerCharacter
		? OwnerCharacter->FindComponentByClass<UMVStatComponent>()
		: nullptr;
	if (!StatComponent || DodgeActionRow.StaminaCost <= 0.0f)
	{
		return true;
	}

	switch (DodgeActionRow.StaminaCostType)
	{
	case EMVActionResourceCostType::Instant:
		return StatComponent->ConsumeStamina(DodgeActionRow.StaminaCost);
	case EMVActionResourceCostType::None:
	case EMVActionResourceCostType::PerSecond:
	case EMVActionResourceCostType::OnDemand:
	default:
		return true;
	}
}

void UMVPlayerDodge::ApplyDodgeChooserSnapshot(
	AMVCharacterBase& OwnerCharacter,
	const bool bHasMovementInput,
	const ELocomotionDirection InputDirection,
	const FVector& MovementDirection) const
{
	OwnerCharacter.bHasDodgeMovementInput = bHasMovementInput;

	if (!bHasMovementInput)
	{
		OwnerCharacter.ApplyLocomotionDirectionSnapshot(-OwnerCharacter.GetActorForwardVector());
		OwnerCharacter.LocomotionDirection = ELocomotionDirection::B;
		OwnerCharacter.LocomotionDirectionFromAcceleration = ELocomotionDirection::B;
		return;
	}

	if (!MovementDirection.IsNearlyZero())
	{
		OwnerCharacter.ApplyLocomotionDirectionSnapshot(MovementDirection);
	}

	OwnerCharacter.LocomotionDirection = InputDirection;
	OwnerCharacter.LocomotionDirectionFromAcceleration = InputDirection;
}

void UMVPlayerDodge::HandleActionInputSubmitted(
	const int32 ActionId,
	const FVector2D ControllerSpaceInput,
	const bool bHasMovementInput)
{
	if (ActionId != MVActionIds::Dodge)
	{
		return;
	}

	if (bHasMovementInput)
	{
		CacheControllerSpaceMovementInput(ControllerSpaceInput);
	}

	if (TryStartDodgeAction())
	{
		if (const AMVCharacterBase* OwnerCharacter = GetPlayerCharacter())
		{
			if (UMVInputManagerComponent* InputManager = OwnerCharacter->InputManagerComponent)
			{
				InputManager->ClearBufferedActionInput();
			}
		}
	}
}

void UMVPlayerDodge::HandleActionEnded(
	const FName ActionTableName,
	const FName ActionRowName,
	const bool)
{
	const bool bEndedActiveDodgeAction = !ActiveDodgeActionTableName.IsNone()
		&& !ActiveDodgeActionRowName.IsNone()
		&& ActionTableName == ActiveDodgeActionTableName
		&& ActionRowName == ActiveDodgeActionRowName;
	if (!bEndedActiveDodgeAction)
	{
		return;
	}

	ActiveDodgeActionTableName = NAME_None;
	ActiveDodgeActionRowName = NAME_None;
	EndLockOnPawnRotationSuppressionForDodge();
}

void UMVPlayerDodge::HandleRecoveryEscapeWindowChanged(const bool bOpen)
{
	if (bOpen)
	{
		TryConsumeBufferedDodgeInput();
	}
}
