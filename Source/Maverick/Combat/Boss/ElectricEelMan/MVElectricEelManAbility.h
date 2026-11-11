#pragma once
#include "Combat/MVAbilityBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MVElectricEelManAbility.generated.h"

/** W/E/R 공용 타이머·이동 잠금·적중 전달
 * Notify 시작 후 파생 TickSkill 실행, 종료·사망·소멸에서 타이머와 이동 복구
 * 판정은 플레이어 대상, 피해 계산과 피격 반응은 HitResolver에 위임
 */
UCLASS(Abstract, Blueprintable)
class MAVERICK_API UMVElectricEelManAbility : public UMVAbilityBase
{
	GENERATED_BODY()
public:
	virtual void EndAbility_Implementation() override;
	virtual void BeginDestroy() override;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Skill|Debug")
	bool bDrawDebug = true;
protected:
	bool BeginSkill(int32 AbilityIndex, bool bUltimate = false);
	AMVCharacterBase* FindTarget();
	bool IsEligiblePlayer(AMVCharacterBase* Player) const;
	bool HitPlayer(AMVCharacterBase* Player, float Multiplier, EMVActionHitReactionType Reaction);
	void HitSphere(const FVector& Center, float Radius, float Multiplier, EMVActionHitReactionType Reaction,
		TSet<TWeakObjectPtr<AActor>>& HitActors);
	void LockOwnerMovement();
	virtual void TickSkill(float Elapsed) PURE_VIRTUAL(UMVElectricEelManAbility::TickSkill, );
	virtual void StopExecution();
private:
	void UpdateSkill();
	FTimerHandle SkillTimer;
	TWeakObjectPtr<UWorld> SkillWorld;
	double StartTime = 0.0;
	TWeakObjectPtr<UCharacterMovementComponent> LockedMovement;
	EMovementMode SavedMode = MOVE_None;
	uint8 SavedCustomMode = 0;
};
