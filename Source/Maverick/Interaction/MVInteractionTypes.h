#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Interaction/MVInteractionCommandTypes.h"
#include "StructUtils/InstancedStruct.h"
#include "Templates/SubclassOf.h"
#include "UI/System/MVUIDataTypes.h"
#include "MVInteractionTypes.generated.h"

class UMVWindowBase;

USTRUCT(BlueprintType, meta = (ToolTip = "메뉴 항목을 선택했을 때 이동할 다음 step을 연결합니다."))
struct MAVERICK_API FMVInteractionStepTransition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Transition", meta = (Categories = "Interaction.Menu.Entry", ToolTip = "이 transition을 실행할 메뉴 항목 EntryId입니다."))
	FGameplayTag TriggerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Transition", meta = (Categories = "Interaction.Flow.Step", ToolTip = "TriggerId 항목이 선택되면 이동할 다음 step입니다. 비워두면 flow를 종료합니다."))
	FGameplayTag NextStepId;
};

USTRUCT(BlueprintType, meta = (ToolTip = "대화 중 잠깐 뜨는 단층 선택지의 항목입니다."))
struct MAVERICK_API FMVInteractionChoiceEntryData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Choice", meta = (Categories = "Interaction.Choice.Entry", ToolTip = "선택지 항목의 고유 태그입니다."))
	FGameplayTag ChoiceId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Choice", meta = (ToolTip = "화면에 표시할 선택지 문구입니다."))
	FText Label;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Choice", meta = (ToolTip = "false면 선택지는 표시되지만 선택할 수 없습니다."))
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Choice", meta = (Categories = "Interaction.Flow.Step", ToolTip = "이 선택지를 골랐을 때 이동할 다음 step입니다. 비워두면 ChoiceStep의 공통 NextStepId를 따릅니다."))
	FGameplayTag NextStepId;
};

USTRUCT(BlueprintType, meta = (ToolTip = "ChoiceStep에 표시할 질문과 단층 선택지 목록입니다."))
struct MAVERICK_API FMVInteractionChoiceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Choice", meta = (ToolTip = "선택지 위에 표시할 질문이나 안내 문구입니다."))
	FText PromptText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Choice", meta = (ToolTip = "플레이어가 고를 수 있는 선택지 목록입니다."))
	TArray<FMVInteractionChoiceEntryData> Choices;
};

/**
 * 상호작용 flow를 구성하는 instanced struct step 데이터의 공통 기반.
 *
 * Flow asset이나 InteractableComponent inline definition 안에 `FInstancedStruct`로 저장된다. 하위 struct는
 * 자기 step에 필요한 필드만 노출하므로, 에디터에서 타입별로 불필요한 데이터가 보이지 않는다.
 */
USTRUCT(BlueprintType, meta = (ToolTip = "모든 interaction step이 공유하는 기본 데이터입니다."))
struct MAVERICK_API FMVInteractionStepData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Step", meta = (Categories = "Interaction.Flow.Step", ToolTip = "이 step의 고유 ID입니다. 같은 flow 안에서 중복되면 안 됩니다."))
	FGameplayTag StepId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Step", meta = (Categories = "Interaction.Flow.Step", ToolTip = "이 step이 끝난 뒤 자동으로 이동할 다음 step입니다. 비워두면 flow를 종료합니다."))
	FGameplayTag NextStepId;
};

USTRUCT(BlueprintType, meta = (DisplayName = "Interaction Route Step", ToolTip = "아무 UI나 이벤트 없이 NextStepId로 즉시 넘어가는 중계 step입니다."))
struct MAVERICK_API FMVInteractionRouteStepData : public FMVInteractionStepData
{
	GENERATED_BODY()
};

