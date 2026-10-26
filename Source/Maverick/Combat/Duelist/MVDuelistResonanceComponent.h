#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MVDuelistResonanceComponent.generated.h"

class AMVCharacterBase;
class UAnimMontage;
class UMaterialInterface;
class UStaticMeshComponent;
class UMVStatComponent;
class UMVResonanceScreenWidget;

UENUM(BlueprintType)
enum class EMVDuelistResonancePhase : uint8 { Ready, Jump, Channel, Groggy, Finished };

/**
 * Duelist의 HP 50% 도달 예약·1회 중앙 도약·공명 폭주 실행 소유
 * StateTree Resonanace Task가 시작·종료 요청, 채널링 강인도 판정은 Stat·HitResolver 경유
 * UI 반복 시험은 HP 예약·1회 사용 표식 변경 없이 같은 실행 경로 사용, 활성 중 재시작 차단
 * BeginPlay에서 HP·그로기·사망 구독, 중단·EndPlay에서 이동·강인도·연출 복원
 * 실패 피해는 최대 HP 비율의 직접 피해, 암전은 플레이어 화면 위젯의 독립 10초 수명
 */
UCLASS(ClassGroup=(Maverick), meta=(BlueprintSpawnableComponent))
class MAVERICK_API UMVDuelistResonanceComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UMVDuelistResonanceComponent();
	UFUNCTION(BlueprintPure) bool IsResonancePending() const { return bPending && !bUsed; }
	UFUNCTION(BlueprintCallable) bool StartResonance();
	UFUNCTION(BlueprintCallable, Category="Resonance|Test") bool StartResonanceForTest();
	UFUNCTION(BlueprintCallable) void CancelResonance();
	UFUNCTION(BlueprintPure) bool IsResonanceActive() const;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Resonance") FVector ArenaCenter = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Resonance", meta=(ClampMin="0.1")) float JumpDuration = 0.75f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Resonance", meta=(ClampMin="0")) float JumpHeight = 350.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Resonance", meta=(ClampMin="0.1")) float ChannelDuration = 5.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Resonance", meta=(ClampMin="1")) float ChannelPoise = 40.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Resonance", meta=(ClampMin="0", ClampMax="1")) float FailureDamageRatio = 0.8f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Resonance", meta=(ClampMin="0.1")) float BlackoutDuration = 10.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Resonance", meta=(ClampMin="100", Units="cm")) float ArenaRadius = 4000.0f;
	UPROPERTY(EditAnywhere, Category="Resonance|Presentation") TObjectPtr<UAnimMontage> ChannelMontage;
	UPROPERTY(EditAnywhere, Category="Resonance|Presentation") TObjectPtr<UAnimMontage> JumpMontage;
	UPROPERTY(EditAnywhere, Category="Resonance|Presentation") TObjectPtr<UAnimMontage> GroggyMontage;
	UPROPERTY(EditAnywhere, Category="Resonance|Presentation") TObjectPtr<UMaterialInterface> WarningMaterial;
	UPROPERTY(EditAnywhere, Category="Resonance|Presentation") TObjectPtr<UMaterialInterface> EnergyMaterial;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Resonance") EMVDuelistResonancePhase Phase = EMVDuelistResonancePhase::Ready;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Resonance") float ChannelRemaining = 0.0f;
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* TickFunction) override;
private:
	bool BeginResonance(bool bForTest);
	UFUNCTION() void HandleHPChanged(float HP, float MaxHP);
	UFUNCTION() void HandleGroggy();
	UFUNCTION() void HandleDead();
	void BeginChannel();
	void Explode();
	void StopPresentation();
	void RestoreMovement();
	void PlayPose(UAnimMontage* Montage);
	UPROPERTY(Transient) TObjectPtr<AMVCharacterBase> Character;
	UPROPERTY(Transient) TObjectPtr<UMVStatComponent> Stat;
	UPROPERTY(Transient) TObjectPtr<UStaticMeshComponent> WarningDisc;
	UPROPERTY(Transient) TArray<TObjectPtr<UStaticMeshComponent>> EnergyOrbs;
	UPROPERTY(Transient) TArray<TObjectPtr<UMVResonanceScreenWidget>> Screens;
	UPROPERTY(Transient) TObjectPtr<UAnimMontage> PlayingMontage;
	bool bPending = false;
	float LastObservedHP = 0.0f;
	bool bUsed = false;
	bool bMovementSaved = false;
	uint8 SavedMovementMode = 0;
	uint8 SavedCustomMovementMode = 0;
	FVector JumpStart;
	FVector JumpEnd;
	float JumpElapsed = 0.0f;
	double ChannelDeadline = 0.0;
};
