#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MVActionCooldownComponent.generated.h"

USTRUCT(BlueprintType)
struct FMVActionCooldownDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cooldown")
	FName ActionId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cooldown", meta = (ClampMin = "0.0"))
	float CooldownDuration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cooldown")
	bool bStartReady = true;
};

USTRUCT(BlueprintType)
struct FMVActionCooldownState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cooldown")
	float CooldownDuration = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cooldown")
	float RemainingTime = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cooldown")
	bool bIsReady = true;
};

/**
 * AI Action ID별 남은 쿨다운과 ready 상태를 보관하는 ActorComponent.
 * StateTree에서는 GlobalSensing 또는 Global Action Cooldown Task 한 곳만 이 컴포넌트를 구성하고 tick한다.
 */
UCLASS(ClassGroup = (AI), meta = (BlueprintSpawnableComponent))
class MAVERICK_API UMVActionCooldownComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMVActionCooldownComponent();

	void ConfigureCooldowns(const TArray<FMVActionCooldownDefinition>& Definitions);
	void TickCooldowns(float DeltaTime);

	bool StartCooldown(FName ActionId);
	bool IsCooldownReady(FName ActionId) const;
	float GetRemainingTime(FName ActionId) const;
	void GetReadyActionIds(TArray<FName>& OutActionIds) const;

	const TMap<FName, FMVActionCooldownState>& GetCooldownStates() const { return CooldownStates; }

private:
	UPROPERTY(VisibleAnywhere, Category = "Cooldown")
	TMap<FName, FMVActionCooldownState> CooldownStates;
};
