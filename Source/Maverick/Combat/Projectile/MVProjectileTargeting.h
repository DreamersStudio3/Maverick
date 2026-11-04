#pragma once

#include "CoreMinimal.h"
#include "Combat/Projectile/MVProjectileTypes.h"

class AMVCharacterBase;
class USphereComponent;

namespace MVProjectileTargeting
{
	// 비행 중에도 유효한 적대 대상인지 확인한다. 일시적인 무적은 포함하지 않는다.
	MAVERICK_API bool IsLivingHostile(
		const AMVCharacterBase* Shooter,
		EMVProjectileTeam Team,
		const AMVCharacterBase* Target);

	// 새 유도 대상으로 선택할 수 있는지 확인한다.
	MAVERICK_API bool CanAcquire(
		const AMVCharacterBase* Shooter,
		EMVProjectileTeam Team,
		const AMVCharacterBase* Target);

	// 발사 시 한 번 호출한다. 대상이 없으면 nullptr를 반환한다.
	MAVERICK_API AActor* SelectTarget(
		AMVCharacterBase* Shooter,
		EMVProjectileTeam Team,
		AActor* ExplicitTarget,
		AActor* CurrentTarget,
		const FVector& LaunchLocation,
		const USphereComponent* ProjectileCollision,
		const FMVProjectileTargetingSettings& Settings);
}