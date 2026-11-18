#include "Weapon/Skill/MVSkillExecutionData.h"
#include "Combat/MVAbilityBase.h"
#include "Public/Tables/MVSkillDataTableColumn.h"

bool MVSkillExecutionData::ValidateDefinition(const FDataTableRowHandle& Start, TArray<FName>& OutRows, const bool bAllowPrototype)
{
	OutRows.Reset();
	if (!Start.DataTable || Start.RowName.IsNone() || !Start.DataTable->GetRowStruct()
		|| !Start.DataTable->GetRowStruct()->IsChildOf(FMVSkillDataTableColumn::StaticStruct())) return false;
	TArray<FName> Rows;
	FName Name = Start.RowName;
	while (!Name.IsNone())
	{
		if (Rows.Num() >= 100 || Rows.Contains(Name)) return false;
		const auto* Row = Start.DataTable->FindRow<FMVSkillDataTableColumn>(Name, TEXT("SkillExecution.Validate"), false);
		if (!Row) return false;
		if (Row->ImplementationState == EMVSkillImplementationState::Prototype)
		{
			if (!bAllowPrototype || !Rows.IsEmpty()) return false;
			OutRows.Add(Name);
			return true;
		}
		if (Row->ImplementationState != EMVSkillImplementationState::Ready || !Row->bEnabled || !Row->AbilityReference
			|| Row->AbilityReference->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists)) return false;
		Rows.Add(Name);
		if (Row->bIsChained && Row->NextChainName.IsNone()) return false;
		Name = Row->bIsChained ? Row->NextChainName : NAME_None;
	}
	OutRows = MoveTemp(Rows);
	return !OutRows.IsEmpty();
}
