#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "UObject/Object.h"
#include "UI/System/MVUIDataTypes.h"
#include "MVInteractionTypes.generated.h"

class UMVWindowBase;

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
 * 상호작용 flow를 구성하는 instanced step 데이터의 공통 기반.
 *
 * Flow asset이나 InteractableComponent inline definition 안에 직접 생성되는 polymorphic UObject다.
 * 하위 클래스는 자기 step에 필요한 필드만 노출하므로, 에디터에서 타입별로 불필요한 데이터가 보이지 않는다.
 */
UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class MAVERICK_API UMVInteractionStepData : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Step")
	FName StepId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Step")
	FName NextStepId = NAME_None;
};

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, meta = (DisplayName = "Interaction Route Step"))
class MAVERICK_API UMVInteractionRouteStepData : public UMVInteractionStepData
{
	GENERATED_BODY()
};

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, meta = (DisplayName = "Interaction Dialogue Step"))
class MAVERICK_API UMVInteractionDialogueStepData : public UMVInteractionStepData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Dialogue")
	FName DialogueId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Dialogue")
	FText DialogueText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Dialogue", meta = (ClampMin = "-1.0"))
	float DialogueDuration = -1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Dialogue", meta = (ClampMin = "-1.0"))
	float DialogueMinimumSkipDelay = -1.0f;
};

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, meta = (DisplayName = "Interaction Action Step"))
class MAVERICK_API UMVInteractionActionStepData : public UMVInteractionStepData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Action")
	FName ActionName = NAME_None;
};

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, meta = (DisplayName = "Interaction Warning Popup Step"))
class MAVERICK_API UMVInteractionWarningPopupStepData : public UMVInteractionStepData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Warning")
	FName WarningMessageId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Warning")
	FText WarningMessageText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Warning", meta = (ClampMin = "0.0"))
	float WarningDuration = 2.0f;
};

UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class MAVERICK_API UMVInteractionSelectionStepData : public UMVInteractionStepData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Menu")
	FMVInteractionMenuData MenuData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Transition")
	TArray<FMVInteractionStepTransition> Transitions;
};

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, meta = (DisplayName = "Interaction Menu Step"))
class MAVERICK_API UMVInteractionMenuStepData : public UMVInteractionSelectionStepData
{
	GENERATED_BODY()
};

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, meta = (DisplayName = "Interaction Choice Step"))
class MAVERICK_API UMVInteractionChoiceStepData : public UMVInteractionSelectionStepData
{
	GENERATED_BODY()
};

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, meta = (DisplayName = "Interaction Window Step"))
class MAVERICK_API UMVInteractionWindowStepData : public UMVInteractionStepData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Window")
	TSubclassOf<UMVWindowBase> WindowClass;
};
