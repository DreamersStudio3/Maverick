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

USTRUCT(BlueprintType, meta = (ToolTip = "상호작용 메뉴 안에 표시되는 버튼 또는 항목입니다."))
struct FMVMenuEntryData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Menu", meta = (Categories = "Interaction.Menu.Entry", ToolTip = "메뉴 항목의 고유 ID입니다. 버튼이나 선택 가능한 항목을 구분합니다."))
	FGameplayTag EntryId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Menu", meta = (Categories = "Interaction.Menu.Page", ToolTip = "이 항목이 표시될 메뉴 페이지 ID입니다. 비워두면 root 페이지에 표시됩니다."))
	FGameplayTag ParentMenuId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Menu", meta = (ToolTip = "화면에 표시할 항목 문구입니다."))
	FText Label;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Menu", meta = (ToolTip = "선택 시 외부 로직에 전달할 액션 이름입니다. 동적 액션이 아니면 비워두고 EntryId를 사용합니다."))
	FName ActionName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Menu", meta = (ToolTip = "false면 항목은 표시되지만 선택할 수 없습니다."))
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Menu", meta = (Categories = "Interaction.Menu.Page", ToolTip = "선택하면 이동할 하위 메뉴 페이지 ID입니다. 비워두면 하위 메뉴로 이동하지 않습니다."))
	FGameplayTag SubMenuId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Menu", meta = (ToolTip = "선택하면 추가로 열어야 할 CommonUI window 클래스입니다."))
	TSubclassOf<UMVWindowBase> WindowClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Menu", meta = (ToolTip = "항목 실행 후 메뉴를 닫을지 여부입니다. 하위 메뉴 이동 항목은 보통 false입니다."))
	bool bCloseMenuOnExecute = true;
};

USTRUCT(BlueprintType, meta = (ToolTip = "MenuStep과 SelectionStep에서 표시할 메뉴 페이지와 항목 목록입니다."))
struct FMVInteractionMenuData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Menu", meta = (ToolTip = "메뉴 상단에 표시할 제목입니다."))
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Menu", meta = (Categories = "Interaction.Menu.Page", ToolTip = "처음 열릴 메뉴 페이지 ID입니다. 비워두면 root 페이지를 사용합니다."))
	FGameplayTag RootMenuId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Menu", meta = (ToolTip = "메뉴에 표시할 항목 목록입니다. ParentMenuId와 SubMenuId로 하위 페이지를 구성합니다."))
	TArray<FMVMenuEntryData> Entries;
};
