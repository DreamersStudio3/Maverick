#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/MVInputManagerComponent.h"
#include "Engine/EngineTypes.h"
#include "Enum/MVEquipmentEnums.h"
#include "GameplayTagContainer.h"
#include "Struct/MVHitTypes.h"
#include "Tables/MVActionRowTableTypes.h"
#include "Tables/MVHitReactionActionTableTypes.h"
#include "UObject/SoftObjectPath.h"
#include "MVHitReactionComponent.generated.h"

class AMVCharacterBase;
class ACharacter;
class UMVActionComponent;

UENUM(BlueprintType)
enum class EMVHitReactionDirection : uint8
{
	Front,
	Left,
	Right,
	Back
};

struct FMVHitReactionActionData
{
	FName ActionTableName = NAME_None;
	FName ActionRowName = NAME_None;
	FName StartSection = NAME_None;
	EMVHitReactionDirection Direction = EMVHitReactionDirection::Front;
	FMVHitReactionActionRow ActionRow;
};

/**
 * 피격 리액션 액션 선택을 담당하는 컴포넌트.
 *
 * CharacterBase.OnDamaged에 바인딩되어 HitResolver가 확정한 HitReactionType과 충격 방향을
 * CHT_HitReaction의 선택 문맥으로 제공한다. Chooser가 반환한 상황별 액션 테이블 이름에서
 * 실행할 row name과 섹션을 확정한 뒤 ActionComponent의 실행 전용 API로 전달한다.
 *
 * 라이프사이클:
 *   1) BeginPlay -> 소유 캐릭터와 ActionComponent를 캐시한다.
 *   2) HandleDamaged -> CHT_HitReaction 또는 명명 규칙으로 상황별 액션 테이블을 고른다.
 *   3) 무적/interrupt 가능 여부를 확인하고 ActionComponent.TryStartActionFromTable로 재생한다.
 *   4) KD/AB recovery window -> 저장된 Dodge 입력은 별도 EscapeDodge 액션, 입력 없음/이동 입력은 별도 Getup 액션으로 전환한다.
 */
UCLASS(ClassGroup = (Maverick), meta = (BlueprintSpawnableComponent))
class MAVERICK_API UMVHitReactionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMVHitReactionComponent();

protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable, Category = "Maverick|HitReaction")
	void HandleDamaged(const FMVResolvedHitData& HitData);

	UFUNCTION(BlueprintPure, Category = "Maverick|HitReaction")
	EMVHitReactionDirection ResolveHitReactionDirection(const FMVResolvedHitData& HitData) const;

	UFUNCTION(BlueprintCallable, Category = "Maverick|HitReaction|Airborne")
	void BeginAirborneLandDetector();

	UFUNCTION(BlueprintCallable, Category = "Maverick|HitReaction|Airborne")
	void EndAirborneLandDetector();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|HitReaction|Chooser")
	FSoftObjectPath HitReactionChooserTable = TEXT("/Game/Table/Chooser/CHT_HitReaction.CHT_HitReaction");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|HitReaction|Chooser")
	bool bUseNamingConventionWhenChooserUnavailable = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|HitReaction|Table", meta = (ClampMin = "1"))
	int32 DefaultHitReactionRowIndex = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|HitReaction|Action")
	bool bCancelActiveActionBeforeReaction = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|HitReaction|Action", meta = (ClampMin = "0.0"))
	float CancelActiveActionBlendOutTime = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|HitReaction|Recovery", meta = (ClampMin = "0.0"))
	float RecoveryEscapeCancelBlendOutTime = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|HitReaction|Recovery", meta = (ClampMin = "0.0"))
	float RecoveryActionTransitionBlendOutTime = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|HitReaction|Recovery", meta = (ClampMin = "1"))
	int32 DefaultRecoveryRowIndex = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|HitReaction|Recovery")
	EMVActionInputDirection DefaultEscapeDodgeDirection = EMVActionInputDirection::Back;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|HitReaction|Recovery")
	FName GetupRecoveryActionTableName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|HitReaction|Recovery")
	FName EscapeDodgeRecoveryActionTableName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|HitReaction|Airborne")
	FName AirborneLandSectionName = TEXT("Land");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|HitReaction|Airborne")
	bool bRequireFallingBeforeAirborneLand = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Maverick|HitReaction|Chooser", meta = (Categories = "Character"))
	FGameplayTag ChooserCharacterIndexCode;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Maverick|HitReaction|Chooser", meta = (Categories = "Character"))
	FGameplayTagContainer ChooserCharacterIndexCodeTags;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Maverick|HitReaction|Chooser")
	EMVEquippedStyle ChooserEquippedStyle = EMVEquippedStyle::BareHand;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Maverick|HitReaction|Chooser")
	EMVActionHitReactionType ChooserHitReactionType = EMVActionHitReactionType::None;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Maverick|HitReaction|Chooser")
	EMVHitReactionDirection ChooserHitReactionDirection = EMVHitReactionDirection::Front;

