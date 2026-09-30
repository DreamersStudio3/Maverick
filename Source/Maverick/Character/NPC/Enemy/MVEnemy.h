// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/MVCharacterBase.h"
#include "TimerManager.h"
#include "MVEnemy.generated.h"

class UMVMainHUDWidget;
class AActor;
struct FMVAIDodgeRequest;

UCLASS()
class MAVERICK_API AMVEnemy : public AMVCharacterBase
{
	GENERATED_BODY()
	
public:
	AMVEnemy();
	virtual void BeginPlay() override;

	bool ReceiveAttackNotice(const FMVAIDodgeRequest& Notice);

	void HideBoundBossHUD();

	/**
	 * AIController가 런타임에 지정하는 공격 대상
	 * StateTree·블루프린트에서 읽을 수 있으며, 유효하지 않은 동안 대상 없음 상태
	 */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "AI")
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
