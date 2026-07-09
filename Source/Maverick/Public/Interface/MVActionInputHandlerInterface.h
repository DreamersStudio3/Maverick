#pragma once

#include "CoreMinimal.h"
#include "Enum/MVActionInputPhase.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "MVActionInputHandlerInterface.generated.h"

namespace MVActionInputHandlerPriorities
{
	inline constexpr int32 HitReaction = 300;
	inline constexpr int32 Dodge = 200;
	inline constexpr int32 Consumable = 150;
	inline constexpr int32 Combat = 100;
}

UINTERFACE(MinimalAPI)
class UMVActionInputHandlerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * InputManager가 액션 입력을 중앙 라우팅할 때 호출하는 C++ 전용 handler 인터페이스.
 *
 * 도메인 컴포넌트는 자신이 처리할 ActionInputTag만 판단하고, 입력 버퍼 저장/재시도/소비 순서는
 * UMVInputManagerComponent가 우선순위에 따라 한 곳에서 관리한다.
 */
class MAVERICK_API IMVActionInputHandlerInterface
{
	GENERATED_BODY()

public:
	virtual bool TryHandleActionInput(
		FGameplayTag ActionInputTag,
		FVector2D ControllerSpaceInput,
		bool bHasMovementInput)
	{
		return false;
	}

	virtual bool TryHandleHoldActionInput(
		FGameplayTag ActionInputTag,
		EMVActionInputPhase Phase,
		float HeldSeconds,
		FVector2D ControllerSpaceInput,
		bool bHasMovementInput)
	{
		return false;
	}

	virtual bool TryHandleRecoveryWindowOpened()
	{
		return false;
	}
};
