#pragma once

#include "Engine/EngineTypes.h"

/**
 * 프로젝트 전용 충돌 채널 별칭.
 *
 * 책임:
 *   - Config/DefaultEngine.ini에 선언된 커스텀 채널과 C++ 사용처를 한 이름으로 연결한다.
 *   - 어빌리티, 상호작용, 향후 파괴 가능 오브젝트가 같은 채널 슬롯을 공유하게 한다.
 */
namespace MVCollisionChannels
{
	constexpr ECollisionChannel AttackTarget = ECC_GameTraceChannel1;
}
