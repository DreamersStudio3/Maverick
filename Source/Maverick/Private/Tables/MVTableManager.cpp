#include "Tables/MVTableManager.h"

#include "Engine/DataTable.h"
#include "Engine/Engine.h"
#include "JsonObjectConverter.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "UObject/TextProperty.h"
#include "UObject/UnrealType.h"

DEFINE_LOG_CATEGORY_STATIC(LogMVTableManager, Log, All);

namespace
{
	const TCHAR* ManifestObjectPath = TEXT("/Game/Table/DT_MVTableManifest.DT_MVTableManifest");

	bool ExportPropertyToString(const FProperty* Property, const void* RowData, FString& OutValue)
	{
		if (!Property || !RowData)
		{
			return false;
		}

		const void* ValuePtr = Property->ContainerPtrToValuePtr<void>(RowData);

		if (const FStrProperty* StringProperty = CastField<FStrProperty>(Property))
		{
			OutValue = StringProperty->GetPropertyValue(ValuePtr);
			return true;
		}
		if (const FNameProperty* NameProperty = CastField<FNameProperty>(Property))
		{
			OutValue = NameProperty->GetPropertyValue(ValuePtr).ToString();
			return true;
		}
		if (const FTextProperty* TextProperty = CastField<FTextProperty>(Property))
		{
			OutValue = TextProperty->GetPropertyValue(ValuePtr).ToString();
			return true;
		}
		if (const FBoolProperty* BoolProperty = CastField<FBoolProperty>(Property))
		{
			OutValue = BoolProperty->GetPropertyValue(ValuePtr) ? TEXT("true") : TEXT("false");
			return true;
		}
		if (const FNumericProperty* NumericProperty = CastField<FNumericProperty>(Property))
		{
			if (NumericProperty->IsInteger())
			{
				OutValue = FString::Printf(TEXT("%lld"), NumericProperty->GetSignedIntPropertyValue(ValuePtr));
			}
			else
			{
				OutValue = FString::SanitizeFloat(NumericProperty->GetFloatingPointPropertyValue(ValuePtr));
			}
			return true;
		}
		if (const FEnumProperty* EnumProperty = CastField<FEnumProperty>(Property))
		{
			const int64 EnumValue = EnumProperty->GetUnderlyingProperty()->GetSignedIntPropertyValue(ValuePtr);
			if (const UEnum* Enum = EnumProperty->GetEnum())
			{
				OutValue = Enum->GetNameStringByValue(EnumValue);
			}
			else
			{
				OutValue = FString::Printf(TEXT("%lld"), EnumValue);
			}
			return true;
		}

		Property->ExportTextItem_Direct(OutValue, ValuePtr, nullptr, nullptr, PPF_None);
		return true;
	}

	bool TryParseBoolString(const FString& Text, bool& OutValue)
	{
		if (Text.Equals(TEXT("true"), ESearchCase::IgnoreCase) || Text == TEXT("1"))
		{
			OutValue = true;
			return true;
		}
		if (Text.Equals(TEXT("false"), ESearchCase::IgnoreCase) || Text == TEXT("0"))
		{
			OutValue = false;
			return true;
		}
		return false;
	}

	bool JsonValueToString(const TSharedPtr<FJsonValue>& Value, FString& OutValue)
	{
		if (!Value.IsValid())
		{
			return false;
		}

		switch (Value->Type)
		{
		case EJson::String:
			OutValue = Value->AsString();
			return true;
		case EJson::Number:
		{
			const double Number = Value->AsNumber();
			const int64 IntegerValue = static_cast<int64>(Number);
			OutValue = FMath::IsNearlyEqual(Number, static_cast<double>(IntegerValue))
				? FString::Printf(TEXT("%lld"), IntegerValue)
				: FString::SanitizeFloat(Number);
			return true;
		}
		case EJson::Boolean:
			OutValue = Value->AsBool() ? TEXT("true") : TEXT("false");
			return true;
		default:
			return false;
		}
	}
}

void UMVTableManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadAllTables();
}

