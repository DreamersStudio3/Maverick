#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Tables/MVActionTableTypes.h"
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
	int32,
	ActionId,
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

/**
 * 캐릭터 공용 액션 입력 관리자.
 *
 * 플레이어 입력을 도메인 컴포넌트가 구독할 수 있는 입력 의도와 짧은 액션 입력 버퍼로 정규화한다.
 * 이동 입력은 controller-space로 저장해 Dodge, Combat, HitReaction 같은 도메인 컴포넌트가 같은 기준의 입력 문맥을 사용할 수 있게 한다.
 * 이 컴포넌트는 어떤 액션을 실행할지, 리커버리 window에서 어떤 섹션으로 이동할지 결정하지 않는다.
 *
 * 라이프사이클:
 *   1) BeginPlay -> OwnerCharacter를 캐시하고 이동 입력 이벤트를 구독한다.
 *   2) 이동 입력 -> 최근 controller-space 입력을 저장하고 브로드캐스트한다.
 *   3) SubmitActionInput -> ActionId, 이동 입력 스냅샷, 이동 입력 존재 여부를 버퍼에 저장하고 브로드캐스트한다.
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
	bool SubmitActionInput(EMVActionId ActionId);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Input|Action")
	bool SubmitActionInputById(int32 ActionId);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Input|Action")
	void UpdateActionMovementInput(FVector WorldMovementInput);

	UFUNCTION(BlueprintPure, Category = "Maverick|Input|Action")
	bool TryGetBufferedActionInput(
		int32& OutActionId,
		FVector2D& OutControllerSpaceInput,
		bool& bOutHasMovementInput) const;

	UFUNCTION(BlueprintCallable, Category = "Maverick|Input|Action")
	void ClearBufferedActionInput();

	UFUNCTION(BlueprintPure, Category = "Maverick|Input|Action")
	bool TryGetRecentActionMovementInput(FVector2D& OutControllerSpaceInput) const;

	UFUNCTION(BlueprintPure, Category = "Maverick|Input|Action")
	EMVActionInputDirection ResolveActionInputDirection(FVector2D ControllerSpaceInput) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Input|Action", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ActionInputDeadZone = 0.25f;

	// Recovery window가 열리기 직전에 들어온 이동 입력을 탈출 의도로 쓸 수 있도록 짧은 rolling window를 둔다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Input|Action", meta = (ClampMin = "0"))
	int32 ActionInputMaxFrameAge = 30;

	// Recovery window가 열리기 전에 들어온 Dodge 같은 액션 입력을 보관하되, 지나치게 오래된 입력은 버린다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Input|Action", meta = (ClampMin = "0"))
	int32 ActionInputBufferMaxFrameAge = 60;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Input|Action")
	FMVOnActionInputSubmitted OnActionInputSubmitted;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Input|Action")
	FMVOnActionMovementInputUpdated OnActionMovementInputUpdated;

private:
	void CacheOwnerReferences();
	void HandleOwnerMovementInput(const FVector& MovementInputDirection);
	FVector2D ResolveControllerSpaceInputFromWorldDirection(const FVector& WorldMovementInput) const;
	static FVector2D ClampActionControllerSpaceInput(FVector2D Input);

	UPROPERTY(Transient)
	TObjectPtr<AMVCharacterBase> OwnerCharacter;

	int32 BufferedActionId = INDEX_NONE;
	FVector2D BufferedActionControllerSpaceInput = FVector2D::ZeroVector;
	uint64 BufferedActionInputFrame = 0;
	bool bBufferedActionHasMovementInput = false;

	FVector2D CachedActionControllerSpaceInput = FVector2D::ZeroVector;
	uint64 CachedActionControllerSpaceInputFrame = 0;
};
