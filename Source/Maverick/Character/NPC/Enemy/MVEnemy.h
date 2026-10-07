// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/MVCharacterBase.h"
#include "TimerManager.h"
#include "MVEnemy.generated.h"

class UMVMainHUDWidget;
class AActor;
struct FMVAIDodgeRequest;

/**
 * 적 공통 대상 상태·피해 이벤트·보스 HUD 연결의 캐릭터 브리지
 * BeginPlay의 HUD 연결 재시도와 EndPlay의 해제 소유
 * 대상 방향 계산은 C++, 회전 실행·구간 제어는 StateTree Task·Notify 책임
 */
UCLASS()
class MAVERICK_API AMVEnemy : public AMVCharacterBase
{
	GENERATED_BODY()
	
public:
	AMVEnemy();
	virtual void BeginPlay() override;

	bool ReceiveAttackNotice(const FMVAIDodgeRequest& Notice);

	void HideBoundBossHUD();

	/** 공격 대상의 수평 방향, 대상 부재·동일 수평 위치에서는 현재 회전 유지 */
	UFUNCTION(BlueprintPure, Category = "AI|Target")
	FRotator GetTargetRotation() const;

	/**
	 * AIController가 런타임에 지정하는 공격 대상
	 * StateTree·블루프린트에서 읽을 수 있으며, 유효하지 않은 동안 대상 없음 상태
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Target")
	TObjectPtr<AActor> AttackTarget = nullptr;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Enemy|Event")
	FMVOnDamagedSignature OnEnemyDamaged;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void BindDamageHandlers() override;

	void BindBossHUDToMainHUD();
	UFUNCTION()
	void HandleEnemyDamaged(const FMVResolvedHitData& HitData);

	FTimerHandle BossHUDBindRetryTimerHandle;
	TWeakObjectPtr<UMVMainHUDWidget> BoundBossHUD;
};