void UMVTableManager::Deinitialize()
{
	ManifestTable = nullptr;
	LoadedTables.Reset();
	ManifestRows.Reset();
	LastLoadErrors.Reset();
	bLoaded = false;
	Super::Deinitialize();
}

UMVTableManager* UMVTableManager::Get(const UObject* WorldContext)
{
	return GEngine ? GEngine->GetEngineSubsystem<UMVTableManager>() : nullptr;
}

void UMVTableManager::ReloadAllTables()
{
	LoadAllTables();
}

bool UMVTableManager::HasTable(FName TableName) const
{
	return LoadedTables.Contains(TableName);
}

bool UMVTableManager::HasRow(FName TableName, const FString& RowKey) const
{
	if (const UDataTable* DataTable = FindDataTable(TableName))
	{
		return DataTable->GetRowMap().Contains(FName(*RowKey));
	}
	return false;
}

TArray<FName> UMVTableManager::GetTableNames() const
{
	TArray<FName> Names;
	LoadedTables.GetKeys(Names);
	Names.Sort([](const FName& A, const FName& B)
	{
		return A.ToString() < B.ToString();
	});
	return Names;
}

TArray<FString> UMVTableManager::GetRowKeys(FName TableName) const
{
	TArray<FString> Keys;
	if (const UDataTable* DataTable = FindDataTable(TableName))
	{
		for (const TPair<FName, uint8*>& Pair : DataTable->GetRowMap())
		{
			Keys.Add(Pair.Key.ToString());
		}
		Keys.Sort();
	}
	return Keys;
}

const UDataTable* UMVTableManager::FindDataTable(FName TableName) const
{
	const TObjectPtr<UDataTable>* Found = LoadedTables.Find(TableName);
	return Found ? Found->Get() : nullptr;
}

const FMVTableManifestRow* UMVTableManager::FindManifestRow(FName TableName) const
{
	return ManifestRows.Find(TableName);
}

bool UMVTableManager::GetString(FName TableName, const FString& RowKey, const FString& FieldName, FString& OutValue) const
{
	TSharedPtr<FJsonValue> GenericValue;
	if (TryGetGenericField(TableName, RowKey, FieldName, GenericValue))
	{
		return JsonValueToString(GenericValue, OutValue);
	}

	const uint8* RowData = nullptr;
	const FProperty* Property = FindProperty(TableName, RowKey, FieldName, RowData);
	return ExportPropertyToString(Property, RowData, OutValue);
}

bool UMVTableManager::GetInt(FName TableName, const FString& RowKey, const FString& FieldName, int32& OutValue) const
{
	TSharedPtr<FJsonValue> GenericValue;
	if (TryGetGenericField(TableName, RowKey, FieldName, GenericValue))
	{
		if (GenericValue->Type == EJson::Number)
		{
			const double Number = GenericValue->AsNumber();
			const int32 Rounded = FMath::RoundToInt(Number);
			if (FMath::IsNearlyEqual(Number, static_cast<double>(Rounded)))
			{
				OutValue = Rounded;
				return true;
			}
			return false;
		}
		if (GenericValue->Type == EJson::String)
		{
			return LexTryParseString(OutValue, *GenericValue->AsString());
		}
		return false;
	}

	const uint8* RowData = nullptr;
	const FProperty* Property = FindProperty(TableName, RowKey, FieldName, RowData);
	if (!Property || !RowData)
	{
		return false;
	}

	const void* ValuePtr = Property->ContainerPtrToValuePtr<void>(RowData);
	if (const FNumericProperty* NumericProperty = CastField<FNumericProperty>(Property))
	{
		if (NumericProperty->IsInteger())
		{
			OutValue = static_cast<int32>(NumericProperty->GetSignedIntPropertyValue(ValuePtr));
			return true;
		}

		const double Value = NumericProperty->GetFloatingPointPropertyValue(ValuePtr);
		const int32 Rounded = FMath::RoundToInt(Value);
		if (FMath::IsNearlyEqual(Value, static_cast<double>(Rounded)))
		{
			OutValue = Rounded;
			return true;
		}
		return false;
	}

	FString StringValue;
	return ExportPropertyToString(Property, RowData, StringValue) && LexTryParseString(OutValue, *StringValue);
}