USTRUCT(BlueprintType, meta = (DisplayName = "Interaction Dialogue Step", ToolTip = "대화창을 표시하고 닫힌 뒤 다음 step으로 넘어갑니다."))
struct MAVERICK_API FMVInteractionDialogueStepData : public FMVInteractionStepData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Dialogue", meta = (ToolTip = "등록된 대화 ID로 대화창을 엽니다. 비워두면 DialogueText를 직접 표시합니다."))
	FName DialogueId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Dialogue", meta = (ToolTip = "DialogueId가 비어 있을 때 표시할 직접 입력 대사입니다."))
	FText DialogueText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Dialogue", meta = (ClampMin = "-1.0", ToolTip = "대사를 자동으로 닫을 시간입니다. -1이면 기본값이나 수동 닫기를 사용합니다."))
	float DialogueDuration = -1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Dialogue", meta = (ClampMin = "-1.0", ToolTip = "플레이어가 대사를 넘기기 전 최소로 기다려야 하는 시간입니다. -1이면 기본값을 사용합니다."))
	float DialogueMinimumSkipDelay = -1.0f;
};

USTRUCT(BlueprintType, meta = (DisplayName = "Interaction Action Step", ToolTip = "애니메이션이나 도메인 이벤트 command를 순서대로 실행합니다."))
struct MAVERICK_API FMVInteractionActionStepData : public FMVInteractionStepData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Command", meta = (BaseStruct = "/Script/Maverick.MVInteractionCommandData", ExcludeBaseStruct, ToolTip = "이 step에서 순서대로 실행할 command 목록입니다."))
	TArray<FInstancedStruct> Commands;
};

USTRUCT(BlueprintType, meta = (DisplayName = "Interaction Warning Popup Step", ToolTip = "짧은 경고 팝업을 표시하고 닫힌 뒤 다음 step으로 넘어갑니다."))
struct MAVERICK_API FMVInteractionWarningPopupStepData : public FMVInteractionStepData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Warning", meta = (ToolTip = "등록된 메시지 ID로 팝업을 엽니다. 비워두면 WarningMessageText를 직접 표시합니다."))
	FName WarningMessageId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Warning", meta = (ToolTip = "WarningMessageId가 비어 있을 때 표시할 직접 입력 문구입니다."))
	FText WarningMessageText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Warning", meta = (ClampMin = "0.0", ToolTip = "팝업을 표시할 시간입니다."))
	float WarningDuration = 2.0f;
};

USTRUCT(BlueprintType, meta = (DisplayName = "Interaction Selection Step", ToolTip = "하위 메뉴나 여러 패널을 가진 선택 화면을 표시합니다."))
struct MAVERICK_API FMVInteractionSelectionStepData : public FMVInteractionStepData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Menu", meta = (ToolTip = "표시할 메뉴 화면과 항목 데이터입니다."))
	FMVInteractionMenuData MenuData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Transition", meta = (ToolTip = "특정 메뉴 EntryId를 선택했을 때 이동할 step 목록입니다."))
	TArray<FMVInteractionStepTransition> Transitions;
};

USTRUCT(BlueprintType, meta = (DisplayName = "Interaction Menu Step", ToolTip = "상호작용 메뉴를 열고 선택한 항목에 따라 다음 step으로 분기합니다."))
struct MAVERICK_API FMVInteractionMenuStepData : public FMVInteractionSelectionStepData
{
	GENERATED_BODY()
};

USTRUCT(BlueprintType, meta = (DisplayName = "Interaction Choice Step", ToolTip = "대화 중 잠깐 뜨는 단층 선택지를 표시하고 선택지별 다음 step으로 분기합니다."))
struct MAVERICK_API FMVInteractionChoiceStepData : public FMVInteractionStepData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Choice", meta = (ToolTip = "표시할 질문과 선택지 목록입니다."))
	FMVInteractionChoiceData ChoiceData;
};

USTRUCT(BlueprintType, meta = (DisplayName = "Interaction Window Step", ToolTip = "인벤토리나 스킬트리 같은 별도 window를 열고 닫힌 뒤 다음 step으로 넘어갑니다."))
struct MAVERICK_API FMVInteractionWindowStepData : public FMVInteractionStepData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Window", meta = (ToolTip = "열어야 할 CommonUI window 클래스입니다."))
	TSubclassOf<UMVWindowBase> WindowClass;
};
