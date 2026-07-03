#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "MVInteractionCommandTypes.generated.h"

UENUM(BlueprintType)
enum class EMVInteractionCommandKind : uint8
{
	None,
	PlayAction,
	GameplayEvent
};

/**
 * Interaction flow의 leaf에서 실행되는 command 데이터의 공통 기반.
 *
 * Command는 flow step이나 menu entry에 배열로 조립된다. `PlayAction`은 애니메이션 프레젠테이션을
 * 요청하고 기본적으로 수동 완료를 기다리며, `GameplayEvent`는 대미지, 구매, 보관함 처리처럼 도메인
 * 시스템이 받아 처리할 이벤트를 전달한다.
 */
USTRUCT(BlueprintType, meta = (ToolTip = "Interaction flow leaf에서 실행되는 command의 기본 데이터입니다."))
struct MAVERICK_API FMVInteractionCommandData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Command", meta = (Categories = "Interaction.Command", ToolTip = "이 command를 구분하는 선택적 ID입니다."))
	FGameplayTag CommandId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Command", meta = (ToolTip = "true면 외부 로직이 FinishInteractionCommand를 호출할 때까지 다음 command나 step으로 넘어가지 않습니다."))
	bool bWaitForCompletion = false;
};

USTRUCT(BlueprintType, meta = (DisplayName = "Interaction Play Action Command", ToolTip = "MVActionRow 기반 액션/몽타주 실행을 요청하는 command입니다."))
struct MAVERICK_API FMVInteractionPlayActionCommandData : public FMVInteractionCommandData
{
	GENERATED_BODY()

	FMVInteractionPlayActionCommandData()
	{
		bWaitForCompletion = true;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Command", meta = (RowType = "/Script/Maverick.MVActionRow", ToolTip = "실행할 액션 DataTable row입니다. MVActionRow 계열 테이블만 선택합니다."))
	FDataTableRowHandle ActionRow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Command", meta = (ToolTip = "액션 몽타주를 특정 섹션에서 시작해야 할 때 입력합니다."))
	FName StartSection = NAME_None;
};

USTRUCT(BlueprintType, meta = (DisplayName = "Interaction Gameplay Event Command", ToolTip = "대미지, 구매, 보관함 처리처럼 도메인 로직에 이벤트를 전달하는 command입니다."))
struct MAVERICK_API FMVInteractionGameplayEventCommandData : public FMVInteractionCommandData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Command", meta = (Categories = "Interaction.Command.Event,Event", ToolTip = "도메인 로직이 분기할 이벤트 태그입니다. 예: Event.HitReaction.ApplyDamage 또는 Interaction.Command.Event.Shop.Buy"))
	FGameplayTag EventTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Command", meta = (ToolTip = "이벤트에 함께 전달할 태그 묶음입니다."))
	FGameplayTagContainer Tags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Command", meta = (ToolTip = "이벤트에 함께 전달할 이름 값입니다. 아이템 ID나 대상 ID 같은 간단한 키에 사용합니다."))
	FName Name = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Command", meta = (ToolTip = "이벤트에 함께 전달할 수치 값입니다. 대미지량, 가격, 개수 같은 값에 사용합니다."))
	float Magnitude = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Command", meta = (ToolTip = "필요할 때 도메인 로직에 전달할 UObject 페이로드입니다."))
	TObjectPtr<UObject> PayloadObject;
};

USTRUCT(BlueprintType)
struct MAVERICK_API FMVInteractionCommandRequest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Interaction|Command")
	EMVInteractionCommandKind CommandKind = EMVInteractionCommandKind::None;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Interaction|Command")
	FGameplayTag CommandId;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Interaction|Command")
	bool bWaitForCompletion = false;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Interaction|Command")
	FDataTableRowHandle ActionRow;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Interaction|Command")
	FName StartSection = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Interaction|Command")
	FGameplayTag EventTag;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Interaction|Command")
	FGameplayTagContainer Tags;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Interaction|Command")
	FName Name = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Interaction|Command")
	float Magnitude = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Interaction|Command")
	TObjectPtr<UObject> PayloadObject;
};
