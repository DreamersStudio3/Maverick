#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Tables/MVTableTypes.h"
#include "MVDialogueTableTypes.generated.h"

class USoundBase;

USTRUCT(BlueprintType, meta = (MVTable = "Dialogue"))
struct MAVERICK_API FMVDialogueRow : public FMVTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|UI|Dialogue", meta = (Categories = "Interaction.Dialogue", ToolTip = "대화 row의 기획 식별 태그입니다. RowName은 RowHandle 선택용이고, 이 값은 대화 참조/분기/툴 표시용 ID로 사용합니다."))
	FGameplayTag DialogueId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|UI|Dialogue")
	FText SpeakerName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|UI|Dialogue", meta = (MultiLine = "true"))
	FText DialogueText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|UI|Dialogue", meta = (ClampMin = "0.0"))
	float DisplayDuration = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|UI|Dialogue", meta = (ClampMin = "0.0"))
	float MinimumSkipDelay = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|UI|Dialogue", meta = (Categories = "Interaction.Dialogue", ToolTip = "테이블 기반 대화 체인에서 다음 대화를 가리킬 때 사용할 선택적 태그입니다. InteractionFlow의 다음 진행은 Step NextStepId가 담당합니다."))
	FGameplayTag NextDialogueId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|UI|Dialogue")
	TSoftObjectPtr<USoundBase> VoiceSound;
};
