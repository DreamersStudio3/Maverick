#include "Components/MVInputManagerComponent.h"

#include "Character/MVCharacterBase.h"
#include "Components/MVStatComponent.h"
#include "Interface/MVActionInputHandlerInterface.h"

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

bool UMVInputManagerComponent::SubmitActionInput(const FGameplayTag ActionInputTag)
{
	if (!ActionInputTag.IsValid())
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
	BufferedActionInputTag = ActionInputTag;
	BufferedActionControllerSpaceInput = ControllerSpaceInput;
	BufferedActionInputFrame = GFrameCounter;
	bBufferedActionHasMovementInput = bHasMovementInput;
	const bool bHandled = TryRouteActionInput(ActionInputTag, ControllerSpaceInput, bHasMovementInput);
	if (bHandled)
	{
		ClearBufferedActionInput();
	}
	OnActionInputSubmitted.Broadcast(ActionInputTag, ControllerSpaceInput, bHasMovementInput);
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
	FGameplayTag& OutActionInputTag,
	FVector2D& OutControllerSpaceInput,
	bool& bOutHasMovementInput) const
{
	OutActionInputTag = FGameplayTag();
	OutControllerSpaceInput = FVector2D::ZeroVector;
	bOutHasMovementInput = false;

	if (!BufferedActionInputTag.IsValid())
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

	OutActionInputTag = BufferedActionInputTag;
	OutControllerSpaceInput = BufferedActionControllerSpaceInput;
	bOutHasMovementInput = bBufferedActionHasMovementInput;
	return true;
}

void UMVInputManagerComponent::ClearBufferedActionInput()
{
	BufferedActionInputTag = FGameplayTag();
	BufferedActionControllerSpaceInput = FVector2D::ZeroVector;
	BufferedActionInputFrame = 0;
	bBufferedActionHasMovementInput = false;
}

void UMVInputManagerComponent::RegisterActionInputHandler(UObject* HandlerObject, const int32 Priority)
{
	if (!HandlerObject || !Cast<IMVActionInputHandlerInterface>(HandlerObject))
	{
		return;
	}

	ActionInputHandlers.RemoveAll(
		[HandlerObject](const FMVActionInputHandlerEntry& Entry)
		{
			return !Entry.HandlerObject.IsValid() || Entry.HandlerObject.Get() == HandlerObject;
		});

	FMVActionInputHandlerEntry Entry;
	Entry.HandlerObject = HandlerObject;
	Entry.Priority = Priority;
	Entry.RegistrationOrder = NextActionInputHandlerOrder++;
	ActionInputHandlers.Add(Entry);
	SortActionInputHandlers();
}

void UMVInputManagerComponent::UnregisterActionInputHandler(const UObject* HandlerObject)
{
	if (!HandlerObject)
	{
		return;
	}

	ActionInputHandlers.RemoveAll(
		[HandlerObject](const FMVActionInputHandlerEntry& Entry)
		{
			return !Entry.HandlerObject.IsValid() || Entry.HandlerObject.Get() == HandlerObject;
		});
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
		if (IsRecoveryEscapeWindowOpen() && !TryRouteBufferedActionInput())
		{
			TryRouteRecoveryWindowOpened();
		}
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

bool UMVInputManagerComponent::TryRouteActionInput(
	const FGameplayTag ActionInputTag,
	const FVector2D ControllerSpaceInput,
	const bool bHasMovementInput)
{
	if (!ActionInputTag.IsValid())
	{
		return false;
	}

	CompactActionInputHandlers();
	for (const FMVActionInputHandlerEntry& Entry : ActionInputHandlers)
	{
		UObject* HandlerObject = Entry.HandlerObject.Get();
		IMVActionInputHandlerInterface* Handler = Cast<IMVActionInputHandlerInterface>(HandlerObject);
		if (!Handler)
		{
			continue;
		}

		if (Handler->TryHandleActionInput(ActionInputTag, ControllerSpaceInput, bHasMovementInput))
		{
			return true;
		}
	}

	return false;
}

bool UMVInputManagerComponent::TryRouteBufferedActionInput()
{
	FGameplayTag ActionInputTag;
	FVector2D ControllerSpaceInput = FVector2D::ZeroVector;
	bool bHasMovementInput = false;
	if (!TryGetBufferedActionInput(ActionInputTag, ControllerSpaceInput, bHasMovementInput))
	{
		return false;
	}

	if (!TryRouteActionInput(ActionInputTag, ControllerSpaceInput, bHasMovementInput))
	{
		return false;
	}

	ClearBufferedActionInput();
	return true;
}

bool UMVInputManagerComponent::TryRouteRecoveryWindowOpened()
{
	CompactActionInputHandlers();
	for (const FMVActionInputHandlerEntry& Entry : ActionInputHandlers)
	{
		UObject* HandlerObject = Entry.HandlerObject.Get();
		IMVActionInputHandlerInterface* Handler = Cast<IMVActionInputHandlerInterface>(HandlerObject);
		if (!Handler)
		{
			continue;
		}

		if (Handler->TryHandleRecoveryWindowOpened())
		{
			return true;
		}
	}

	return false;
}

void UMVInputManagerComponent::SortActionInputHandlers()
{
	ActionInputHandlers.Sort(
		[](const FMVActionInputHandlerEntry& Left, const FMVActionInputHandlerEntry& Right)
		{
			if (Left.Priority != Right.Priority)
			{
				return Left.Priority > Right.Priority;
			}

			return Left.RegistrationOrder < Right.RegistrationOrder;
		});
}

void UMVInputManagerComponent::CompactActionInputHandlers()
{
	ActionInputHandlers.RemoveAll(
		[](const FMVActionInputHandlerEntry& Entry)
		{
			return !Entry.HandlerObject.IsValid()
				|| !Cast<IMVActionInputHandlerInterface>(Entry.HandlerObject.Get());
		});
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
