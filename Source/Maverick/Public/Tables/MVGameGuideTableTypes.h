#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Tables/MVTableTypes.h"
#include "MVGameGuideTableTypes.generated.h"

UENUM(BlueprintType)
enum class EMVGameGuideKind : uint8
{
	GameplayTip,
	SystemHelp,
	CombatTip,
	ExplorationTip,
	RespawnTip
};

/**
 * 게임 도움말과 팁을 정의하는 row.
 *
 * 특정 도메인의 원본 데이터가 아니라 조작, 전투, 탐험, 사망/부활 같은 게임 전반의 안내 문구를 보관한다.
 * 로딩 윈도우의 카드 섹션은 이 테이블의 row 중 bShowInLoading과 ContextTags 조건을 만족하는 항목을
 * 카드 형태로 렌더링한다.
 */
USTRUCT(BlueprintType, meta = (MVTable = "GameGuide"))
struct MAVERICK_API FMVGameGuideRow : public FMVTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|GameGuide")
	FString GuideId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|GameGuide")
	EMVGameGuideKind GuideKind = EMVGameGuideKind::GameplayTip;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|GameGuide")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|GameGuide", meta = (MultiLine = "true"))
	FText Body;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|GameGuide")
	FGameplayTagContainer ContextTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|GameGuide")
	bool bShowInLoading = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|GameGuide")
	int32 Priority = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|GameGuide", meta = (ClampMin = "0.0"))
	float Weight = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|GameGuide")
	bool bEnabled = true;
};
