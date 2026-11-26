#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Struct/MVHitTypes.h"
#include "MVDuelistDodgeChanceComponent.generated.h"

class UAnimMontage;

/** Duelist의 공격 알림별 회피 판정. 피격과 실제 회피 몽타주 시작 이벤트로 확률을 갱신한다. */
UCLASS(ClassGroup=(Maverick), meta=(BlueprintSpawnableComponent))
class MAVERICK_API UMVDuelistDodgeChanceComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UMVDuelistDodgeChanceComponent();
	bool ShouldEvade(AActor* Attacker);
	bool IsEvasionPlaying() const;

	UPROPERTY(EditAnywhere, Category="Dodge|Dash", meta=(ClampMin="0", Units="cm"))
	float BackDashDistance = 250.f;

	UPROPERTY(EditAnywhere, Category="Dodge|Dash", meta=(ClampMin="0.01", Units="s"))
	float BackDashDuration = 0.3f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Dodge")
	int32 CurrentDodgeChance = 20;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;

private:
	TWeakObjectPtr<AActor> Threat;
	uint16 BackDashId = 0;
	void StopBackDash();
	UFUNCTION()
	void OnMontageBlendOut(UAnimMontage* Montage, bool bInterrupted);
	UPROPERTY(EditDefaultsOnly, Category="Dodge")
	TObjectPtr<UAnimMontage> EvasionMontage;

	UFUNCTION()
	void OnDamaged(const FMVResolvedHitData& Hit);
	UFUNCTION()
	void OnMontageStarted(UAnimMontage* Montage);
};
