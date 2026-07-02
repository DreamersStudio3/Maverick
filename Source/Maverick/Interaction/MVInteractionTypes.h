#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "UI/System/MVUIDataTypes.h"
#include "MVInteractionTypes.generated.h"

class UMVWindowBase;

UENUM(BlueprintType)
enum class EMVInteractionStepType : uint8
{
	None,
	Dialogue,
	Action,
	WarningPopup,
	Menu,
	Choice,
	Window
};

USTRUCT(BlueprintType)
struct MAVERICK_API FMVInteractionStepTransition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Transition")
	FName TriggerName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Transition")
	FName NextStepId = NAME_None;
};

/**
 * 상호작용 플로우의 모듈식 실행 단위.
 *
 * 각 step은 완료 시 `NextStepId`로 이동하고, 메뉴/선택지 step은 `Transitions`로 항목별 분기를 만든다.
 * `Action` step은 문 열기, 레버 조작, 아이템 획득, 사다리 오르기처럼 실제 액션/애니메이션이 진행되는
 * 구간이며, 애니메이션 notify나 도메인 로직의 완료 지점에서 `FinishInteractionAction`을 호출해야 끝난다.
 */
USTRUCT(BlueprintType)
struct MAVERICK_API FMVInteractionStepConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Step")
	FName StepId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Step")
	EMVInteractionStepType StepType = EMVInteractionStepType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Step")
	FName NextStepId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Action")
	FName ActionName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Dialogue")
	FName DialogueId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Dialogue")
	FText DialogueText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Dialogue", meta = (ClampMin = "-1.0"))
	float DialogueDuration = -1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Dialogue", meta = (ClampMin = "-1.0"))
	float DialogueMinimumSkipDelay = -1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Warning")
	FName WarningMessageId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Warning")
	FText WarningMessageText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Warning", meta = (ClampMin = "0.0"))
	float WarningDuration = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Menu")
	FMVInteractionMenuData MenuData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Window")
	TSubclassOf<UMVWindowBase> WindowClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Transition")
	TArray<FMVInteractionStepTransition> Transitions;
};

USTRUCT(BlueprintType)
struct MAVERICK_API FMVInteractionDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction")
	FName StartStepId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction")
	TArray<FMVInteractionStepConfig> Steps;
};
