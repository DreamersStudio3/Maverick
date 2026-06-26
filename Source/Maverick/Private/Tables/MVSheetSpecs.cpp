#include "Tables/MVSheetSpecs.h"

#include "Tables/MVActionRowTableTypes.h"
#include "Tables/MVHitReactionActionTableTypes.h"
#include "Tables/MVTableTypes.h"

#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "UObject/UObjectIterator.h"

namespace
{
	const FName MetaTable = TEXT("MVTable");

	FString GetRecipePath()
	{
		return FPaths::ConvertRelativePathToFull(
			FPaths::ProjectDir() / TEXT("MaverickDesign") / TEXT("Json") / TEXT("SheetRecipe.json"));
	}

	TMap<FString, FString> LoadRecipe()
	{
		TMap<FString, FString> Out;

		const FString Path = GetRecipePath();
		FString Raw;
		if (!FFileHelper::LoadFileToString(Raw, *Path))
		{
			UE_LOG(LogTemp, Warning, TEXT("[MVSheetSpecs] Recipe not found: %s"), *Path);
			return Out;
		}

		TSharedPtr<FJsonObject> Root;
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Raw);
		if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("[MVSheetSpecs] Invalid recipe JSON: %s"), *Path);
			return Out;
		}

		for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : Root->Values)
		{
			FString KeyColumn;
			if (Pair.Value.IsValid() && Pair.Value->TryGetString(KeyColumn) && !KeyColumn.IsEmpty())
			{
				Out.Add(Pair.Key, KeyColumn);
			}
		}

		return Out;
	}

	bool& BuiltFlag()
	{
		static bool bBuilt = false;
		return bBuilt;
	}

	TMap<FString, FMVSheetSpec>& MutableSpecMap()
	{
		static TMap<FString, FMVSheetSpec> Map;
		return Map;
	}

	const TMap<FString, FMVSheetSpec>& GetSpecMap()
	{
		TMap<FString, FMVSheetSpec>& Map = MutableSpecMap();
		if (BuiltFlag())
		{
			return Map;
		}

		Map.Reset();

		const TMap<FString, FString> Recipe = LoadRecipe();
		UScriptStruct* Base = FMVTableRowBase::StaticStruct();

		TMap<FString, UScriptStruct*> TableToStruct;
		for (TObjectIterator<UScriptStruct> It; It; ++It)
		{
			UScriptStruct* Struct = *It;
			if (!Struct || Struct == Base || Struct == FMVGenericTableRow::StaticStruct())
			{
				continue;
			}
			if (!Struct->IsChildOf(Base) || !Struct->HasMetaData(MetaTable))
			{
				continue;
			}

			const FString TableName = Struct->GetMetaData(MetaTable);
			if (TableName.IsEmpty())
			{
				continue;
			}

			if (UScriptStruct** Existing = TableToStruct.Find(TableName))
			{
				UE_LOG(
					LogTemp,
					Warning,
					TEXT("[MVSheetSpecs] Duplicate MVTable '%s' on %s. Existing struct %s is kept."),
					*TableName,
					*Struct->GetName(),
					*(*Existing)->GetName());
				continue;
			}

			TableToStruct.Add(TableName, Struct);
		}

		for (const TPair<FString, FString>& Pair : Recipe)
		{
			const FString& TableName = Pair.Key;
			const FString& KeyColumnName = Pair.Value;

			if (UScriptStruct* const* TypedStruct = TableToStruct.Find(TableName))
			{
				Map.Add(TableName, FMVSheetSpec{ *TypedStruct, KeyColumnName, false });
			}
			else if (KeyColumnName == TEXT("RowName") && (TableName.StartsWith(TEXT("HR_")) || TableName.Contains(TEXT("_HR_"))))
			{
				Map.Add(TableName, FMVSheetSpec{ FMVHitReactionActionRow::StaticStruct(), KeyColumnName, false });
			}
			else
			{
				Map.Add(TableName, FMVSheetSpec{ FMVGenericTableRow::StaticStruct(), KeyColumnName, true });
				UE_LOG(
					LogTemp,
					Warning,
					TEXT("[MVSheetSpecs] '%s' has no USTRUCT meta=(MVTable=\"%s\"). Generic row fallback will be generated."),
					*TableName,
					*TableName);
			}
		}

		for (const TPair<FString, UScriptStruct*>& Pair : TableToStruct)
		{
			if (!Recipe.Contains(Pair.Key))
			{
				UE_LOG(
					LogTemp,
					Warning,
					TEXT("[MVSheetSpecs] Struct %s maps to '%s', but SheetRecipe has no key column entry."),
					*Pair.Value->GetName(),
					*Pair.Key);
			}
		}

		UE_LOG(LogTemp, Log, TEXT("[MVSheetSpecs] %d table spec(s) registered."), Map.Num());
		BuiltFlag() = true;
		return Map;
	}
}

const FMVSheetSpec* FMVSheetSpecs::Find(const FString& SheetName)
{
	return GetSpecMap().Find(SheetName);
}

TArray<FString> FMVSheetSpecs::GetAllSheetNames()
{
	TArray<FString> Out;
	GetSpecMap().GetKeys(Out);
	return Out;
}

void FMVSheetSpecs::Invalidate()
{
	BuiltFlag() = false;
	MutableSpecMap().Reset();
}
