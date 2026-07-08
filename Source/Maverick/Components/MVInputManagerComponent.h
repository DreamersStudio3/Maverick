#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Enum/MVActionInputPhase.h"
#include "GameplayTagContainer.h"
#include "MVInputManagerComponent.generated.h"

class AMVCharacterBase;

UENUM(BlueprintType)
enum class EMVActionInputDirection : uint8
{
	None,
	Forward,
	Left,
	Right,
	Back
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FMVOnActionInputSubmitted,
	FGameplayTag,
	ActionInputTag,
	FVector2D,
	ControllerSpaceInput,
	bool,
	bHasMovementInput);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FMVOnActionMovementInputUpdated,
	FVector2D,
	ControllerSpaceInput,
	bool,
	bHasMovementInput);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMVOnActionRecoveryEscapeWindowChanged, bool, bOpen);


/**
 * 캐릭터 공용 액션 입력 관리자.
 *
 * 플레이어 입력을 짧은 액션 입력 버퍼로 정규화하고 등록된 도메인 handler로 중앙 라우팅한다.
 * 이동 입력은 controller-space로 저장해 Dodge, Combat, HitReaction 같은 도메인 컴포넌트가 같은 기준의 입력 문맥을 사용할 수 있게 한다.
 * 이 컴포넌트는 ActionInputTag별 도메인 규칙을 직접 알지 않고, 우선순위가 높은 handler부터 처리 기회를 넘긴다.
 *
 * 라이프사이클:
 *   1) BeginPlay -> OwnerCharacter를 캐시하고 이동 입력 이벤트를 구독한다.
 *   2) 이동 입력 -> 최근 controller-space 입력을 저장하고 브로드캐스트한다.
 *   3) SubmitActionInput/SubmitHoldActionInput -> 입력 태그와 phase, 이동 입력 스냅샷을 버퍼에 저장하고 handler 라우팅을 시도한다.
 *   4) RecoveryEscapeWindow 입력 소비 -> 버퍼 입력, handler별 recovery 처리, active action cancel trigger를 같은 흐름에서 평가한다.
 */
UCLASS(ClassGroup = (Maverick), meta = (BlueprintSpawnableComponent))
class MAVERICK_API UMVInputManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMVInputManagerComponent();

protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable, Category = "Maverick|Input|Action")
	bool SubmitActionInput(UPARAM(meta = (Categories = "Action.Input")) FGameplayTag ActionInputTag);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Input|Action")
	bool SubmitHoldActionInput(
		UPARAM(meta = (Categories = "Action.Input")) FGameplayTag ActionInputTag,
		EMVActionInputPhase Phase,
		float HeldSeconds);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Input|Action")
	void UpdateActionMovementInput(FVector WorldMovementInput);

	UFUNCTION(BlueprintPure, Category = "Maverick|Input|Action")
	bool TryGetBufferedActionInput(
		FGameplayTag& OutActionInputTag,
		FVector2D& OutControllerSpaceInput,
		bool& bOutHasMovementInput) const;

	UFUNCTION(BlueprintCallable, Category = "Maverick|Input|Action")
	void ClearBufferedActionInput();

	void RegisterActionInputHandler(UObject* HandlerObject, int32 Priority);
	void UnregisterActionInputHandler(const UObject* HandlerObject);

	UFUNCTION(BlueprintPure, Category = "Maverick|Input|Action")
	bool TryGetRecentActionMovementInput(FVector2D& OutControllerSpaceInput) const;

	UFUNCTION(BlueprintPure, Category = "Maverick|Input|Action")
	EMVActionInputDirection ResolveActionInputDirection(FVector2D ControllerSpaceInput) const;

	// Moved From ActionComponent to InputManagerComponent
	UFUNCTION(BlueprintCallable, Category = "Maverick|Action|NotifyState")
	void BeginMovementInputBlock();

	UFUNCTION(BlueprintCallable, Category = "Maverick|Action|NotifyState")
	void EndMovementInputBlock();

	UFUNCTION(BlueprintPure, Category = "Maverick|Action|NotifyState")
	bool IsMovementInputBlocked() const;

	UFUNCTION(BlueprintCallable, Category = "Maverick|Action|NotifyState")
	void BeginRecoveryEscapeWindow();

	UFUNCTION(BlueprintCallable, Category = "Maverick|Action|NotifyState")
	void EndRecoveryEscapeWindow();

	UFUNCTION(BlueprintPure, Category = "Maverick|Action|NotifyState")
	bool IsRecoveryEscapeWindowOpen() const;

	UFUNCTION(BlueprintCallable, Category = "Maverick|Action|NotifyState")
	void ResetNotifyState();

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Action|NotifyState")
	FMVOnActionRecoveryEscapeWindowChanged OnRecoveryEscapeWindowChanged;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Input|Action", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ActionInputDeadZone = 0.25f;

	// Recovery window가 열리기 직전에 들어온 이동 입력을 탈출 의도로 쓸 수 있도록 짧은 rolling window를 둔다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Input|Action", meta = (ClampMin = "0"))
	int32 ActionInputMaxFrameAge = 30;

	// Recovery window가 열리기 전에 들어온 Dodge 같은 액션 입력을 보관하되, 지나치게 오래된 입력은 버린다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Input|Action", meta = (ClampMin = "0"))
	int32 ActionInputBufferMaxFrameAge = 60;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Input|Recovery")
	bool bAllowMovementInputRecoveryCancel = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Input|Recovery", meta = (ClampMin = "0.0", Units = "s"))
	float RecoveryLocomotionCancelBlendOutTime = 0.2f;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Input|Action")
	FMVOnActionInputSubmitted OnActionInputSubmitted;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Input|Action")
	FMVOnActionMovementInputUpdated OnActionMovementInputUpdated;

