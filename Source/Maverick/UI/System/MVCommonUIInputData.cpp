#include "UI/System/MVCommonUIInputData.h"

#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "CommonUITypes.h"
#include "UObject/ConstructorHelpers.h"

UMVCommonUIInputData::UMVCommonUIInputData()
{
	static ConstructorHelpers::FObjectFinder<UDataTable> GenericInputActionTable(
		TEXT("/CommonUI/GenericInputActionDataTable.GenericInputActionDataTable"));

	if (GenericInputActionTable.Succeeded())
	{
		DefaultClickAction.DataTable = GenericInputActionTable.Object;
		DefaultClickAction.RowName = TEXT("GenericForward");

		DefaultBackAction.DataTable = GenericInputActionTable.Object;
		DefaultBackAction.RowName = TEXT("GenericBack");
	}
}

FDataTableRowHandle UMVCommonUIInputData::GetBackActionForWorld(const UWorld* World)
{
#if WITH_EDITOR
	if (World && World->WorldType == EWorldType::PIE)
	{
		if (!PIEBackActionTable)
		{
			const FCommonInputActionDataBase* Default = DefaultBackAction.GetRow<FCommonInputActionDataBase>(TEXT("MVPIEBackInput"));
			if (!Default)
			{
				return DefaultBackAction;
			}
			struct FMVPIEBackActionData : FCommonInputActionDataBase
			{
				explicit FMVPIEBackActionData(const FCommonInputActionDataBase& Original) : FCommonInputActionDataBase(Original)
				{
					KeyboardInputTypeInfo = FCommonInputTypeInfo();
					KeyboardInputTypeInfo.SetKey(EKeys::BackSpace);
				}
			};
			PIEBackActionTable = NewObject<UDataTable>(this, NAME_None, RF_Transient);
			PIEBackActionTable->RowStruct = FCommonInputActionDataBase::StaticStruct();
			PIEBackActionTable->AddRow(TEXT("PIEBack"), FMVPIEBackActionData(*Default));
		}
		FDataTableRowHandle Action;
		Action.DataTable = PIEBackActionTable;
		Action.RowName = TEXT("PIEBack");
		return Action;
	}
#endif
	return DefaultBackAction;
}
