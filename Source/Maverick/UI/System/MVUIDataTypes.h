#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InputCoreTypes.h"
#include "Templates/SubclassOf.h"
#include "MVUIDataTypes.generated.h"

class UMVWindowBase;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Menu", meta = (Categories = "Interaction.Menu.Entry"))
	FGameplayTag EntryId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Menu", meta = (Categories = "Interaction.Menu.Page"))
	FGameplayTag ParentMenuId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Menu")
	FText Label;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Menu")
	FName ActionName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Menu")
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Menu", meta = (Categories = "Interaction.Menu.Page"))
	FGameplayTag SubMenuId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Menu")
	TSubclassOf<UMVWindowBase> WindowClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Menu")
	bool bCloseMenuOnExecute = true;
};

USTRUCT(BlueprintType)
struct FMVInteractionMenuData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Menu")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Menu", meta = (Categories = "Interaction.Menu.Page"))
	FGameplayTag RootMenuId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Menu")
	TArray<FMVMenuEntryData> Entries;
};
