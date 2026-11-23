#pragma once

#include "CoreMinimal.h"
#include "Combat/Projectile/MVProjectileAbility.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MVWeaponSkillSupport.generated.h"

/** 원형·부채꼴 판정 구간. 시작 자세를 고정하고 구간 종료까지 대상별 적중 기록과 반복 타이머를 소유한다. */
UCLASS()
class MAVERICK_API UMVWeaponSkillAreaWindow : public UObject
{
	GENERATED_BODY()

public:
	enum class EShape { Cone, Circle };
	bool Open(UMVAbilityBase* Ability, EShape InShape, float OuterRadius, float InnerRadius);
	void Close();

protected:
	bool IsOpenFor(const UMVAbilityBase* Ability) const;

private:
	virtual void BeginDestroy() override;
	void CheckArea();
#if WITH_EDITOR && ENABLE_DRAW_DEBUG
	bool ShouldDraw(const UWorld& World) const;
	void DrawArea(const UWorld& World, float Lifetime) const;
#endif

	TWeakObjectPtr<UMVAbilityBase> SourceAbility;
	TWeakObjectPtr<UWorld> WindowWorld;
	TSet<TWeakObjectPtr<AMVCharacterBase>> HitCharacters;
	FTimerHandle CheckTimer;
	FVector Origin = FVector::ZeroVector;
	FVector Forward = FVector::ZeroVector;
	float HalfHeight = 0.0f;
	double Range = 0.0;
	double AirborneRange = 0.0;
	EShape Shape = EShape::Cone;
	int32 AttackId = INDEX_NONE;
};

/** 기존 부채꼴 Blueprint의 열기·닫기 노드를 공용 판정 구간에 연결한다. */
UCLASS(BlueprintType)
class MAVERICK_API UMVWeaponSkillConeWindow : public UMVWeaponSkillAreaWindow
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Maverick|Weapon Skill")
	static UMVWeaponSkillConeWindow* OpenWeaponSkillCone(UMVAbilityBase* Ability, float Radius,
		UMVWeaponSkillConeWindow* ExistingWindow, float AirborneStartDistance = 700.0f);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Weapon Skill")
	static void CloseWeaponSkillCone(UMVWeaponSkillConeWindow* Window);
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
