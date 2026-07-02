#include "Components/MVInputManagerComponent.h"

#include "Character/MVCharacterBase.h"
#include "Components/MVStatComponent.h"

UMVInputManagerComponent::UMVInputManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UMVInputManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	CacheOwnerReferences();
	if (OwnerCharacter)
	{
		OwnerCharacter->OnMovementInputReceived.AddUObject(
			this,
			&UMVInputManagerComponent::HandleOwnerMovementInput);
	}
}

bool UMVInputManagerComponent::SubmitActionInput(const EMVActionId ActionId)
{
	return SubmitActionInputById(MVActionIds::ToRawActionId(ActionId));
}

bool UMVInputManagerComponent::SubmitActionInputById(const int32 ActionId)
{
	if (ActionId <= MVActionIds::None)
	{
		return false;
	}

	if (const AMVCharacterBase* Character = OwnerCharacter.Get())
	{
		if (Character->StatComponent && Character->StatComponent->IsDead())
		{
			ClearBufferedActionInput();
			return false;
		}
	}

	FVector2D ControllerSpaceInput = FVector2D::ZeroVector;
	bool bHasCurrentMovementInput = false;
	if (const AMVCharacterBase* Character = OwnerCharacter.Get())
	{
		bHasCurrentMovementInput = Character->TryGetControllerSpaceMovementInput(ControllerSpaceInput, 0);
	}

	const uint64 CurrentFrame = GFrameCounter;
	if (!bHasCurrentMovementInput
		&& CachedActionControllerSpaceInputFrame == CurrentFrame
		&& !CachedActionControllerSpaceInput.IsNearlyZero())
	{
		ControllerSpaceInput = CachedActionControllerSpaceInput;
		bHasCurrentMovementInput = true;
	}

	const bool bHasMovementInput = bHasCurrentMovementInput
		&& ResolveActionInputDirection(ControllerSpaceInput) != EMVActionInputDirection::None;
	BufferedActionId = ActionId;
	BufferedActionControllerSpaceInput = ControllerSpaceInput;
	BufferedActionInputFrame = GFrameCounter;
	bBufferedActionHasMovementInput = bHasMovementInput;
	OnActionInputSubmitted.Broadcast(ActionId, ControllerSpaceInput, bHasMovementInput);
	return true;
}

void UMVInputManagerComponent::UpdateActionMovementInput(const FVector WorldMovementInput)
{
	const FVector2D ControllerSpaceInput = ResolveControllerSpaceInputFromWorldDirection(WorldMovementInput);
	if (ControllerSpaceInput.SizeSquared() < FMath::Square(FMath::Max(0.0f, ActionInputDeadZone)))
	{
		return;
	}

	if (CachedActionControllerSpaceInputFrame == GFrameCounter)
	{
		CachedActionControllerSpaceInput = ClampActionControllerSpaceInput(
			CachedActionControllerSpaceInput + ControllerSpaceInput);
	}
	else
	{
		CachedActionControllerSpaceInput = ControllerSpaceInput;
	}
	CachedActionControllerSpaceInputFrame = GFrameCounter;
	OnActionMovementInputUpdated.Broadcast(CachedActionControllerSpaceInput, true);
}

bool UMVInputManagerComponent::TryGetBufferedActionInput(
	int32& OutActionId,
	FVector2D& OutControllerSpaceInput,
	bool& bOutHasMovementInput) const
{
	OutActionId = INDEX_NONE;
	OutControllerSpaceInput = FVector2D::ZeroVector;
	bOutHasMovementInput = false;

	if (BufferedActionId <= MVActionIds::None)
	{
		return false;
	}

	const uint64 CurrentFrame = GFrameCounter;
	if (BufferedActionInputFrame > CurrentFrame)
	{
		return false;
	}

	// 입력 후 너무 오래 지난 버퍼는 현재 조작 의도로 보기 어려워 recovery window에서 소비하지 않는다.
	const uint64 FrameAge = CurrentFrame - BufferedActionInputFrame;
	if (FrameAge > static_cast<uint64>(FMath::Max(0, ActionInputBufferMaxFrameAge)))
	{
		return false;
	}

	OutActionId = BufferedActionId;
	OutControllerSpaceInput = BufferedActionControllerSpaceInput;
	bOutHasMovementInput = bBufferedActionHasMovementInput;
	return true;
}

void UMVInputManagerComponent::ClearBufferedActionInput()
{
	BufferedActionId = INDEX_NONE;
	BufferedActionControllerSpaceInput = FVector2D::ZeroVector;
	BufferedActionInputFrame = 0;
	bBufferedActionHasMovementInput = false;
}

