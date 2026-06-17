#pragma once

#include "CoreMinimal.h"
#include "Tables/MVTableTypes.h"
#include "MVDialogueTableTypes.generated.h"

class USoundBase;

USTRUCT(BlueprintType, meta = (MVTable = "Dialogue"))
struct MAVERICK_API FMVDialogueRow : public FMVTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|UI|Dialogue")
	FString DialogueId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|UI|Dialogue")
	FText SpeakerName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|UI|Dialogue", meta = (MultiLine = "true"))
	FText DialogueText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|UI|Dialogue", meta = (ClampMin = "0.0"))
	float DisplayDuration = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|UI|Dialogue", meta = (ClampMin = "0.0"))
	float MinimumSkipDelay = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|UI|Dialogue")
	FString NextDialogueId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|UI|Dialogue")
	TSoftObjectPtr<USoundBase> VoiceSound;
};
