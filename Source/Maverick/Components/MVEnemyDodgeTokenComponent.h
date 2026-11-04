#pragma once

#include "CoreMinimal.h"
#include "AI/MVDodgeThreatEvaluatorLibrary.h"
#include "Components/ActorComponent.h"
#include "Struct/MVHitTypes.h"
#include "MVEnemyDodgeTokenComponent.generated.h"

class AMVCharacterBase;
class AMVEnemy;

UENUM(BlueprintType)
enum class EMVEnemyDodgeTokenGrantReason : uint8
{
	None UMETA(DisplayName = "None"),
	GroggyRecovered UMETA(DisplayName = "Groggy Recovered"),
	ReceivedHitThreshold UMETA(DisplayName = "Received Hit Threshold"),
	LandedHitThreshold UMETA(DisplayName = "Landed Hit Threshold")
};

/**
 * Enemy-only dodge token state used by StateTree dodge threat filtering.
 *
 * The component observes enemy damage/groggy events and optional player damage
 * events, grants a limited dodge token from combat flow milestones, then spends
 * that token only after `UMVDodgeThreatEvaluatorLibrary` accepts a player
 * combat action as a valid dodge threat.
 */
UCLASS(ClassGroup = (Maverick), meta = (BlueprintSpawnableComponent))
class MAVERICK_API UMVEnemyDodgeTokenComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMVEnemyDodgeTokenComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	UFUNCTION(BlueprintCallable, Category = "Maverick|AI|Dodge")
	void NotifyEnemyDamaged(const FMVResolvedHitData& HitData);

	UFUNCTION(BlueprintCallable, Category = "Maverick|AI|Dodge")
	void NotifyEnemyLandedHit(const FMVResolvedHitData& HitData);

	UFUNCTION(BlueprintCallable, Category = "Maverick|AI|Dodge")
	void NotifyGroggyEndedByFinisher();

	UFUNCTION(BlueprintCallable, Category = "Maverick|AI|Dodge")
	void GrantDodgeToken(EMVEnemyDodgeTokenGrantReason Reason);

	UFUNCTION(BlueprintPure, Category = "Maverick|AI|Dodge")
	bool CanSpendDodgeToken() const;

	UFUNCTION(BlueprintCallable, Category = "Maverick|AI|Dodge")
	bool TrySpendDodgeToken();

	UFUNCTION(BlueprintCallable, Category = "Maverick|AI|Dodge")
	bool TryConsumeDodgeTokenForThreat(
		AActor* Target,
		const FMVCombatActionEvent& CombatActionEvent,
		const FMVDodgeThreatConfig& ThreatConfig,
		bool bOwnerActionRunning,
		FMVAIDodgeRequest& OutDodgeRequest,
		FMVDodgeThreatDecision& OutThreatDecision);

	UFUNCTION(BlueprintPure, Category = "Maverick|AI|Dodge")
	int32 GetDodgeTokenCount() const { return DodgeTokenCount; }

	UFUNCTION(BlueprintPure, Category = "Maverick|AI|Dodge")
	int32 GetReceivedHitCount() const { return ReceivedHitCount; }

	UFUNCTION(BlueprintPure, Category = "Maverick|AI|Dodge")
	int32 GetLandedHitCount() const { return LandedHitCount; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|AI|Dodge", meta = (ClampMin = "0"))
	int32 MaxDodgeTokens = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|AI|Dodge", meta = (ClampMin = "0"))
	int32 ReceivedHitThreshold = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|AI|Dodge", meta = (ClampMin = "0"))
	int32 LandedHitThreshold = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|AI|Dodge", meta = (ClampMin = "0.0", Units = "s"))
	float SpendCooldown = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|AI|Dodge")
	bool bGrantOnGroggyRecovered = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|AI|Dodge")
	bool bGrantOnReceivedHitThreshold = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|AI|Dodge")
	bool bGrantOnLandedHitThreshold = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|AI|Dodge")
	bool bAutoBindPlayerDamage = true;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Maverick|AI|Dodge")
	EMVEnemyDodgeTokenGrantReason LastGrantReason = EMVEnemyDodgeTokenGrantReason::None;

private:
	void BindOwnerEvents();
	void UnbindOwnerEvents();
	void BindObservedTargetDamage();
	void UnbindObservedTargetDamage();
	void ResetCombatFlowCounters();
	bool IsRelevantEnemyHitData(const FMVResolvedHitData& HitData) const;
	bool IsRelevantLandedHitData(const FMVResolvedHitData& HitData) const;

	UFUNCTION()
	void HandleOwnerDamaged(const FMVResolvedHitData& HitData);

	UFUNCTION()
	void HandleOwnerGroggyStarted();

	UFUNCTION()
	void HandleOwnerGroggyEnded();

	UFUNCTION()
	void HandleObservedTargetDamaged(const FMVResolvedHitData& HitData);

	UPROPERTY(Transient)
	TObjectPtr<AMVEnemy> OwnerEnemy;

	UPROPERTY(Transient)
	TObjectPtr<AMVCharacterBase> ObservedTargetCharacter;

	int32 DodgeTokenCount = 0;
	int32 ReceivedHitCount = 0;
	int32 LandedHitCount = 0;
	float NextSpendAllowedTime = 0.0f;
	bool bGroggyActive = false;
	bool bGroggyEndedByFinisher = false;
};
