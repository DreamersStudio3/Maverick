#pragma once

#include "CoreMinimal.h"
#include "Components/MVCombatComponent.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Struct/MVAIDodgeTypes.h"
#include "MVDodgeThreatEvaluatorLibrary.generated.h"

UENUM(BlueprintType)
enum class EMVDodgeThreatRejectReason : uint8
{
	None UMETA(DisplayName = "None"),
	Disabled UMETA(DisplayName = "Disabled"),
	InvalidOwner UMETA(DisplayName = "Invalid Owner"),
	InvalidThreat UMETA(DisplayName = "Invalid Threat"),
	ThreatIsSelf UMETA(DisplayName = "Threat Is Self"),
	TargetMismatch UMETA(DisplayName = "Target Mismatch"),
	ActionTypeRejected UMETA(DisplayName = "Action Type Rejected"),
	ActionRunning UMETA(DisplayName = "Action Running"),
	TooFar UMETA(DisplayName = "Too Far"),
	OutsideAngle UMETA(DisplayName = "Outside Angle"),
	TokenUnavailable UMETA(DisplayName = "Token Unavailable")
};

USTRUCT(BlueprintType)
struct MAVERICK_API FMVDodgeThreatConfig
{
	GENERATED_BODY()

	FMVDodgeThreatConfig();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|AI|Dodge")
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|AI|Dodge")
	bool bRequireInstigatorIsTarget = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|AI|Dodge")
	bool bAllowWhileActionRunning = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|AI|Dodge", meta = (ClampMin = "0.0"))
	float MaxDistance = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|AI|Dodge", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float MaxAbsAngle = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|AI|Dodge")
	TArray<EMVCombatActionTypes> ThreatActionTypes;
};

USTRUCT(BlueprintType)
struct MAVERICK_API FMVDodgeThreatDecision
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|AI|Dodge")
	bool bShouldSendEvent = false;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|AI|Dodge")
	EMVDodgeThreatRejectReason RejectReason = EMVDodgeThreatRejectReason::None;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|AI|Dodge")
	FMVAIDodgeRequest DodgeRequest;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|AI|Dodge")
	float DistanceToThreat = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|AI|Dodge")
	float AngleToThreat = 0.0f;
};

UCLASS()
class MAVERICK_API UMVDodgeThreatEvaluatorLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Maverick|AI|Dodge")
	static FMVDodgeThreatDecision EvaluateDodgeThreat(
		AActor* Owner,
		AActor* Target,
		const FMVCombatActionEvent& CombatActionEvent,
		const FMVDodgeThreatConfig& Config,
		bool bOwnerActionRunning);
};
