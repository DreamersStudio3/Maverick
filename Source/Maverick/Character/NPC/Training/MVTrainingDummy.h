#pragma once

#include "CoreMinimal.h"
#include "Character/MVCharacterBase.h"
#include "MVTrainingDummy.generated.h"

class UAnimSequence;
class UAudioComponent;
class USoundBase;

/**
 * 무기 판정으로 반복 타격 가능한 고정식 훈련 대상
 * BeginPlay에서 대기 동작 시작, 공용 OnDamaged 수신 시 피격 동작·효과음 재생
 * 체력 감소·밀림·사망 처리 대신 자체 표현만 구독
 * Tick에서 실제 재생 종료 후 대기 복귀, EndPlay에서 구독 해제
 */
UCLASS()
class MAVERICK_API AMVTrainingDummy : public AMVCharacterBase
{
	GENERATED_BODY()

public:
	AMVTrainingDummy();
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Training")
	TObjectPtr<UAnimSequence> IdleAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Training")
	TObjectPtr<UAnimSequence> HitAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Training")
	TArray<TObjectPtr<USoundBase>> HitSounds;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Training")
	TObjectPtr<UAudioComponent> HitAudio;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Training")
	int32 HitCount = 0;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void BindDamageHandlers() override;

private:
	UFUNCTION()
	void HandleTrainingHit(const FMVResolvedHitData& HitData);

	void PlayIdle();
};