private:
	void CacheOwnerReferences();
	void BindInputManagerHandlers();
	void BindActionComponentHandlers();
	bool GetActionData(const FMVResolvedHitData& HitData, FMVHitReactionActionData& OutActionData);
	void ApplyHitReactionLaunch(const FMVResolvedHitData& HitData, const FMVHitReactionActionRow& ActionRow);
	bool TryConsumeBufferedRecoveryInput();
	bool TryConsumeBufferedRecoveryMovementInput();
	bool TryConsumeRecoveryInput(int32 ActionId, FVector2D ControllerSpaceInput, bool bHasMovementInput);
	bool TryConsumeRecoveryMovementInput(FVector2D ControllerSpaceInput, bool bHasMovementInput);
	bool TryStartDefaultRecoveryAction();
	bool TryStartEscapeDodgeRecoveryAction(EMVActionInputDirection Direction);
	bool TryTransitionRecoveryAction(FName ActionTableName, FName ActionRowName, const TCHAR* Source);
	bool ShouldCancelRecoveryInputDirectly() const;
	bool IsAirborneLandDetectorActive() const;
	void BindAirborneMovementModeChanged();
	void UnbindAirborneMovementModeChanged();
	void ResetAirborneLandDetector();
	void TryJumpAirborneLandSection();
	bool TryCancelActiveRecoveryAction();
	FName ResolveGetupRecoveryActionTableName() const;
	FName ResolveEscapeDodgeRecoveryActionTableName() const;
	FName ResolveHitReactionActionTableName(
		EMVActionHitReactionType HitReactionType,
		EMVHitReactionDirection Direction) const;
	EMVHitReactionDirection ResolveSupportedHitReactionDirection(
		EMVActionHitReactionType HitReactionType,
		EMVHitReactionDirection Direction) const;
	FName EvaluateHitReactionChooserTable() const;
	FName MakeHitReactionActionTableName(
		FGameplayTag CharacterIndexCode,
		EMVActionHitReactionType HitReactionType,
		EMVHitReactionDirection Direction) const;
	FName MakeHitReactionRecoveryActionTableName(FGameplayTag CharacterIndexCode, FName RecoveryType) const;
	FName MakeHitReactionActionRowName(
		FGameplayTag CharacterIndexCode,
		EMVActionHitReactionType HitReactionType,
		EMVHitReactionDirection Direction,
		int32 Index) const;
	FName MakeGetupRecoveryActionRowName(FGameplayTag CharacterIndexCode, EMVHitReactionDirection Direction, int32 Index) const;
	FName MakeEscapeDodgeRecoveryActionRowName(
		FGameplayTag CharacterIndexCode,
		EMVHitReactionDirection FallDirection,
		EMVActionInputDirection EscapeDirection,
		int32 Index) const;
	FGameplayTag ResolveCharacterIndexCode() const;
	const FMVHitReactionActionRow* FindHitReactionActionRow(FName ActionTableName, FName ActionRowName) const;
	const FMVActionRow* FindRecoveryActionRow(FName ActionTableName, FName ActionRowName) const;
	static FString CharacterIndexCodeToTableToken(FGameplayTag CharacterIndexCode);
	static FString HitReactionTypeToTableToken(EMVActionHitReactionType HitReactionType);
	static FString HitReactionDirectionToTableToken(EMVHitReactionDirection Direction);
	static FString ActionInputDirectionToTableToken(EMVActionInputDirection Direction);

	UFUNCTION()
	void HandleActionInputSubmitted(int32 ActionId, FVector2D ControllerSpaceInput, bool bHasMovementInput);

	UFUNCTION()
	void HandleActionEnded(FName ActionTableName, FName ActionRowName, bool bInterrupted);

	UFUNCTION()
	void HandleRecoveryEscapeWindowChanged(bool bOpen);

	UFUNCTION()
	void HandleOwnerMovementModeChanged(ACharacter* Character, EMovementMode PrevMovementMode, uint8 PreviousCustomMode);

	UPROPERTY(Transient)
	TObjectPtr<AMVCharacterBase> OwnerCharacter;

	UPROPERTY(Transient)
	TObjectPtr<UMVActionComponent> CachedActionComponent;

	UPROPERTY(Transient)
	TObjectPtr<UMVInputManagerComponent> CachedInputManager;

	FName ActiveHitReactionActionRowName = NAME_None;
	EMVActionHitReactionType ActiveHitReactionType = EMVActionHitReactionType::None;
	EMVHitReactionDirection ActiveHitReactionDirection = EMVHitReactionDirection::Front;
	int32 AirborneLandDetectorCount = 0;
	bool bAirborneMovementModeDelegateBound = false;
	bool bAirborneLandDetectorSawFalling = false;
	bool bAirborneLandJumpRequested = false;
};
