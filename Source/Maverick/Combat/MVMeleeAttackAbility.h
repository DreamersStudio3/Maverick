#pragma once

#include "CoreMinimal.h"
#include "Combat/MVAbilityBase.h"
#include "Interface/MVAttackAbilityDataInterface.h"
#include "Tables/MVActionTableTypes.h"
#include "TimerManager.h"
#include "UObject/ObjectKey.h"
#include "MVMeleeAttackAbility.generated.h"

class AMVCharacterBase;
class UMeshComponent;

/**
 * 무기 소켓 구간을 긴 캡슐로 검사하는 근접 공격 Ability.
 *
 * 책임:
 *   - Ability NotifyState 활성 구간 동안 현재 장착 무기의 지정 소켓쌍을 주기적으로 샘플링한다.
 *   - 이전 샘플과 현재 샘플 사이를 촘촘히 보간해 빠른 스윙에서도 타격 궤적 빈틈을 줄인다.
 *   - 유효한 캐릭터 피해 후보를 액션 1회당 중복 제거한 뒤 HitResolver로 전달한다.
 *   - 디버그 캡슐, 소켓 라인, 샘플 이동 라인을 선택적으로 표시한다.
 *
 * 라이프사이클:
 *   1) CombatComponent가 row의 AbilityReference로 인스턴스를 만든다.
 *   2) Ability Notify Begin -> StartAbility -> 타이머 기반 캡슐 트레이스 시작.
 *   3) Ability Notify End 또는 액션 종료 -> EndAbility -> 타이머와 히트 캐시 정리.
 */
UCLASS(Blueprintable, BlueprintType, meta = (ShowWorldContextPin))
class MAVERICK_API UMVMeleeAttackAbility : public UMVAbilityBase
{
	GENERATED_BODY()

public:
	UMVMeleeAttackAbility();

	UFUNCTION(BlueprintCallable, Category = "Maverick|Ability|Melee")
	void SetWeaponTraceSockets(FName StartSocketName, FName EndSocketName);

	virtual void StartAbility_Implementation() override;
	virtual void EndAbility_Implementation() override;
	virtual void BeginDestroy() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Ability|Melee|Provider")
	TScriptInterface<IMVAttackAbilityDataInterface> ExplicitAttackAbilityDataProvider;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Ability|Melee|Trace")
	FName TraceStartSocketName = TEXT("Trace_Start");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Ability|Melee|Trace")
	FName TraceEndSocketName = TEXT("Trace_End");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Ability|Melee|Trace")
	bool bIncludeSecondaryWeapon = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Ability|Melee|Trace", meta = (ClampMin = "0.001", Units = "s"))
	float TraceInterval = 0.01f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Ability|Melee|Trace", meta = (ClampMin = "1.0", Units = "cm"))
	float MaxTraceStepDistance = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Ability|Melee|Trace", meta = (ClampMin = "1.0", Units = "cm"))
	float TraceCapsuleRadius = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Ability|Melee|Trace")
	TArray<TEnumAsByte<ECollisionChannel>> TargetObjectChannels;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Ability|Melee|Hit")
	bool bHitEachVictimOnce = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Ability|Melee|Hit")
	EMVActionHitReactionType HitReactionType = EMVActionHitReactionType::SmallHit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Ability|Melee|Debug")
	bool bDrawDebugTrace = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Ability|Melee|Debug", meta = (ClampMin = "0.0", Units = "s"))
	float DebugDrawDuration = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Ability|Melee|Debug")
	float DebugLineThickness = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Ability|Melee|Debug")
	FColor DebugTraceColor = FColor(255, 0, 0, 128);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Ability|Melee|Debug")
	FColor DebugHitColor = FColor(128, 255, 0, 128);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Ability|Melee|Debug")
	FColor DebugSocketLineColor = FColor(255, 255, 0, 128);

private:
	struct FMVMeleeAttackTraceSample
	{
		TWeakObjectPtr<UMeshComponent> SourceComponent;
		FVector StartLocation = FVector::ZeroVector;
		FVector EndLocation = FVector::ZeroVector;
		bool bSecondaryWeapon = false;
	};

	struct FMVMeleeAttackCapsule
	{
		FVector Center = FVector::ZeroVector;
		FQuat Rotation = FQuat::Identity;
		float Radius = 1.0f;
		float HalfHeight = 1.0f;
	};

	void BeginMeleeTrace();
	void StopMeleeTrace();
	void TickMeleeTrace();
	void TraceWeaponSegment(const FMVMeleeAttackTraceSample& CurrentSample);
	void PerformInitialOverlap(const FMVMeleeAttackTraceSample& CurrentSample, float CapsuleRadius);
	void PerformSubstepSweep(
		const FMVMeleeAttackTraceSample& PreviousSample,
		const FMVMeleeAttackTraceSample& CurrentSample,
		float CapsuleRadius);
	void ProcessHitActor(AActor* HitActor, const FVector& HitLocation);
	void CachePreviousSample(const FMVMeleeAttackTraceSample& Sample);
	const FMVMeleeAttackTraceSample* FindPreviousSample(const UMeshComponent* SourceComponent) const;
	void BuildObjectQueryParams(FCollisionObjectQueryParams& OutObjectQueryParams) const;
	bool CollectMeleeWeaponDataFromProvider(
		UObject* AbilityDataProvider,
		TArray<FMVMeleeWeaponData>& OutMeleeWeaponData) const;
	void CollectAttackAbilityDataProviders(TArray<UObject*>& OutProviders);
	void AddAttackAbilityDataProvider(UObject* Candidate, TArray<UObject*>& InOutProviders, TSet<TObjectKey<UObject>>& InOutProviderKeys) const;
	void DrawDebugTraceCapsule(
		const FMVMeleeAttackCapsule& Capsule,
		const FMVMeleeAttackTraceSample& Sample,
		bool bHit,
		const FVector* PreviousCenter = nullptr) const;
	float ResolveTraceCapsuleRadius() const;
	int32 ResolveSubstepCount(
		const FMVMeleeAttackTraceSample& PreviousSample,
		const FMVMeleeAttackTraceSample& CurrentSample) const;
	UWorld* GetAbilityWorld() const;
	static bool BuildTraceCapsule(
		const FMVMeleeAttackTraceSample& Sample,
		float CapsuleRadius,
		FMVMeleeAttackCapsule& OutCapsule);
	static AMVCharacterBase* ResolveHitCharacter(AActor* HitActor);

	FTimerHandle TraceTimerHandle;
	TArray<FMVMeleeAttackTraceSample> PreviousSamples;
	TSet<TObjectKey<AMVCharacterBase>> HitVictimKeys;
	bool bLoggedMissingTraceSegments = false;
};