bool UMVTableManager::GetFloat(FName TableName, const FString& RowKey, const FString& FieldName, float& OutValue) const
{
	TSharedPtr<FJsonValue> GenericValue;
	if (TryGetGenericField(TableName, RowKey, FieldName, GenericValue))
	{
		if (GenericValue->Type == EJson::Number)
		{
			OutValue = static_cast<float>(GenericValue->AsNumber());
			return true;
		}
		if (GenericValue->Type == EJson::String)
		{
			return LexTryParseString(OutValue, *GenericValue->AsString());
		}
		return false;
	}

	const uint8* RowData = nullptr;
	const FProperty* Property = FindProperty(TableName, RowKey, FieldName, RowData);
	if (!Property || !RowData)
	{
		return false;
	}

	const void* ValuePtr = Property->ContainerPtrToValuePtr<void>(RowData);
	if (const FNumericProperty* NumericProperty = CastField<FNumericProperty>(Property))
	{
		OutValue = NumericProperty->IsInteger()
			? static_cast<float>(NumericProperty->GetSignedIntPropertyValue(ValuePtr))
			: static_cast<float>(NumericProperty->GetFloatingPointPropertyValue(ValuePtr));
		return true;
	}

	FString StringValue;
	return ExportPropertyToString(Property, RowData, StringValue) && LexTryParseString(OutValue, *StringValue);
}

bool UMVTableManager::GetBool(FName TableName, const FString& RowKey, const FString& FieldName, bool& OutValue) const
{
	TSharedPtr<FJsonValue> GenericValue;
	if (TryGetGenericField(TableName, RowKey, FieldName, GenericValue))
	{
		if (GenericValue->Type == EJson::Boolean)
		{
			OutValue = GenericValue->AsBool();
			return true;
		}
		if (GenericValue->Type == EJson::String)
		{
			return TryParseBoolString(GenericValue->AsString(), OutValue);
		}
		if (GenericValue->Type == EJson::Number)
		{
			const double Number = GenericValue->AsNumber();
			if (FMath::IsNearlyEqual(Number, 0.0))
			{
				OutValue = false;
				return true;
			}
			if (FMath::IsNearlyEqual(Number, 1.0))
			{
				OutValue = true;
				return true;
			}
		}
		return false;
	}

	const uint8* RowData = nullptr;
	const FProperty* Property = FindProperty(TableName, RowKey, FieldName, RowData);
	if (!Property || !RowData)
	{
		return false;
	}

	const void* ValuePtr = Property->ContainerPtrToValuePtr<void>(RowData);
	if (const FBoolProperty* BoolProperty = CastField<FBoolProperty>(Property))
	{
		OutValue = BoolProperty->GetPropertyValue(ValuePtr);
		return true;
	}

	FString StringValue;
	return ExportPropertyToString(Property, RowData, StringValue) && TryParseBoolString(StringValue, OutValue);
}

bool UMVTableManager::GetRowJson(FName TableName, const FString& RowKey, FString& OutValue) const
{
	const UScriptStruct* RowStruct = nullptr;
	const uint8* RowData = FindRawRow(TableName, RowKey, RowStruct);
	if (!RowData || !RowStruct)
	{
		return false;
	}

	if (RowStruct == FMVGenericTableRow::StaticStruct())
	{
		const FMVGenericTableRow* GenericRow = reinterpret_cast<const FMVGenericTableRow*>(RowData);
		OutValue = GenericRow->RowJson;
		return true;
	}

	return FJsonObjectConverter::UStructToJsonObjectString(RowStruct, RowData, OutValue, 0, CPF_Transient);
}

