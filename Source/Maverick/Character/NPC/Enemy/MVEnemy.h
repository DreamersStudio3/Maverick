// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/MVCharacterBase.h"
#include "TimerManager.h"
#include "MVEnemy.generated.h"

class UMVMainHUDWidget;
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
