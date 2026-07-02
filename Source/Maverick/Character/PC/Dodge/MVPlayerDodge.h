#pragma once

#include "CoreMinimal.h"
#include "Enum/CharacterLocomotionEnums.h"
#include "GameplayTagContainer.h"
#include "Tables/MVMovementActionTableTypes.h"
#include "UObject/Object.h"
#include "UObject/SoftObjectPath.h"
#include "MVPlayerDodge.generated.h"

class AMVCharacterBase;
class AMVPlayerCharacter;
class UDataTable;
class UWorld;
class UMVActionComponent;
class UMVInputManagerComponent;

/**
 * PlayerCharacter 전용 회피 런타임 서브모듈.
 *
 * 플레이어 입력에서 Dodge row를 선택하고, ActionComponent 실행 API로 몽타주를 요청한다. Dodge가 필요로 하는
 * controller-space 입력 스냅샷, Dodge 시작 yaw, chooser 입력 플래그를 관리하며 이동 자체는 montage root motion이 담당한다.
 *
 * 책임:
 *   - 입력 버퍼 구간의 Dodge 이동 의도를 controller-space raw 2D로 저장하고 소비 시점에 재사용한다.
 *   - Roll과 대각 Step은 root motion 재생 전에 캐릭터 yaw를 입력 방향으로 보정한다.
 *   - 대각 Step chooser에는 앞대각 F, 뒤대각 B 문맥을 제공해 축 방향 root motion row를 재사용할 수 있게 한다.
 *
 * 라이프사이클:
 *   1) PlayerCharacter BeginPlay -> Initialize로 이동 입력 이벤트와 InputManager 액션 입력을 바인딩한다.
 *   2) Dodge 입력 -> 입력 스냅샷과 chooser 문맥을 준비하고, Chooser/DataTable row를 확정해 ActionComponent에 전달한다.
 *   3) 현재 Dodge의 RecoveryEscapeWindow 안에서 Dodge 입력이 들어오면 다음 Dodge row로 전환한다.
 *   4) PlayerCharacter EndPlay -> Deinitialize로 입력 이벤트 바인딩을 해제한다.
 */
UCLASS(BlueprintType, DefaultToInstanced, EditInlineNew)
class MAVERICK_API UMVPlayerDodge : public UObject
{
	GENERATED_BODY()

public:
	UMVPlayerDodge();

	virtual UWorld* GetWorld() const override;

	void Initialize(AMVPlayerCharacter& InOwnerCharacter);
	void Deinitialize();
	void PrepareDodgeAction();

	UFUNCTION(BlueprintCallable, Category = "Maverick|Action|Dodge")
	void UpdateBufferedDodgeMovementInput(const FVector& MovementInputDirection);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Action|Dodge|Chooser")
	FSoftObjectPath DodgeChooserTable = TEXT("/Game/Table/Dodge/CHT_Dodge.CHT_Dodge");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Action|Dodge|Chooser")
	bool bUseNamingConventionWhenChooserUnavailable = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Action|Dodge|Table", meta = (ClampMin = "1"))
	int32 DefaultDodgeRowIndex = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Action|Dodge|Recovery", meta = (ClampMin = "0.0", Units = "s"))
	float RecoveryDodgeTransitionBlendOutTime = 0.05f;

	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Maverick|Action|Dodge|Chooser")
	FMVDodgeActionRowHandle ChooserDodgeActionRowHandle;

private:
	AMVPlayerCharacter* GetPlayerCharacter() const;
	void CacheControllerSpaceMovementInput(const FVector2D& ControllerSpaceMovementInput);
	void HandleOwnerMovementInput(const FVector& MovementInputDirection);
	void BeginLockOnPawnRotationSuppressionForDodge(AMVCharacterBase& OwnerCharacter);
	void EndLockOnPawnRotationSuppressionForDodge();
	FVector2D CaptureControllerSpaceMovementInput(const AMVCharacterBase& OwnerCharacter) const;
	bool TryStartDodgeAction();
	bool TryConsumeBufferedDodgeInput();
	bool ResolveDodgeActionRowHandle(FMVDodgeActionRowHandle& OutActionRowHandle);
	bool EvaluateDodgeChooserActionRowHandle(FMVDodgeActionRowHandle& OutActionRowHandle);
	FName MakeDodgeActionTableName(FGameplayTag CharacterIndexCode) const;
	FName MakeDodgeActionTableName(const UDataTable* ActionDataTable) const;
	FName MakeDodgeActionRowName(FGameplayTag CharacterIndexCode, int32 Index) const;
	FGameplayTag ResolveCharacterIndexCode() const;
	bool CanTransitionActiveDodgeAction(const UMVInputManagerComponent& InputManager, const UMVActionComponent& ActionComponent) const;
	const FMVDodgeActionRow* FindDodgeActionRow(FDataTableRowHandle ActionRowHandle) const;
	bool CanConsumeDodgeCost(const FMVDodgeActionRow& DodgeActionRow) const;
	bool ConsumeDodgeCost(const FMVDodgeActionRow& DodgeActionRow);
	void ApplyDodgeChooserSnapshot(
		AMVCharacterBase& OwnerCharacter,
		bool bHasMovementInput,
		ELocomotionDirection InputDirection,
		const FVector& MovementDirection) const;

	UFUNCTION()
	void HandleActionInputSubmitted(int32 ActionId, FVector2D ControllerSpaceInput, bool bHasMovementInput);
	UFUNCTION()
	void HandleActionEnded(FName ActionTableName, FName ActionRowName, bool bInterrupted);
	UFUNCTION()
	void HandleRecoveryEscapeWindowChanged(bool bOpen);

	FName ActiveDodgeActionTableName = NAME_None;
	FName ActiveDodgeActionRowName = NAME_None;
	FVector2D CachedControllerSpaceMovementInput = FVector2D::ZeroVector;
	uint64 CachedControllerSpaceMovementInputFrame = 0;
	TWeakObjectPtr<AMVPlayerCharacter> OwnerPlayerCharacter;
	bool bLockOnPawnRotationSuppressedForDodge = false;
};