bool UMVInputManagerComponent::TryGetRecentActionMovementInput(FVector2D& OutControllerSpaceInput) const
{
	OutControllerSpaceInput = FVector2D::ZeroVector;

	const uint64 CurrentFrame = GFrameCounter;
	if (CachedActionControllerSpaceInputFrame <= CurrentFrame)
	{
		// 오래된 방향 입력은 사용자가 지금 의도한 Escape 방향이 아닐 수 있으므로 무시한다.
		const uint64 FrameAge = CurrentFrame - CachedActionControllerSpaceInputFrame;
		if (FrameAge <= static_cast<uint64>(FMath::Max(0, ActionInputMaxFrameAge))
			&& !CachedActionControllerSpaceInput.IsNearlyZero())
		{
			OutControllerSpaceInput = CachedActionControllerSpaceInput;
			return true;
		}
	}

	const AMVCharacterBase* Character = OwnerCharacter.Get();
	if (!Character)
	{
		Character = Cast<AMVCharacterBase>(GetOwner());
	}

	return Character
		&& Character->TryGetControllerSpaceMovementInput(
			OutControllerSpaceInput,
			FMath::Max(0, ActionInputMaxFrameAge));
}

EMVActionInputDirection UMVInputManagerComponent::ResolveActionInputDirection(
	const FVector2D ControllerSpaceInput) const
{
	const FVector2D ClampedInput = ClampActionControllerSpaceInput(ControllerSpaceInput);
	if (ClampedInput.SizeSquared() < FMath::Square(FMath::Max(0.0f, ActionInputDeadZone)))
	{
		return EMVActionInputDirection::None;
	}

	if (FMath::Abs(ClampedInput.X) >= FMath::Abs(ClampedInput.Y))
	{
		return ClampedInput.X >= 0.0f
			? EMVActionInputDirection::Forward
			: EMVActionInputDirection::Back;
	}

	return ClampedInput.Y >= 0.0f
		? EMVActionInputDirection::Right
		: EMVActionInputDirection::Left;
}

void UMVInputManagerComponent::BeginMovementInputBlock()
{
	++MovementInputBlockCount;
}

void UMVInputManagerComponent::EndMovementInputBlock()
{
	const bool bWasBlocked = MovementInputBlockCount > 0;
	MovementInputBlockCount = FMath::Max(0, MovementInputBlockCount - 1);
}

bool UMVInputManagerComponent::IsMovementInputBlocked() const
{
	return MovementInputBlockCount > 0;
}

void UMVInputManagerComponent::BeginRecoveryEscapeWindow()
{
	const bool bWasOpen = IsRecoveryEscapeWindowOpen();
	++RecoveryEscapeWindowCount;
	if (!bWasOpen)
	{
		OnRecoveryEscapeWindowChanged.Broadcast(true);
	}
}

void UMVInputManagerComponent::EndRecoveryEscapeWindow()
{
	const bool bWasOpen = IsRecoveryEscapeWindowOpen();
	if (RecoveryEscapeWindowCount <= 0)
	{
		RecoveryEscapeWindowCount = 0;
		return;
	}

	RecoveryEscapeWindowCount = FMath::Max(0, RecoveryEscapeWindowCount - 1);
	if (bWasOpen && !IsRecoveryEscapeWindowOpen())
	{
		OnRecoveryEscapeWindowChanged.Broadcast(false);
	}
}

bool UMVInputManagerComponent::IsRecoveryEscapeWindowOpen() const
{
	return RecoveryEscapeWindowCount > 0;
}

void UMVInputManagerComponent::ResetNotifyState()
{
	const bool bWasRecoveryEscapeWindowOpen = IsRecoveryEscapeWindowOpen();
	MovementInputBlockCount = 0;
	RecoveryEscapeWindowCount = 0;
	if (bWasRecoveryEscapeWindowOpen)
	{
		OnRecoveryEscapeWindowChanged.Broadcast(false);
	}
}

void UMVInputManagerComponent::CacheOwnerReferences()
{
	OwnerCharacter = Cast<AMVCharacterBase>(GetOwner());
}

void UMVInputManagerComponent::HandleOwnerMovementInput(const FVector& MovementInputDirection)
{
	UpdateActionMovementInput(MovementInputDirection);
}

FVector2D UMVInputManagerComponent::ResolveControllerSpaceInputFromWorldDirection(
	const FVector& WorldMovementInput) const
{
	const AMVCharacterBase* Character = OwnerCharacter.Get();
	if (!Character)
	{
		Character = Cast<AMVCharacterBase>(GetOwner());
	}

	const FVector WorldInput2D(WorldMovementInput.X, WorldMovementInput.Y, 0.0f);
	if (!Character || WorldInput2D.IsNearlyZero())
	{
		return FVector2D::ZeroVector;
	}

	const FRotator ReferenceRotation = Character->ResolveMovementInputReferenceRotation();
	const FVector ForwardVector = FRotationMatrix(ReferenceRotation).GetUnitAxis(EAxis::X).GetSafeNormal2D();
	const FVector RightVector = FRotationMatrix(ReferenceRotation).GetUnitAxis(EAxis::Y).GetSafeNormal2D();
	const FVector NormalizedWorldInput = WorldInput2D.GetSafeNormal2D();
	return ClampActionControllerSpaceInput(FVector2D(
		FVector::DotProduct(NormalizedWorldInput, ForwardVector),
		FVector::DotProduct(NormalizedWorldInput, RightVector)));
}

FVector2D UMVInputManagerComponent::ClampActionControllerSpaceInput(FVector2D Input)
{
	const float SizeSquared = Input.SizeSquared();
	if (SizeSquared <= 1.0f)
	{
		return Input;
	}

	return Input / FMath::Sqrt(SizeSquared);
}
