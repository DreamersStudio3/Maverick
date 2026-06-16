#include "UI/System/MVCommonUIInputData.h"

#include "Engine/DataTable.h"
#include "UObject/ConstructorHelpers.h"

UMVCommonUIInputData::UMVCommonUIInputData()
{
	static ConstructorHelpers::FObjectFinder<UDataTable> GenericInputActionTable(
		TEXT("/CommonUI/GenericInputActionDataTable.GenericInputActionDataTable"));

	if (GenericInputActionTable.Succeeded())
	{
		DefaultClickAction.DataTable = GenericInputActionTable.Object;
		DefaultClickAction.RowName = TEXT("GenericAccept");

		DefaultBackAction.DataTable = GenericInputActionTable.Object;
		DefaultBackAction.RowName = TEXT("GenericBack");
	}
}
