#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/MVCombatComponent.h"
#include "Struct/MVHitTypes.h"
#include "MVCombatStateComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMVOnCombatStateChangedSignature, bool, bInCombat);

/**
 * 캐릭터의 전투·비전투 상태 전환 판정 컴포넌트.
 *
 * 책임:
 *   - 전투 액션 시작과 확정 타격을 전투 활동으로 기록
 *   - 마지막 전투 활동 이후 지연 시간 경과 여부 판정
 *   - Groggy, Lying, Exhaustion, Debuff 상태의 비전투 전환 차단
 *   - 전투 상태 변경 이벤트 제공
 *   - 전투 상태와 방해 조건을 함께 반영한 비전투 회복 가능 여부 제공
 */
UCLASS(ClassGroup = (Maverick), meta = (BlueprintSpawnableComponent))
class MAVERICK_API UMVCombatStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMVCombatStateComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintPure, Category = "Maverick|Combat State")
	bool IsInCombat() const { return bInCombat; }

	UFUNCTION(BlueprintPure, Category = "Maverick|Combat State")
	bool IsOutOfCombat() const;

	// Guard 등 이후 추가될 전투 행동에서 공통으로 호출할 진입점
	UFUNCTION(BlueprintCallable, Category = "Maverick|Combat State")
	void NotifyCombatActivity();

	UFUNCTION(BlueprintCallable, Category = "Maverick|Combat State")
	void SetAggroThreatActive(AActor* ThreatActor, bool bActive);

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Combat State|Event")
	FMVOnCombatStateChangedSignature OnCombatStateChanged;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UFUNCTION()
	void HandleCombatActionStarted(const FMVCombatActionEvent& Event);

	UFUNCTION()
	void HandleHitResolved(const FMVResolvedHitData& HitData);

	bool HasCombatExitBlocker() const;
	bool HasActiveDebuff() const;
	bool HasActiveAggroThreat() const;
	void SetInCombat(bool bNewInCombat);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Combat State", meta = (ClampMin = "0.0", Units = "s", AllowPrivateAccess = "true"))
	float CombatExitDelay = 3.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Combat State", meta = (AllowPrivateAccess = "true"))
	bool bInCombat = false;

	double LastCombatActivityTime = 0.0;

	TSet<TWeakObjectPtr<AActor>> ActiveAggroThreats;
};
