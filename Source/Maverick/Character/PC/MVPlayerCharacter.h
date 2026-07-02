// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/MVCharacterBase.h"
#include "MVPlayerCharacter.generated.h"

UCLASS()
class MAVERICK_API AMVPlayerCharacter : public AMVCharacterBase
{
	GENERATED_BODY()

public:
	AMVPlayerCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void BeginLockOnPawnRotationSuppression();
	void EndLockOnPawnRotationSuppression();

protected:
	virtual void BeginPlay() override;

private:
	virtual void BindDamageHandlers() override;
	void RefreshLockOnPawnRotationExtension();
	bool ShouldSuppressLockOnPawnRotation() const;

	int32 LockOnPawnRotationSuppressionCount = 0;
};
