#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "System/MVWorldStateTypes.h"
#include "MVWorldSaveGame.generated.h"

/**
 * Maverick 월드 상태를 저장 슬롯에 직렬화하는 SaveGame 오브젝트.
 *
 * 저장/로드 호출은 `UMVWorldStateSubsystem`이 담당하고, 이 클래스는 `FMVWorldSaveData`를
 * Unreal SaveGame 시스템에 실어 보내는 컨테이너 역할만 한다.
 */
UCLASS()
class MAVERICK_API UMVWorldSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Save")
	FMVWorldSaveData SaveData;
};
