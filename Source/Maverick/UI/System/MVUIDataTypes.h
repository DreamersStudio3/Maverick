#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "InputCoreTypes.h"
#include "Templates/SubclassOf.h"
#include "MVUIDataTypes.generated.h"

class UMVWindowBase;
class UMVInteractionMenuPageData;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Menu", meta = (ToolTip = "화면에 표시할 항목 문구입니다."))
	FText Label;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Menu", meta = (RowType = "/Script/Maverick.MVActionRow", ToolTip = "선택 시 외부 로직에 전달할 action DataTable row입니다. MVActionRow 계열 테이블만 선택합니다."))
	FDataTableRowHandle ActionRow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Menu", meta = (ToolTip = "false면 항목은 표시되지만 선택할 수 없습니다."))
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Menu", meta = (ToolTip = "선택하면 추가로 열어야 할 CommonUI window 클래스입니다."))
	TSubclassOf<UMVWindowBase> WindowClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Menu", meta = (ToolTip = "항목 실행 후 메뉴를 닫을지 여부입니다. SubMenu가 있으면 이 값과 관계없이 하위 메뉴로 이동합니다."))
	bool bCloseMenuOnExecute = true;

	UPROPERTY(EditAnywhere, Instanced, BlueprintReadWrite, Category = "Maverick|UI|Menu|SubMenu", meta = (ToolTip = "이 항목에 1:1로 귀속된 하위 메뉴입니다. 설정하면 선택 시 이 메뉴로 들어갑니다."))
	TObjectPtr<UMVInteractionMenuPageData> SubMenu;

	FName RuntimeActionId = NAME_None;
	bool bInternalBackEntry = false;

	bool HasActionRow() const
	{
		return ActionRow.DataTable && !ActionRow.RowName.IsNone();
	}

	bool HasSubMenu() const;

	FName ResolveSelectionName() const
	{
		if (!RuntimeActionId.IsNone())
		{
			return RuntimeActionId;
		}

		if (HasActionRow())
		{
			return ActionRow.RowName;
		}

		return EntryId.GetTagName();
	}
};

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, CollapseCategories)
class MAVERICK_API UMVInteractionMenuPageData : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Menu", meta = (ToolTip = "하위 메뉴에 들어갔을 때 상단에 표시할 제목입니다. 비워두면 이전 메뉴 제목을 유지합니다."))
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Menu", meta = (ToolTip = "이 하위 메뉴 페이지에 표시할 항목 목록입니다."))
	TArray<FMVMenuEntryData> Entries;
};

inline bool FMVMenuEntryData::HasSubMenu() const
{
	return SubMenu != nullptr;
}

USTRUCT(BlueprintType, meta = (ToolTip = "MenuStep과 SelectionStep에서 표시할 메뉴 페이지와 항목 목록입니다."))
struct FMVInteractionMenuData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Menu", meta = (ToolTip = "메뉴 상단에 표시할 제목입니다."))
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Menu", meta = (Categories = "Interaction.Menu.Page", ToolTip = "처음 열릴 메뉴 페이지 ID입니다. 비워두면 root 페이지를 사용합니다."))
	FGameplayTag RootMenuId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Menu", meta = (ToolTip = "처음 메뉴 페이지에 표시할 항목 목록입니다. 하위 메뉴는 각 Entry의 SubMenu에 직접 추가합니다."))
	TArray<FMVMenuEntryData> Entries;
};
