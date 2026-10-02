#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "MVProgressionSettings.generated.h"

class UMVProgressionDefinition;

/**
 * 플레이어 성장 시스템에서 사용할 데이터 애셋 참조 설정.
 *
 * 프로젝트 설정에서 성장 정의를 지정하며 런타임 가변 상태는 소유하지 않는다.
 * UMVProgressionSubsystem이 GameInstance 초기화 시 이 참조를 로드한다.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Maverick Progression Settings"))
class MAVERICK_API UMVProgressionSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Definition")
	TSoftObjectPtr<UMVProgressionDefinition> DefinitionAsset;
};