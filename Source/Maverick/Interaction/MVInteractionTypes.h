#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"
#include "Templates/SubclassOf.h"
#include "UI/System/MVUIDataTypes.h"
#include "MVInteractionTypes.generated.h"

class UMVWindowBase;

USTRUCT(BlueprintType)
struct MAVERICK_API FMVInteractionStepTransition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Transition", meta = (Categories = "Interaction.Menu.Entry"))
	FGameplayTag TriggerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Transition", meta = (Categories = "Interaction.Flow.Step"))
	FGameplayTag NextStepId;
};

USTRUCT(BlueprintType)
struct MAVERICK_API FMVInteractionChoiceEntryData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Choice", meta = (Categories = "Interaction.Choice.Entry"))
	FGameplayTag ChoiceId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Choice")
	FText Label;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Choice")
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Choice", meta = (Categories = "Interaction.Flow.Step"))
	FGameplayTag NextStepId;
};

USTRUCT(BlueprintType)
struct MAVERICK_API FMVInteractionChoiceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Choice")
	FText PromptText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Choice")
	TArray<FMVInteractionChoiceEntryData> Choices;
};

/**
 * 상호작용 flow를 구성하는 instanced struct step 데이터의 공통 기반.
 *
 * Flow asset이나 InteractableComponent inline definition 안에 `FInstancedStruct`로 저장된다. 하위 struct는
 * 자기 step에 필요한 필드만 노출하므로, 에디터에서 타입별로 불필요한 데이터가 보이지 않는다.
 */
USTRUCT(BlueprintType)
struct MAVERICK_API FMVInteractionStepData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Step", meta = (Categories = "Interaction.Flow.Step"))
	FGameplayTag StepId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Step", meta = (Categories = "Interaction.Flow.Step"))
	FGameplayTag NextStepId;
};

USTRUCT(BlueprintType, meta = (DisplayName = "Interaction Route Step"))
struct MAVERICK_API FMVInteractionRouteStepData : public FMVInteractionStepData
{
	GENERATED_BODY()
};

USTRUCT(BlueprintType, meta = (DisplayName = "Interaction Dialogue Step"))
struct MAVERICK_API FMVInteractionDialogueStepData : public FMVInteractionStepData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Dialogue")
	FName DialogueId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Dialogue")
	FText DialogueText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Dialogue", meta = (ClampMin = "-1.0"))
	float DialogueDuration = -1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Dialogue", meta = (ClampMin = "-1.0"))
	float DialogueMinimumSkipDelay = -1.0f;
};

USTRUCT(BlueprintType, meta = (DisplayName = "Interaction Action Step"))
struct MAVERICK_API FMVInteractionActionStepData : public FMVInteractionStepData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Action", meta = (RowType = "/Script/Maverick.MVActionRow"))
	FDataTableRowHandle ActionRow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Action")
	FName StartSection = NAME_None;
};

USTRUCT(BlueprintType, meta = (DisplayName = "Interaction Warning Popup Step"))
struct MAVERICK_API FMVInteractionWarningPopupStepData : public FMVInteractionStepData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Warning")
	FName WarningMessageId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Warning")
	FText WarningMessageText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Warning", meta = (ClampMin = "0.0"))
	float WarningDuration = 2.0f;
};

USTRUCT(BlueprintType, meta = (DisplayName = "Interaction Selection Step"))
struct MAVERICK_API FMVInteractionSelectionStepData : public FMVInteractionStepData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Menu")
	FMVInteractionMenuData MenuData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Transition")
	TArray<FMVInteractionStepTransition> Transitions;
};

USTRUCT(BlueprintType, meta = (DisplayName = "Interaction Menu Step"))
struct MAVERICK_API FMVInteractionMenuStepData : public FMVInteractionSelectionStepData
{
	GENERATED_BODY()
};

USTRUCT(BlueprintType, meta = (DisplayName = "Interaction Choice Step"))
struct MAVERICK_API FMVInteractionChoiceStepData : public FMVInteractionStepData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Choice")
	FMVInteractionChoiceData ChoiceData;
};

USTRUCT(BlueprintType, meta = (DisplayName = "Interaction Window Step"))
struct MAVERICK_API FMVInteractionWindowStepData : public FMVInteractionStepData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Window")
	TSubclassOf<UMVWindowBase> WindowClass;
};
