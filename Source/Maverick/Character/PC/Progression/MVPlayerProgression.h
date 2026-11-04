#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "MVPlayerProgression.generated.h"

class AMVPlayerCharacter;
class UMVProgressionSubsystem;
class UMVStatComponent;
class UMVWorldStateSubsystem;
struct FMVPlayerProgressionSaveData;

/**
 * 플레이어 캐릭터와 영구 성장 상태를 연결하는 런타임 브리지.
 *
 * WorldState의 성장 변경과 StatComponent의 기본 스탯 준비 이벤트를 구독하고,
 * ProgressionSubsystem이 계산한 전체 성장 보너스를 StatComponent에 교체 적용한다.
 * 투자량·재화·성장 정의는 소유하지 않는다.
 *
 * 라이프사이클:
 *   1) PlayerCharacter BeginPlay에서 Initialize 호출
 *   2) 현재 저장 성장 상태를 즉시 StatComponent에 적용
 *   3) 저장 성장 변경 또는 기본 스탯 재로드 시 전체 보너스 재계산
 *   4) PlayerCharacter EndPlay에서 이벤트 구독 해제
 */
UCLASS(BlueprintType, DefaultToInstanced, EditInlineNew)
class MAVERICK_API UMVPlayerProgression : public UObject
{
	GENERATED_BODY()

public:
	virtual UWorld* GetWorld() const override;

	void Initialize(AMVPlayerCharacter& InOwnerCharacter);
	void Deinitialize();

	bool ApplyCurrentProgression();

private:
	UFUNCTION()
	void HandlePlayerProgressionChanged(const FMVPlayerProgressionSaveData& PlayerProgression);

	UFUNCTION()
	void HandleBaseStatsReady(int32 Revision);

	TWeakObjectPtr<AMVPlayerCharacter> OwnerPlayerCharacter;
	TWeakObjectPtr<UMVWorldStateSubsystem> WorldStateSubsystem;
	TWeakObjectPtr<UMVProgressionSubsystem> ProgressionSubsystem;
	TWeakObjectPtr<UMVStatComponent> StatComponent;
};