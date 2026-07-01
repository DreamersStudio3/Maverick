#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Enum/CharacterLocomotionEnums.h"
#include "GameplayTagContainer.h"
#include "Tables/MVMovementActionTableTypes.h"
#include "UObject/SoftObjectPath.h"
#include "MVDodgeComponent.generated.h"

class AMVCharacterBase;
class UCurveFloat;
class UMVActionComponent;
class UMVInputManagerComponent;

/**
 * 회피 액션 전용 런타임 컴포넌트.
 *
 * InputManager 액션 입력에서 Dodge row를 선택하고, ActionComponent 실행 API로 몽타주를 요청한다.
 * Dodge가 필요로 하는 controller-space 입력 스냅샷, Dodge 시작 yaw, chooser 입력 플래그를 관리한다.
 * Dodge 이동 자체는 montage root motion이 담당한다.
 *
 * 책임:
 *   - 입력 버퍼 구간의 Dodge 이동 의도를 controller-space raw 2D로 저장하고 소비 시점에 재사용한다.
 *   - Roll과 대각 Step은 root motion 재생 전에 캐릭터 yaw를 입력 방향으로 보정한다.
 *   - 대각 Step chooser에는 앞대각 F, 뒤대각 B 문맥을 제공해 축 방향 root motion row를 재사용할 수 있게 한다.
 *   - 기존 DodgeLaunch NotifyState 호출은 애셋 호환을 위해 no-op으로 받는다.
 *
 * 라이프사이클:
 *   1) BeginPlay -> CharacterBase 이동 입력 이벤트와 InputManager 액션 입력을 바인딩한다.
 *   2) Dodge 입력 -> 입력 스냅샷과 chooser 문맥을 준비하고, Chooser/DataTable row를 확정해 ActionComponent에 전달한다.
 *   3) 현재 Dodge의 RecoveryEscapeWindow 안에서 Dodge 입력이 들어오면 다음 Dodge row로 전환한다.
 */
UCLASS(ClassGroup = (Maverick), meta = (BlueprintSpawnableComponent))
class MAVERICK_API UMVDodgeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMVDodgeComponent();

protected:
	virtual void BeginPlay() override;

public:
	void PrepareDodgeAction();

	UFUNCTION(BlueprintCallable, Category = "Maverick|Action|Dodge")
	void UpdateBufferedDodgeMovementInput(const FVector& MovementInputDirection);

	bool BeginDodgeLaunchWindow(
		float NotifyDuration,
		UCurveFloat* DistanceCurve,
		float DistanceScale,
		bool bApplyVerticalLaunch,
		int32 MontageInstanceId,
		bool bClearPreparedLaunch = true);
	void TickDodgeLaunchWindow(float DeltaTime, int32 MontageInstanceId);
	void EndDodgeLaunchWindow(bool bClearHorizontalVelocity, int32 MontageInstanceId);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Action|Dodge|Chooser")
	FSoftObjectPath DodgeChooserTable = TEXT("/Game/Table/Chooser/CHT_Dodge.CHT_Dodge");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Action|Dodge|Chooser")
	bool bUseNamingConventionWhenChooserUnavailable = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Action|Dodge|Table", meta = (ClampMin = "1"))
	int32 DefaultDodgeRowIndex = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Action|Dodge|Recovery", meta = (ClampMin = "0.0", Units = "s"))
	float RecoveryDodgeTransitionBlendOutTime = 0.05f;

	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Maverick|Action|Dodge|Chooser")
	FMVDodgeActionRowHandle ChooserDodgeActionRowHandle;

private:
	void CacheControllerSpaceMovementInput(const FVector2D& ControllerSpaceMovementInput);
	void HandleOwnerMovementInput(const FVector& MovementInputDirection);
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
	void HandleRecoveryEscapeWindowChanged(bool bOpen);

	FName ActiveDodgeActionTableName = NAME_None;
	FName ActiveDodgeActionRowName = NAME_None;
	FVector2D CachedControllerSpaceMovementInput = FVector2D::ZeroVector;
	uint64 CachedControllerSpaceMovementInputFrame = 0;
};