bool UMVTableManager::LoadAllTables()
{
	ManifestTable = nullptr;
	LoadedTables.Reset();
	ManifestRows.Reset();
	LastLoadErrors.Reset();

	ManifestTable = LoadObject<UDataTable>(nullptr, ManifestObjectPath);
	if (!ManifestTable)
	{
		AddLoadError(FString::Printf(
			TEXT("[MVTableManager] Table manifest not found: %s. Run MV.Table.GenerateDataTables in the editor."),
			ManifestObjectPath));
		bLoaded = false;
		return false;
	}

	if (!ManifestTable->GetRowStruct() || ManifestTable->GetRowStruct() != FMVTableManifestRow::StaticStruct())
	{
		AddLoadError(FString::Printf(TEXT("[MVTableManager] Invalid table manifest row struct: %s"), ManifestObjectPath));
		bLoaded = false;
		return false;
	}

	for (const TPair<FName, uint8*>& Pair : ManifestTable->GetRowMap())
	{
		const FMVTableManifestRow* ManifestRow = reinterpret_cast<const FMVTableManifestRow*>(Pair.Value);
		if (!ManifestRow || ManifestRow->TableName.IsNone() || ManifestRow->AssetPath.IsEmpty())
		{
			AddLoadError(FString::Printf(TEXT("[MVTableManager] Invalid manifest row: %s"), *Pair.Key.ToString()));
			continue;
		}

		UDataTable* DataTable = LoadObject<UDataTable>(nullptr, *ManifestRow->AssetPath);
		if (!DataTable)
		{
			AddLoadError(FString::Printf(
				TEXT("[MVTableManager] DataTable not found for '%s': %s"),
				*ManifestRow->TableName.ToString(),
				*ManifestRow->AssetPath));
			continue;
		}

		LoadedTables.Add(ManifestRow->TableName, DataTable);
		ManifestRows.Add(ManifestRow->TableName, *ManifestRow);
	}

	bLoaded = true;
	UE_LOG(LogMVTableManager, Log, TEXT("[MVTableManager] Loaded %d table(s)."), LoadedTables.Num());
	return LastLoadErrors.IsEmpty();
}

void UMVTableManager::AddLoadError(const FString& Message)
{
	LastLoadErrors.Add(Message);
	UE_LOG(LogMVTableManager, Warning, TEXT("%s"), *Message);
}

const uint8* UMVTableManager::FindRawRow(FName TableName, const FString& RowKey, const UScriptStruct*& OutRowStruct) const
{
	OutRowStruct = nullptr;

	const UDataTable* DataTable = FindDataTable(TableName);
	if (!DataTable || !DataTable->GetRowStruct())
	{
		return nullptr;
	}

	OutRowStruct = DataTable->GetRowStruct();
	const uint8* const* Row = DataTable->GetRowMap().Find(FName(*RowKey));
	return Row ? *Row : nullptr;
}

const FProperty* UMVTableManager::FindProperty(FName TableName, const FString& RowKey, const FString& FieldName, const uint8*& OutRowData) const
{
	OutRowData = nullptr;

	const UScriptStruct* RowStruct = nullptr;
	OutRowData = FindRawRow(TableName, RowKey, RowStruct);
	if (!OutRowData || !RowStruct)
	{
		return nullptr;
	}

	return RowStruct->FindPropertyByName(FName(*FieldName));
}

bool UMVTableManager::TryGetGenericField(FName TableName, const FString& RowKey, const FString& FieldName, TSharedPtr<FJsonValue>& OutValue) const
{
	const UScriptStruct* RowStruct = nullptr;
	const uint8* RowData = FindRawRow(TableName, RowKey, RowStruct);
	if (!RowData || RowStruct != FMVGenericTableRow::StaticStruct())
	{
		return false;
	}

	const FMVGenericTableRow* GenericRow = reinterpret_cast<const FMVGenericTableRow*>(RowData);
	if (!GenericRow || GenericRow->RowJson.IsEmpty())
	{
		return false;
	}

	TSharedPtr<FJsonObject> RowObject;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(GenericRow->RowJson);
	if (!FJsonSerializer::Deserialize(Reader, RowObject) || !RowObject.IsValid())
	{
		return false;
	}

	const TSharedPtr<FJsonValue>* Found = RowObject->Values.Find(FieldName);
	if (!Found || !Found->IsValid())
	{
		return false;
	}

	OutValue = *Found;
	return true;
}
