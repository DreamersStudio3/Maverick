#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "MVUIDataTypes.generated.h"

USTRUCT(BlueprintType)
struct FMVInteractionPromptData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Interaction")
	FText PromptText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Interaction")
	FKey InputKey = EKeys::Invalid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Interaction", meta = (ClampMin = "0"))
	int32 CandidateIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Interaction", meta = (ClampMin = "0"))
	int32 CandidateCount = 0;
};

USTRUCT(BlueprintType)
struct FMVPopupMessageData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Message")
	FName MessageId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Message")
	FText MessageText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Message", meta = (ClampMin = "0.0"))
	float Duration = 2.0f;
};

USTRUCT(BlueprintType)
struct FMVMenuEntryData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Menu")
	FText Label;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Menu")
	FName ActionName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Menu")
	bool bEnabled = true;
};