private:
	struct FMVActionInputHandlerEntry
	{
		TWeakObjectPtr<UObject> HandlerObject;
		int32 Priority = 0;
		uint64 RegistrationOrder = 0;
	};

	enum class EMVRecoveryEscapeCancelTrigger : uint8
	{
		MovementInput
	};

	enum class EMVBufferedActionInputKind : uint8
	{
		None,
		Instant,
		Hold
	};

	struct FMVBufferedActionInput
	{
		EMVBufferedActionInputKind Kind = EMVBufferedActionInputKind::None;
		FGameplayTag ActionInputTag;
		FVector2D ControllerSpaceInput = FVector2D::ZeroVector;
		uint64 Frame = 0;
		bool bHasMovementInput = false;
		bool bHoldReleased = false;
		bool bHoldCanceled = false;

		void Reset()
		{
			Kind = EMVBufferedActionInputKind::None;
			ActionInputTag = FGameplayTag();
			ControllerSpaceInput = FVector2D::ZeroVector;
			Frame = 0;
			bHasMovementInput = false;
			bHoldReleased = false;
			bHoldCanceled = false;
		}
	};

	void CacheOwnerReferences();
	void HandleOwnerMovementInput(const FVector& MovementInputDirection);
	FVector2D ResolveControllerSpaceInputFromWorldDirection(const FVector& WorldMovementInput) const;
	void BuildActionInputSnapshot(FVector2D& OutControllerSpaceInput, bool& bOutHasMovementInput) const;
	void BufferInstantActionInput(FGameplayTag ActionInputTag, FVector2D ControllerSpaceInput, bool bHasMovementInput);
	bool TryRouteActionInput(FGameplayTag ActionInputTag, FVector2D ControllerSpaceInput, bool bHasMovementInput);
	bool TryRouteHoldActionInput(
		FGameplayTag ActionInputTag,
		EMVActionInputPhase Phase,
		float HeldSeconds,
		FVector2D ControllerSpaceInput,
		bool bHasMovementInput);
	void BufferHoldActionInput(
		FGameplayTag ActionInputTag,
		EMVActionInputPhase Phase,
		float HeldSeconds,
		FVector2D ControllerSpaceInput,
		bool bHasMovementInput);
	bool TryRouteBufferedActionInput();
	bool TryRouteBufferedHoldActionInput();
	bool TryRouteRecoveryWindowOpened();
	bool TryConsumeRecoveryEscapeInput();
	bool TryCancelActiveActionForRecoveryEscape(EMVRecoveryEscapeCancelTrigger CancelTrigger);
	bool TryCancelActiveActionToLocomotionForRecoveryEscape();
	bool CanUseRecoveryEscapeCancelTrigger(EMVRecoveryEscapeCancelTrigger CancelTrigger) const;
	bool HasRecentRecoveryMovementInput() const;
	void SortActionInputHandlers();
	void CompactActionInputHandlers();
	static FVector2D ClampActionControllerSpaceInput(FVector2D Input);

	UPROPERTY(Transient)
	TObjectPtr<AMVCharacterBase> OwnerCharacter;

	FMVBufferedActionInput BufferedActionInput;

	FVector2D CachedActionControllerSpaceInput = FVector2D::ZeroVector;
	uint64 CachedActionControllerSpaceInputFrame = 0;
	TArray<FMVActionInputHandlerEntry> ActionInputHandlers;
	uint64 NextActionInputHandlerOrder = 0;

private:
	int32 MovementInputBlockCount = 0;
	int32 RecoveryEscapeWindowCount = 0;
};
