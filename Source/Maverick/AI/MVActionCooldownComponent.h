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
