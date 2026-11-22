#pragma once

#include "CoreMinimal.h"
#include "Combat/Projectile/MVProjectileAbility.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MVWeaponSkillSupport.generated.h"

/** 부채꼴 BP가 보관하는 활성 구간. 시전 자세와 기적중 대상을 구간 종료까지 보관한다. */
UCLASS(BlueprintType)
class MAVERICK_API UMVWeaponSkillConeWindow : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Maverick|Weapon Skill")
	static UMVWeaponSkillConeWindow* OpenWeaponSkillCone(UMVAbilityBase* Ability, float Radius,
		UMVWeaponSkillConeWindow* ExistingWindow, float AirborneStartDistance = 700.0f);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Weapon Skill")
	static void CloseWeaponSkillCone(UMVWeaponSkillConeWindow* Window);

private:
	virtual void BeginDestroy() override;
	void CheckCone();
	void CloseCone();

	TWeakObjectPtr<UMVAbilityBase> SourceAbility;
	TWeakObjectPtr<UWorld> WindowWorld;
	TSet<TWeakObjectPtr<AMVCharacterBase>> HitCharacters;
	FTimerHandle CheckTimer;
	FVector Origin = FVector::ZeroVector;
	FVector Forward = FVector::ZeroVector;
	float HalfHeight = 0.0f;
	float Range = 0.0f;
	double AirborneRange = 0.0;
	int32 AttackId = INDEX_NONE;
};

/** 스킬 BP의 발사 위치와 지연 이동 연결. 예약은 월드 타이머, 이동은 대상 CharacterMovement가 소유한다. */
UCLASS()
class MAVERICK_API UMVWeaponSkillSupport : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static FMVHitResolveRequest MakeAreaHitRequest(const UMVAbilityBase& Ability, AMVCharacterBase& Owner);

	UFUNCTION(BlueprintPure, Category = "Maverick|Weapon Skill")
	static FMVProjectileSpawnInfo ResolveGroundPullSpawnInfo(UMVProjectileAbility* Ability);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Weapon Skill")
	static void ScheduleGroundPull(AMVProjectileBase* Projectile, const FMVResolvedHitData& HitData,
		float ForwardDistance, float Delay, float Duration);
};
