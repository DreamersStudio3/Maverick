#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Enum/MVStatusEffectEnums.h"
#include "Struct/MVStatusEffectTypes.h"
#include "TimerManager.h"
#include "MVStatusEffectComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FMVStatusEffectAddedSignature, const FMVStatusEffectInstance&, Instance);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FMVStatusEffectUpdatedSignature, const FMVStatusEffectInstance&, Instance, int32, PreviousStacks);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FMVStatusEffectRemovedSignature, const FMVStatusEffectInstance&, Instance, EMVStatusEffectRemovalReason, RemovalReason);

class AActor;
class UMVStatusEffectDefinition;

UCLASS(ClassGroup = (Maverick), meta = (BlueprintSpawnableComponent))
class MAVERICK_API UMVStatusEffectComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMVStatusEffectComponent();

	const TArray<FMVStatusEffectInstance>& GetActiveEffects() const { return ActiveEffects; }

	UFUNCTION(BlueprintPure, Category = "Maverick|StatusEffect")
	int32 GetActiveEffectCount() const { return ActiveEffects.Num(); }

	FMVStatusEffectHandle FindStatusEffectHandle(const UMVStatusEffectDefinition* Definition, const AActor* SourceActor = nullptr) const;

	bool HasStatusEffect(const UMVStatusEffectDefinition* Definition, const AActor* SourceActor = nullptr) const;

	bool SetStatusEffectStacks(FMVStatusEffectHandle Handle, int32 NewStacks);

	UFUNCTION(BlueprintCallable, Category = "Maverick|StatusEffect")
	FMVStatusEffectHandle ApplyStatusEffect(const FMVStatusEffectSpec& Spec);

	UFUNCTION(BlueprintCallable, Category = "Maverick|StatusEffect")
	bool RemoveStatusEffect(FMVStatusEffectHandle Handle, EMVStatusEffectRemovalReason RemovalReason = EMVStatusEffectRemovalReason::Manual);

	UFUNCTION(BlueprintCallable, Category = "Maverick|StatusEffect")
	int32 RemoveAllStatusEffects(EMVStatusEffectRemovalReason RemovalReason = EMVStatusEffectRemovalReason::Cleared);

	UPROPERTY(BlueprintAssignable, Category = "Maverick|StatusEffect")
	FMVStatusEffectAddedSignature OnStatusEffectAdded;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|StatusEffect")
	FMVStatusEffectUpdatedSignature OnStatusEffectUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|StatusEffect")
	FMVStatusEffectRemovedSignature OnStatusEffectRemoved;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	FMVStatusEffectHandle GenerateHandle();

	FMVStatusEffectInstance* FindMatchingInstance(const FMVStatusEffectSpec& Spec);

	FMVStatusEffectHandle CreateInstance(const FMVStatusEffectSpec& Spec, double CurrentTime);

	void ReapplyToInstance(FMVStatusEffectInstance& Instance, const FMVStatusEffectSpec& Spec, double CurrentTime);

	void RemoveExpiredEffects(double CurrentTime);

	double CalculateInitialExpireTime(const UMVStatusEffectDefinition& Definition, double CurrentTime) const;

	void ScheduleNextExpiration();

	void HandleExpirationTimer();

	bool RemoveInstanceAtIndex(int32 Index, EMVStatusEffectRemovalReason RemovalReason);

	void CreateRuntimeBehaviors(FMVStatusEffectInstance& Instance);

	void NotifyBehaviorsApplied(const FMVStatusEffectInstance& Instance);

	void NotifyBehaviorsUpdated(const FMVStatusEffectInstance& Instance, int32 PreviousStacks);

	void NotifyBehaviorsRemoved(const FMVStatusEffectInstance& Instance, EMVStatusEffectRemovalReason RemovalReason);

private:
	UPROPERTY(Transient)
	TArray<FMVStatusEffectInstance> ActiveEffects;

	FTimerHandle ExpirationTimerHandle;

	int64 NextHandleValue = 1;
};
