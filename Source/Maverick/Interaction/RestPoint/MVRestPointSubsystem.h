#pragma once

#include "CoreMinimal.h"
#include "Struct/MVHitTypes.h"
#include "Subsystems/WorldSubsystem.h"
#include "MVRestPointSubsystem.generated.h"

class AMVPlayerCharacter;
class AMVRestPoint;
class UCommonActivatableWidget;
class UMVHUDWidgetBase;
class UMVInputManagerComponent;
class UMVRestPointWindow;
class UMVStatComponent;
class UMVUILayerBase;
enum class ESlateVisibility : uint8;
struct FMVCheckpointSaveData;

enum class EMVRestPointDetail : uint8
{
	LevelUp,
	PassiveSkills
};

/**
 * 현재 월드에서 로컬 플레이어의 거점 이용을 한 번에 하나만 관리한다.
 * CommonUI 목록과 상세 창을 소유하며, 두 화면을 오가는 동안 HUD와 게임 입력을 차단한다.
 * 피격·이탈·전환 시 창과 차단을 정리하고 월드 종료 때 피격 구독을 해제한다.
 */
UCLASS()
class MAVERICK_API UMVRestPointSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual void Deinitialize() override;

	bool Open(AMVRestPoint& RestPoint, AMVPlayerCharacter& Player);
	void CloseFor(const AMVRestPoint& RestPoint);
	bool NotifyMenuActivated(const UMVRestPointWindow& Window, bool& bReturningFromDetail);
	void NotifyMenuDeactivated(const UMVRestPointWindow& Window);
	bool OpenDetail(const UMVRestPointWindow& Window, EMVRestPointDetail Detail);
	bool RequestRest(const UMVRestPointWindow& Window);
	bool IsRestRequestValid(uint64 RequestId) const;
	void FinishRestRequest(uint64 RequestId, bool bApplied);
	bool IsRestPending() const { return PendingRestRequestId != 0; }
	const FText& GetRestStatus() const { return RestStatus; }
	bool CanUseDetail() const;
	bool TryResolveRespawnAnchor(const FMVCheckpointSaveData& Record, FTransform& OutTransform) const;
	bool IsUsingRestPoint(const AMVPlayerCharacter& Player) const;
	FText GetEntryBlockReason(const AMVRestPoint& RestPoint, const AMVPlayerCharacter& Player) const;
	const AMVRestPoint* GetActiveRestPoint() const { return ActiveRestPoint.Get(); }

protected:
	virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;

private:
	bool HasValidSessionPlayer(const AMVPlayerCharacter* Player) const;
	void ObservePlayer(AMVPlayerCharacter* Player);
	void Close();
	void ClearSession();

	UFUNCTION()
	void HandleDamage(float AppliedDamage, float PreviousHP, float CurrentHP, const FMVResolvedHitData& HitData);

	TWeakObjectPtr<AMVRestPoint> ActiveRestPoint;
	TWeakObjectPtr<AMVPlayerCharacter> ActivePlayer;
	TWeakObjectPtr<UMVRestPointWindow> ActiveWindow;
	TWeakObjectPtr<UCommonActivatableWidget> ActiveDetail;
	TWeakObjectPtr<UMVUILayerBase> ActiveLayer;
	TWeakObjectPtr<UMVInputManagerComponent> BlockedInput;
	TWeakObjectPtr<UMVStatComponent> ObservedStats;
	TWeakObjectPtr<UMVHUDWidgetBase> SuppressedHUD;
	ESlateVisibility PreviousHUDVisibility{};
	TOptional<double> LastDamageAt;
	double NextSafetyCheckAt = 0.0;
	uint64 NextRestRequestId = 0;
	uint64 PendingRestRequestId = 0;
	FText RestStatus;
	bool bMenuOpened = false;
	bool bOpeningDetail = false;
	bool bMenuClosed = false;
	bool bClosing = false;
};
