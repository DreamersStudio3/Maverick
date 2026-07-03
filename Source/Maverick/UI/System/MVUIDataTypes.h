#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Menu", meta = (Categories = "Interaction.Menu.Page", AdvancedDisplay, ToolTip = "자동 보정되는 부모 페이지 ID입니다. 일반 데이터 제작에서는 직접 수정하지 않고 Entries 또는 SubMenus 페이지 안에 항목을 추가합니다."))
	FGameplayTag ParentMenuId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Menu", meta = (ToolTip = "화면에 표시할 항목 문구입니다."))
	FText Label;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Menu", meta = (RowType = "/Script/Maverick.MVActionRow", ToolTip = "선택 시 외부 로직에 전달할 action DataTable row입니다. MVActionRow 계열 테이블만 선택합니다."))
	FDataTableRowHandle ActionRow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Menu", meta = (ToolTip = "false면 항목은 표시되지만 선택할 수 없습니다."))
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Menu", meta = (Categories = "Interaction.Menu.Page", ToolTip = "선택하면 이동할 하위 메뉴 페이지 ID입니다. 비워두면 하위 메뉴로 이동하지 않습니다."))
	FGameplayTag SubMenuId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Menu", meta = (ToolTip = "선택하면 추가로 열어야 할 CommonUI window 클래스입니다."))
	TSubclassOf<UMVWindowBase> WindowClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Menu", meta = (ToolTip = "항목 실행 후 메뉴를 닫을지 여부입니다. 하위 메뉴 이동 항목은 보통 false입니다."))
	bool bCloseMenuOnExecute = true;

	FName RuntimeActionId = NAME_None;
	bool bInternalBackEntry = false;

	bool HasActionRow() const
	{
		return ActionRow.DataTable && !ActionRow.RowName.IsNone();
	}

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

USTRUCT(BlueprintType, meta = (ToolTip = "MenuStep 안에서 Entry.SubMenuId가 가리키는 하위 메뉴 페이지입니다."))
struct FMVInteractionMenuPageData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Menu", meta = (Categories = "Interaction.Menu.Page", ToolTip = "이 하위 메뉴 페이지의 고유 ID입니다. Entry의 SubMenuId와 같은 태그를 넣습니다."))
	FGameplayTag MenuId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Menu", meta = (ToolTip = "하위 메뉴에 들어갔을 때 상단에 표시할 제목입니다. 비워두면 루트 메뉴 제목을 사용합니다."))
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Menu", meta = (ToolTip = "이 하위 메뉴 페이지에 표시할 항목 목록입니다."))
	TArray<FMVMenuEntryData> Entries;
};

USTRUCT(BlueprintType, meta = (ToolTip = "MenuStep과 SelectionStep에서 표시할 메뉴 페이지와 항목 목록입니다."))
struct FMVInteractionMenuData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Menu", meta = (ToolTip = "메뉴 상단에 표시할 제목입니다."))
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Menu", meta = (Categories = "Interaction.Menu.Page", ToolTip = "처음 열릴 메뉴 페이지 ID입니다. 비워두면 root 페이지를 사용합니다."))
	FGameplayTag RootMenuId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Menu", meta = (ToolTip = "처음 메뉴 페이지에 표시할 항목 목록입니다. 하위 메뉴로 들어가려면 Entry의 SubMenuId를 설정하고 SubMenus에 같은 MenuId의 페이지를 추가합니다."))
	TArray<FMVMenuEntryData> Entries;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|UI|Menu", meta = (ToolTip = "Entry.SubMenuId로 연결되는 하위 메뉴 페이지 목록입니다. 각 페이지의 MenuId가 SubMenuId와 일치해야 합니다."))
	TArray<FMVInteractionMenuPageData> SubMenus;

	void NormalizeEntryParentMenuIds()
	{
		if (RootMenuId.IsValid())
		{
			for (FMVMenuEntryData& Entry : Entries)
			{
				Entry.ParentMenuId = RootMenuId;
			}
		}

		for (FMVInteractionMenuPageData& SubMenu : SubMenus)
		{
			if (!SubMenu.MenuId.IsValid())
			{
				continue;
			}

			for (FMVMenuEntryData& Entry : SubMenu.Entries)
			{
				Entry.ParentMenuId = SubMenu.MenuId;
			}
		}
	}
};
