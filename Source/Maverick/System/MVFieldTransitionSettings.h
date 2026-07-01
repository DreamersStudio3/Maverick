#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/DeveloperSettings.h"
#include "MVFieldTransitionSettings.generated.h"

/**
 * 필드 전환 전후에 재생할 공용 액션 row 설정.
 *
 * 사망 부활, 체크포인트 이동, 안개문 진입처럼 로딩이나 위치 이동을 동반하는 흐름에서 사용할
 * 캐릭터 연출 row를 프로젝트 설정으로 노출한다. 실제 전환 상태 머신은 `UMVFieldTransitionSubsystem`이
 * 소유하고, 이 객체는 에디터에서 조정 가능한 데이터 참조만 보관한다.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Maverick Field Transition Settings"))
class MAVERICK_API UMVFieldTransitionSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Death Respawn|Action")
	FDataTableRowHandle DeathRespawnPostTransitionActionRow;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Death Respawn|Action")
	FName DeathRespawnPostTransitionStartSection = NAME_None;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Checkpoint Travel|Action")
	FDataTableRowHandle CheckpointTravelPreTransitionActionRow;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Checkpoint Travel|Action")
	FName CheckpointTravelPreTransitionStartSection = NAME_None;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Checkpoint Travel|Action")
	FDataTableRowHandle CheckpointTravelPostTransitionActionRow;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Checkpoint Travel|Action")
	FName CheckpointTravelPostTransitionStartSection = NAME_None;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Fog Gate|Action")
	FDataTableRowHandle FogGateEnterActionRow;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Fog Gate|Action")
	FName FogGateEnterStartSection = NAME_None;
};
