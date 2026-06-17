#pragma once

#include "CoreMinimal.h"
#include "Tables/MVTableTypes.h"
#include "MVUIMessageTableTypes.generated.h"

USTRUCT(BlueprintType, meta = (MVTable = "UIMessage"))
struct MAVERICK_API FMVUIMessageRow : public FMVTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|UI|Message")
	FString MessageId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|UI|Message")
	FText MessageText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|UI|Message", meta = (ClampMin = "0.0"))
	float DefaultDuration = 2.0f;
};
