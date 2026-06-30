#pragma once

#include "CoreMinimal.h"
#include "TimerManager.h"
#include "UObject/Object.h"
#include "MVDeathDissolveEffect.generated.h"

class UMaterialInstanceDynamic;
class UMaterialInterface;
class USkeletalMeshComponent;

USTRUCT()
struct FMVDeathDissolveMeshState
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	TObjectPtr<USkeletalMeshComponent> MeshComponent;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInterface>> OriginalMaterials;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInstanceDynamic>> DynamicMaterials;

	bool bWasHiddenInGame = false;
};

/**
 * DeathComponent가 소유하는 사망 dissolve effect strategy 객체.
 *
 * Actor에 별도 컴포넌트로 붙지 않고 DeathComponent의 instanced UObject로 존재하며, death dissolve cue를 받으면
 * 대상 actor의 skeletal mesh material slot에 Dynamic Material Instance를 만들고 dissolve parameter를 시간에 따라
 * 구동한다. 캐릭터별로 BP subclass를 꽂거나 이 객체를 비활성화해 C++ 기본 dissolve 대신 커스텀 연출을 사용할 수 있다.
 *
 * 라이프사이클:
 *   1) DeathComponent BeginPlay -> InitializeEffect로 대상 actor context를 받는다.
 *   2) DeathComponent dissolve cue -> StartDeathDissolve로 기본 dissolve 또는 BP override 연출을 실행한다.
 *   3) DeathComponent reset/EndPlay -> ResetDeathDissolveVisuals로 material과 hidden 상태를 복구한다.
 */
UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced)
class MAVERICK_API UMVDeathDissolveEffect : public UObject
{
	GENERATED_BODY()

public:
	virtual UWorld* GetWorld() const override;

	UFUNCTION(BlueprintCallable, Category = "Maverick|Death|Dissolve")
	void InitializeEffect(AActor* InOwnerActor);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Maverick|Death|Dissolve")
	void StartDeathDissolve(AActor* TargetActor);
	virtual void StartDeathDissolve_Implementation(AActor* TargetActor);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Maverick|Death|Dissolve")
	void ResetDeathDissolveVisuals();
	virtual void ResetDeathDissolveVisuals_Implementation();

	UFUNCTION(BlueprintCallable, Category = "Maverick|Death|Dissolve")
	void ApplyDeathDissolveAmount(float Amount);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Death|Dissolve")
	bool bEnableDeathDissolve = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Death|Dissolve", meta = (ClampMin = "0.0", Units = "s"))
	float DeathDissolveDuration = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Death|Dissolve", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DeathDissolveMaterialCompleteAmount = 0.33f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Death|Dissolve", meta = (ClampMin = "0.001", Units = "s"))
	float DeathDissolveUpdateInterval = 0.016f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Death|Dissolve")
	FName DeathDissolveAmountParameterName = TEXT("DeathDissolveAmount");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Death|Dissolve")
	bool bHideMeshAfterDeathDissolve = true;

private:
	void MigrateLegacyDurationIfNeeded();
	void UpdateDeathDissolve();
	void FinishDeathDissolve();

	TWeakObjectPtr<AActor> CachedOwnerActor;

	UPROPERTY(Transient)
	TArray<FMVDeathDissolveMeshState> DeathDissolveMeshStates;

	FTimerHandle DeathDissolveTimerHandle;
	float DeathDissolveStartTimeSeconds = 0.0f;
	bool bLegacyDurationMigrationChecked = false;
};
