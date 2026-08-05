#include "Tables/MVTableAssetGenerator.h"

#include "Tables/MVActionRowTableTypes.h"
#include "Tables/MVSheetSpecs.h"
#include "Tables/MVTableManager.h"
#include "Tables/MVTableTypes.h"

#if WITH_EDITOR
#include "AssetRegistry/AssetData.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Containers/StringConv.h"
#include "Engine/DataTable.h"
#include "HAL/FileManager.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformMisc.h"
#include "HAL/PlatformProcess.h"
#include "JsonObjectConverter.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Misc/SecureHash.h"
#include "Modules/ModuleManager.h"
#include "ObjectTools.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "UObject/MetaData.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"
#endif

namespace
{
	const TCHAR* GeneratedTablesPackageRoot = TEXT("/Game/Table");
	const TCHAR* ManifestAssetName = TEXT("DT_MVTableManifest");
	const TCHAR* ManifestPackagePath = TEXT("/Game/Table/DT_MVTableManifest");
	const TCHAR* GeneratedTableHashMetadataKey = TEXT("MVTableAssetGenerator.SourceHash");
	const TCHAR* DirectManagedTablePackageRoots[] =
	{
		TEXT("/Game/Table/Attack"),
		TEXT("/Game/Table/Death"),
		TEXT("/Game/Table/Dodge"),
		TEXT("/Game/Table/Groggy"),
		TEXT("/Game/Table/HitReaction"),
		TEXT("/Game/Table/Props"),
		TEXT("/Game/Table/Sprint"),
		TEXT("/Game/Table/Weapons")
	};
	const TCHAR* DirectManagedDesignRoots[] =
	{
		TEXT("Attack"),
		TEXT("Death"),
		TEXT("Dodge"),
		TEXT("Groggy"),
		TEXT("HitReaction"),
		TEXT("Props"),
		TEXT("Sprint"),
		TEXT("Weapons")
	};

#if WITH_EDITOR
	FString GetMaverickDesignDir()
	{
		return FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() / TEXT("MaverickDesign"));
	}

	FString GetJsonDir()
	{
		return GetMaverickDesignDir() / TEXT("Json");
	}

	FString GetConverterDir()
	{
		return GetMaverickDesignDir() / TEXT("CsvToJsonConverter");
	}

	FString GetConverterExePath()
	{
		return GetConverterDir() / TEXT("CsvToJsonConverter.exe");
	}

	FString GetConverterScriptPath()
	{
		return GetConverterDir() / TEXT("CsvToJsonConverter.py");
	}

	FString SanitizeAssetName(const FString& RawName)
	{
		FString Out;
		Out.Reserve(RawName.Len());

		for (const TCHAR Character : RawName)
		{
			if (FChar::IsAlnum(Character) || Character == TEXT('_'))
			{
				Out.AppendChar(Character);
			}
			else
			{
				Out.AppendChar(TEXT('_'));
			}
		}

		return Out.IsEmpty() ? TEXT("Unnamed") : Out;
	}

	FString ToTableAssetName(const FString& TableName)
	{
		return FString::Printf(TEXT("DT_%s"), *SanitizeAssetName(TableName));
	}

	FString ToTablePackagePath(const FString& TableName)
	{
		const FString TableAssetName = ToTableAssetName(TableName);
		if (TableName.Equals(TEXT("CharacterStat"), ESearchCase::IgnoreCase))
		{
			return FString::Printf(TEXT("%s/Stat/%s"), GeneratedTablesPackageRoot, *TableAssetName);
		}
		if (TableName.Equals(TEXT("GameGuide"), ESearchCase::IgnoreCase))
		{
			return FString::Printf(TEXT("%s/UI/%s"), GeneratedTablesPackageRoot, *TableAssetName);
		}

		return FString::Printf(TEXT("%s/%s"), GeneratedTablesPackageRoot, *TableAssetName);
	}

	bool TableAssetGeneratorIsUnderPackageRoot(const FString& PackagePath, const FString& RootPath)
	{
		return PackagePath.Equals(RootPath)
			|| PackagePath.StartsWith(RootPath + TEXT("/"), ESearchCase::CaseSensitive);
	}

	bool TableAssetGeneratorIsDirectManagedPackagePath(const FString& PackagePath)
	{
		for (const TCHAR* Root : DirectManagedTablePackageRoots)
		{
			if (TableAssetGeneratorIsUnderPackageRoot(PackagePath, Root))
			{
				return true;
			}
		}

		return false;
	}

	bool TableAssetGeneratorIsDirectManagedObjectPath(const FString& ObjectPath)
	{
		FString PackageName;
		FString ObjectName;
		if (!ObjectPath.Split(TEXT("."), &PackageName, &ObjectName, ESearchCase::CaseSensitive, ESearchDir::FromEnd))
		{
			PackageName = ObjectPath;
		}

		return TableAssetGeneratorIsDirectManagedPackagePath(PackageName);
	}

	bool TableAssetGeneratorIsDirectManagedJsonPath(const FString& JsonPath)
	{
		FString RelativePath = FPaths::ConvertRelativePathToFull(JsonPath);
		const FString JsonDir = GetJsonDir();
		if (!FPaths::MakePathRelativeTo(RelativePath, *JsonDir))
		{
			return false;
		}

		FPaths::MakeStandardFilename(RelativePath);
		for (const TCHAR* Root : DirectManagedDesignRoots)
		{
			if (RelativePath.Equals(Root, ESearchCase::IgnoreCase)
				|| RelativePath.StartsWith(FString(Root) + TEXT("/"), ESearchCase::IgnoreCase))
			{
				return true;
			}
		}

		return false;
	}

	FString TableAssetGeneratorResolveKeyColumnName(const UScriptStruct* RowStruct, const FString& FallbackKeyColumnName)
	{
		if (!FallbackKeyColumnName.IsEmpty())
		{
			return FallbackKeyColumnName;
		}

		if (RowStruct && RowStruct->IsChildOf(FMVActionRow::StaticStruct()))
		{
			return TEXT("RowName");
		}

		if (RowStruct == FMVGenericTableRow::StaticStruct())
		{
			return TEXT("Key");
		}

		return TEXT("RowName");
	}

	bool TableAssetGeneratorAddManifestRow(
		TArray<FMVTableManifestRow>& OutRows,
		const FMVTableManifestRow& Row,
		TArray<FString>& OutErrors)
	{
		if (Row.TableName.IsNone())
		{
			OutErrors.Add(TEXT("Manifest row has no TableName."));
			return false;
		}

		for (const FMVTableManifestRow& ExistingRow : OutRows)
		{
			if (ExistingRow.TableName == Row.TableName)
			{
				OutErrors.Add(FString::Printf(
					TEXT("Duplicate manifest table '%s': %s and %s"),
					*Row.TableName.ToString(),
					*ExistingRow.AssetPath,
					*Row.AssetPath));
				return false;
			}
		}

		OutRows.Add(Row);
		return true;
	}

	void TableAssetGeneratorSortManifestRows(TArray<FMVTableManifestRow>& Rows)
	{
		Rows.Sort([](const FMVTableManifestRow& Left, const FMVTableManifestRow& Right)
		{
			return Left.TableName.LexicalLess(Right.TableName);
		});
	}

	bool TryGetObjectField(const TSharedPtr<FJsonObject>& Object, const FString& FieldName, TSharedPtr<FJsonObject>& OutObject)
	{
		if (!Object.IsValid())
		{
			return false;
		}

		const TSharedPtr<FJsonValue>* Value = Object->Values.Find(FieldName);
		if (!Value || !Value->IsValid() || (*Value)->Type != EJson::Object)
		{
			return false;
		}

		OutObject = (*Value)->AsObject();
		return OutObject.IsValid();
	}

	bool TryGetArrayField(const TSharedPtr<FJsonObject>& Object, const FString& FieldName, const TArray<TSharedPtr<FJsonValue>>*& OutArray)
	{
		if (!Object.IsValid())
		{
			return false;
		}

		const TSharedPtr<FJsonValue>* Value = Object->Values.Find(FieldName);
		if (!Value || !Value->IsValid() || (*Value)->Type != EJson::Array)
		{
			return false;
		}

		OutArray = &(*Value)->AsArray();
		return true;
	}

	bool TryGetStringField(const TSharedPtr<FJsonObject>& Object, const FString& FieldName, FString& OutString)
	{
		if (!Object.IsValid())
		{
			return false;
		}

		const TSharedPtr<FJsonValue>* Value = Object->Values.Find(FieldName);
		if (!Value || !Value->IsValid() || (*Value)->Type != EJson::String)
		{
			return false;
		}

		OutString = (*Value)->AsString();
		return !OutString.IsEmpty();
	}

	bool JsonValueToGeneratorString(const TSharedPtr<FJsonValue>& Value, FString& OutValue)
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

	bool JsonObjectToCompactString(const TSharedPtr<FJsonObject>& Object, FString& OutJson)
	{
		if (!Object.IsValid())
		{
			return false;
		}

		const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutJson, 0);
		return FJsonSerializer::Serialize(Object.ToSharedRef(), Writer);
	}

	void TableAssetGeneratorAppendJsonHashValue(const TSharedPtr<FJsonValue>& Value, FString& OutSource)
	{
		if (!Value.IsValid())
		{
			OutSource += TEXT("Invalid;");
			return;
		}

		switch (Value->Type)
		{
		case EJson::String:
		{
			const FString StringValue = Value->AsString();
			OutSource += FString::Printf(TEXT("S%d:"), StringValue.Len());
			OutSource += StringValue;
			OutSource += TEXT(";");
			break;
		}
		case EJson::Number:
		{
			FString NumberString;
			JsonValueToGeneratorString(Value, NumberString);
			OutSource += TEXT("N:");
			OutSource += NumberString;
			OutSource += TEXT(";");
			break;
		}
		case EJson::Boolean:
			OutSource += Value->AsBool() ? TEXT("B:true;") : TEXT("B:false;");
			break;
		case EJson::Array:
			OutSource += TEXT("[");
			for (const TSharedPtr<FJsonValue>& Element : Value->AsArray())
			{
				TableAssetGeneratorAppendJsonHashValue(Element, OutSource);
			}
			OutSource += TEXT("];");
			break;
		case EJson::Object:
		{
			const TSharedPtr<FJsonObject> Object = Value->AsObject();
			if (!Object.IsValid())
			{
				OutSource += TEXT("Object:null;");
				break;
			}

			TArray<FString> FieldNames;
			Object->Values.GetKeys(FieldNames);
			FieldNames.Sort();

			OutSource += TEXT("{");
			for (const FString& FieldName : FieldNames)
			{
				OutSource += FString::Printf(TEXT("K%d:"), FieldName.Len());
				OutSource += FieldName;
				OutSource += TEXT("=");
				TableAssetGeneratorAppendJsonHashValue(Object->Values[FieldName], OutSource);
			}
			OutSource += TEXT("};");
			break;
		}
		case EJson::Null:
			OutSource += TEXT("Null;");
			break;
		default:
			OutSource += TEXT("None;");
			break;
		}
	}

	FString TableAssetGeneratorHashSourceString(const FString& Source)
	{
		const FTCHARToUTF8 Utf8Source(*Source);
		return FMD5::HashBytes(
			reinterpret_cast<const uint8*>(Utf8Source.Get()),
			static_cast<uint64>(Utf8Source.Length()));
	}

	FString TableAssetGeneratorBuildGeneratedTableHash(
		const FString& TableName,
		const FMVSheetSpec& Spec,
		const TArray<TSharedPtr<FJsonValue>>& JsonRows)
	{
		FString Source;
		Source.Reserve(1024);
		Source += TEXT("GeneratedTable:v1;");
		Source += TEXT("Table=");
		Source += TableName;
		Source += TEXT(";Key=");
		Source += Spec.KeyColumnName;
		Source += TEXT(";Struct=");
		Source += Spec.RowStruct ? Spec.RowStruct->GetPathName() : TEXT("None");
		Source += TEXT(";Generic=");
		Source += Spec.bGenericFallback ? TEXT("true;") : TEXT("false;");
		Source += FString::Printf(TEXT("Rows=%d;"), JsonRows.Num());

		for (const TSharedPtr<FJsonValue>& Row : JsonRows)
		{
			TableAssetGeneratorAppendJsonHashValue(Row, Source);
		}

		return TableAssetGeneratorHashSourceString(Source);
	}

	FString TableAssetGeneratorBuildManifestHash(const TArray<FMVTableManifestRow>& ManifestRows)
	{
		FString Source;
		Source.Reserve(1024);
		Source += TEXT("TableManifest:v1;");
		Source += FString::Printf(TEXT("Rows=%d;"), ManifestRows.Num());

		for (const FMVTableManifestRow& Row : ManifestRows)
		{
			Source += TEXT("Table=");
			Source += Row.TableName.ToString();
			Source += TEXT(";Asset=");
			Source += Row.AssetPath;
			Source += TEXT(";Key=");
			Source += Row.KeyColumnName;
			Source += TEXT(";Struct=");
			Source += Row.RowStructName;
			Source += TEXT(";Generic=");
			Source += Row.bGenericFallback ? TEXT("true;") : TEXT("false;");
		}

		return TableAssetGeneratorHashSourceString(Source);
	}

	FString TableAssetGeneratorReadStoredSourceHash(const UObject* Asset)
	{
		if (!Asset)
		{
			return FString();
		}

		UPackage* Package = Asset->GetOutermost();
		if (!Package)
		{
			return FString();
		}

		return Package->GetMetaData().GetValue(Asset, GeneratedTableHashMetadataKey);
	}

	void TableAssetGeneratorWriteStoredSourceHash(UObject* Asset, const FString& SourceHash)
	{
		if (!Asset)
		{
			return;
		}

		if (UPackage* Package = Asset->GetOutermost())
		{
			Package->GetMetaData().SetValue(Asset, GeneratedTableHashMetadataKey, *SourceHash);
		}
	}

	bool TableAssetGeneratorCanReuseDataTable(
		const UDataTable* DataTable,
		const UScriptStruct* ExpectedRowStruct,
		const FString& ExpectedSourceHash)
	{
		return DataTable
			&& DataTable->GetRowStruct() == ExpectedRowStruct
			&& !ExpectedSourceHash.IsEmpty()
			&& TableAssetGeneratorReadStoredSourceHash(DataTable) == ExpectedSourceHash;
	}

	bool TableAssetGeneratorReadStringKeySet(
		const FString& JsonPath,
		const FString& TableName,
		const FString& KeyColumnName,
		TSet<FString>& OutKeys,
		TArray<FString>& OutErrors)
	{
		FString Raw;
		if (!FFileHelper::LoadFileToString(Raw, *JsonPath))
		{
			OutErrors.Add(FString::Printf(TEXT("Cannot read JSON for validation: %s"), *JsonPath));
			return false;
		}

		TSharedPtr<FJsonObject> Root;
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Raw);
		if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
		{
			OutErrors.Add(FString::Printf(TEXT("Invalid JSON for validation: %s"), *JsonPath));
			return false;
		}

		TSharedPtr<FJsonObject> TablesObject;
		TSharedPtr<FJsonObject> TableObject;
		const TArray<TSharedPtr<FJsonValue>>* Rows = nullptr;
		if (!TryGetObjectField(Root, TEXT("tables"), TablesObject)
			|| !TryGetObjectField(TablesObject, TableName, TableObject)
			|| !TryGetArrayField(TableObject, TEXT("rows"), Rows)
			|| !Rows)
		{
			OutErrors.Add(FString::Printf(TEXT("%s: table '%s' has no rows for validation."), *JsonPath, *TableName));
			return false;
		}

		bool bSucceeded = true;
		for (const TSharedPtr<FJsonValue>& Element : *Rows)
		{
			if (!Element.IsValid() || Element->Type != EJson::Object)
			{
				OutErrors.Add(FString::Printf(TEXT("%s: table '%s' has a non-object row."), *JsonPath, *TableName));
				bSucceeded = false;
				continue;
			}

			const TSharedPtr<FJsonObject> RowObject = Element->AsObject();
			const TSharedPtr<FJsonValue>* KeyValue = RowObject->Values.Find(KeyColumnName);
			FString KeyString;
			if (!KeyValue
				|| !KeyValue->IsValid()
				|| !JsonValueToGeneratorString(*KeyValue, KeyString)
				|| KeyString.IsEmpty())
			{
				OutErrors.Add(FString::Printf(
					TEXT("%s: table '%s' has an invalid '%s' value."),
					*JsonPath,
					*TableName,
					*KeyColumnName));
				bSucceeded = false;
				continue;
			}

			if (OutKeys.Contains(KeyString))
			{
				OutErrors.Add(FString::Printf(
					TEXT("%s: table '%s' has duplicate '%s' value '%s'."),
					*JsonPath,
					*TableName,
					*KeyColumnName,
					*KeyString));
				bSucceeded = false;
				continue;
			}

			OutKeys.Add(KeyString);
		}

		return bSucceeded;
	}

	bool TableAssetGeneratorReadIntegerKeySet(
		const FString& JsonPath,
		const FString& TableName,
		const FString& KeyColumnName,
		TSet<FString>& OutKeys,
		TArray<FString>& OutErrors)
	{
		FString Raw;
		if (!FFileHelper::LoadFileToString(Raw, *JsonPath))
		{
			OutErrors.Add(FString::Printf(TEXT("Cannot read JSON for validation: %s"), *JsonPath));
			return false;
		}

		TSharedPtr<FJsonObject> Root;
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Raw);
		if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
		{
			OutErrors.Add(FString::Printf(TEXT("Invalid JSON for validation: %s"), *JsonPath));
			return false;
		}

		TSharedPtr<FJsonObject> TablesObject;
		TSharedPtr<FJsonObject> TableObject;
		const TArray<TSharedPtr<FJsonValue>>* Rows = nullptr;
		if (!TryGetObjectField(Root, TEXT("tables"), TablesObject)
			|| !TryGetObjectField(TablesObject, TableName, TableObject)
			|| !TryGetArrayField(TableObject, TEXT("rows"), Rows)
			|| !Rows)
		{
			OutErrors.Add(FString::Printf(TEXT("%s: table '%s' has no rows for validation."), *JsonPath, *TableName));
			return false;
		}

		bool bSucceeded = true;
		for (const TSharedPtr<FJsonValue>& Element : *Rows)
		{
			if (!Element.IsValid() || Element->Type != EJson::Object)
			{
				OutErrors.Add(FString::Printf(TEXT("%s: table '%s' has a non-object row."), *JsonPath, *TableName));
				bSucceeded = false;
				continue;
			}

			const TSharedPtr<FJsonObject> RowObject = Element->AsObject();
			const TSharedPtr<FJsonValue>* KeyValue = RowObject->Values.Find(KeyColumnName);
			FString KeyString;
			if (!KeyValue
				|| !KeyValue->IsValid()
				|| !JsonValueToGeneratorString(*KeyValue, KeyString)
				|| KeyString.IsEmpty())
			{
				OutErrors.Add(FString::Printf(
					TEXT("%s: table '%s' has an invalid '%s' value."),
					*JsonPath,
					*TableName,
					*KeyColumnName));
				bSucceeded = false;
				continue;
			}

			if (OutKeys.Contains(KeyString))
			{
				OutErrors.Add(FString::Printf(
					TEXT("%s: table '%s' has duplicate '%s' value '%s'."),
					*JsonPath,
					*TableName,
					*KeyColumnName,
					*KeyString));
				bSucceeded = false;
				continue;
			}

			OutKeys.Add(KeyString);
		}

		return bSucceeded;
	}

	bool SaveDataTableAsset(UDataTable* DataTable, const FString& PackagePath, FString& OutError)
	{
		if (!DataTable)
		{
			OutError = TEXT("DataTable is null.");
			return false;
		}

		const FString FilePath = FPackageName::LongPackageNameToFilename(
			PackagePath,
			FPackageName::GetAssetPackageExtension());
		IFileManager::Get().MakeDirectory(*FPaths::GetPath(FilePath), true);

		FSavePackageArgs SaveArgs;
		SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
		SaveArgs.SaveFlags = SAVE_NoError;
		SaveArgs.bForceByteSwapping = false;
		SaveArgs.bWarnOfLongFilename = false;

		UPackage* Package = DataTable->GetPackage();
		if (!UPackage::SavePackage(Package, DataTable, *FilePath, SaveArgs))
		{
			OutError = FString::Printf(TEXT("SavePackage failed: %s"), *FilePath);
			return false;
		}

		return true;
	}

	UDataTable* FindExistingDataTable(const FString& AssetName, const FString& PackagePath)
	{
		UPackage* Package = CreatePackage(*PackagePath);
		if (!Package)
		{
			return nullptr;
		}
		Package->FullyLoad();

		return FindObject<UDataTable>(Package, *AssetName);
	}

	UDataTable* CreateOrResetDataTable(
		const FString& AssetName,
		const FString& PackagePath,
		UScriptStruct* RowStruct)
	{
		UPackage* Package = CreatePackage(*PackagePath);
		if (!Package)
		{
			return nullptr;
		}
		Package->FullyLoad();

		UDataTable* DataTable = FindObject<UDataTable>(Package, *AssetName);
		const bool bNewlyCreated = DataTable == nullptr;
		if (DataTable)
		{
			DataTable->EmptyTable();
		}
		else
		{
			DataTable = NewObject<UDataTable>(Package, FName(*AssetName), RF_Public | RF_Standalone);
		}

		DataTable->RowStruct = RowStruct;

		if (bNewlyCreated)
		{
			FAssetRegistryModule::AssetCreated(DataTable);
		}

		return DataTable;
	}
#endif
}

#if WITH_EDITOR
static FAutoConsoleCommand GenerateMaverickTableDataTablesCommand(
	TEXT("MV.Table.GenerateDataTables"),
	TEXT("Runs CSV conversion and regenerates UDataTable assets under /Game/Table from MaverickDesign/Json."),
	FConsoleCommandDelegate::CreateStatic([]()
	{
		FString Report;
		const bool bSucceeded = UMVTableAssetGenerator::GenerateDataTables(Report);
		UE_LOG(LogTemp, Display, TEXT("[MVTableAssetGenerator] %s%s"), bSucceeded ? TEXT("") : TEXT("Failed: "), *Report);
	}));

static FAutoConsoleCommand RefreshMaverickTableManifestCommand(
	TEXT("MV.Table.RefreshManifest"),
	TEXT("Refreshes /Game/Table/DT_MVTableManifest from CSV-originated root tables and direct-managed DataTables."),
	FConsoleCommandDelegate::CreateStatic([]()
	{
		FString Report;
		const bool bSucceeded = UMVTableAssetGenerator::RefreshTableManifest(Report);
		UE_LOG(LogTemp, Display, TEXT("[MVTableAssetGenerator] %s%s"), bSucceeded ? TEXT("") : TEXT("Failed: "), *Report);
	}));
#endif

bool UMVTableAssetGenerator::GenerateDataTables(FString& OutReport)
{
#if WITH_EDITOR
	OutReport.Reset();

	FString ConverterLog;
	if (!RunCsvConverter(ConverterLog))
	{
		OutReport = FString::Printf(TEXT("CSV conversion failed.\n%s"), *ConverterLog);
		return false;
	}

	FMVSheetSpecs::Invalidate();

	TArray<FString> Errors;
	if (!ValidateCharacterStatMapping(Errors))
	{
		OutReport = FString::Printf(TEXT("CSV conversion succeeded, but table validation failed.\n%s"), *ConverterLog);
		if (!Errors.IsEmpty())
		{
			OutReport += LINE_TERMINATOR;
			OutReport += FString::Join(Errors, LINE_TERMINATOR);
		}
		return false;
	}

	TArray<FMVTableManifestRow> TableManifestRows;
	const int32 ImportedCount = ImportAllJsonFiles(TableManifestRows, Errors);
	const int32 DirectManagedCount = ScanDirectManagedDataTables(TableManifestRows, Errors);

	TableAssetGeneratorSortManifestRows(TableManifestRows);

	TArray<FString> CleanupErrors;
	DeleteStaleGeneratedAssets(TableManifestRows, CleanupErrors);
	Errors.Append(CleanupErrors);

	if (ImportedCount > 0)
	{
		FString ManifestError;
		if (!SaveManifest(TableManifestRows, ManifestError))
		{
			Errors.Add(ManifestError);
		}
	}

	OutReport = FString::Printf(
		TEXT("Converter output:\n%s\nGenerated %d CSV-originated table asset(s).\nRegistered %d direct-managed table asset(s)."),
		*ConverterLog,
		ImportedCount,
		DirectManagedCount);

	if (!Errors.IsEmpty())
	{
		OutReport += LINE_TERMINATOR;
		OutReport += FString::Join(Errors, LINE_TERMINATOR);
	}

	if (Errors.IsEmpty())
	{
		if (UMVTableManager* TableManager = UMVTableManager::Get())
		{
			TableManager->ReloadAllTables();
		}
	}

	return Errors.IsEmpty();
#else
	OutReport = TEXT("Table generation is editor-only.");
	return false;
#endif
}

bool UMVTableAssetGenerator::RefreshTableManifest(FString& OutReport)
{
#if WITH_EDITOR
	OutReport.Reset();

	FMVSheetSpecs::Invalidate();

	TArray<FString> Errors;
	TArray<FMVTableManifestRow> TableManifestRows;
	const int32 CsvOriginCount = BuildCsvOriginManifestRows(TableManifestRows, Errors);
	const int32 DirectManagedCount = ScanDirectManagedDataTables(TableManifestRows, Errors);

	TableAssetGeneratorSortManifestRows(TableManifestRows);

	FString ManifestError;
	if (!SaveManifest(TableManifestRows, ManifestError))
	{
		Errors.Add(ManifestError);
	}

	OutReport = FString::Printf(
		TEXT("Registered %d CSV-originated table asset(s).\nRegistered %d direct-managed table asset(s)."),
		CsvOriginCount,
		DirectManagedCount);

	if (!Errors.IsEmpty())
	{
		OutReport += LINE_TERMINATOR;
		OutReport += FString::Join(Errors, LINE_TERMINATOR);
	}

	if (Errors.IsEmpty())
	{
		if (UMVTableManager* TableManager = UMVTableManager::Get())
		{
			TableManager->ReloadAllTables();
		}
	}

	return Errors.IsEmpty();
#else
	OutReport = TEXT("Table manifest refresh is editor-only.");
	return false;
#endif
}

#if WITH_EDITOR
bool UMVTableAssetGenerator::RunCsvConverter(FString& OutLog)
{
	const FString ExePath = GetConverterExePath();
	const FString ScriptPath = GetConverterScriptPath();

	FString Program;
	FString Args;
	FString WorkingDir = GetConverterDir();

	if (FPaths::FileExists(ExePath))
	{
		Program = ExePath;
	}
	else if (FPaths::FileExists(ScriptPath))
	{
		Program = FPlatformMisc::GetEnvironmentVariable(TEXT("ComSpec"));
		if (Program.IsEmpty())
		{
			Program = TEXT("C:\\Windows\\System32\\cmd.exe");
		}
		Args = FString::Printf(TEXT("/C python \"%s\""), *ScriptPath);
	}
	else
	{
		OutLog = FString::Printf(TEXT("Converter not found: %s or %s"), *ExePath, *ScriptPath);
		return false;
	}

	int32 ReturnCode = -1;
	FString StdOut;
	FString StdErr;
	const bool bExecuted = FPlatformProcess::ExecProcess(
		*Program,
		*Args,
		&ReturnCode,
		&StdOut,
		&StdErr,
		*WorkingDir);

	OutLog = FString::Printf(
		TEXT("Program=%s\nArgs=%s\nExitCode=%d\n--- STDOUT ---\n%s\n--- STDERR ---\n%s"),
		*Program,
		*Args,
		ReturnCode,
		*StdOut,
		*StdErr);

	return bExecuted && ReturnCode == 0;
}

bool UMVTableAssetGenerator::ValidateCharacterStatMapping(TArray<FString>& OutErrors)
{
	const FString JsonDir = GetJsonDir();
	TSet<FString> CharacterIndexCodes;
	TSet<FString> CharacterStatCodes;

	const bool bLoadedCharacterIndex = TableAssetGeneratorReadStringKeySet(
		JsonDir / TEXT("CharacterIndex.json"),
		TEXT("CharacterIndex"),
		TEXT("CharacterIndexCode"),
		CharacterIndexCodes,
		OutErrors);
	const bool bLoadedCharacterStat = TableAssetGeneratorReadStringKeySet(
		JsonDir / TEXT("CharacterStat.json"),
		TEXT("CharacterStat"),
		TEXT("CharacterIndexCode"),
		CharacterStatCodes,
		OutErrors);
	if (!bLoadedCharacterIndex || !bLoadedCharacterStat)
	{
		return false;
	}

	for (const FString& CharacterIndexCode : CharacterIndexCodes)
	{
		if (!CharacterStatCodes.Contains(CharacterIndexCode))
		{
			OutErrors.Add(FString::Printf(
				TEXT("CharacterStat is missing CharacterIndexCode '%s'."),
				*CharacterIndexCode));
		}
	}

	for (const FString& CharacterStatCode : CharacterStatCodes)
	{
		if (!CharacterIndexCodes.Contains(CharacterStatCode))
		{
			OutErrors.Add(FString::Printf(
				TEXT("CharacterStat has CharacterIndexCode '%s' with no matching CharacterIndex row."),
				*CharacterStatCode));
		}
	}

	return OutErrors.IsEmpty();
}

int32 UMVTableAssetGenerator::ImportAllJsonFiles(TArray<FMVTableManifestRow>& OutTableManifestRows, TArray<FString>& OutErrors)
{
	const FString JsonDir = GetJsonDir();
	if (!IFileManager::Get().DirectoryExists(*JsonDir))
	{
		OutErrors.Add(FString::Printf(TEXT("Json directory not found: %s"), *JsonDir));
		return 0;
	}

	TArray<FString> JsonFiles;
	IFileManager::Get().FindFilesRecursive(JsonFiles, *JsonDir, TEXT("*.json"), true, false);
	JsonFiles.Sort();

	int32 ImportedCount = 0;
	for (const FString& JsonPath : JsonFiles)
	{
		if (FPaths::GetCleanFilename(JsonPath).Equals(TEXT("SheetRecipe.json"), ESearchCase::IgnoreCase))
		{
			continue;
		}
		if (TableAssetGeneratorIsDirectManagedJsonPath(JsonPath))
		{
			continue;
		}

		ImportedCount += ImportJsonFile(JsonPath, OutTableManifestRows, OutErrors);
	}

	return ImportedCount;
}

int32 UMVTableAssetGenerator::BuildCsvOriginManifestRows(
	TArray<FMVTableManifestRow>& OutTableManifestRows,
	TArray<FString>& OutErrors)
{
	const FString JsonDir = GetJsonDir();
	if (!IFileManager::Get().DirectoryExists(*JsonDir))
	{
		OutErrors.Add(FString::Printf(TEXT("Json directory not found: %s"), *JsonDir));
		return 0;
	}

	TArray<FString> JsonFiles;
	IFileManager::Get().FindFilesRecursive(JsonFiles, *JsonDir, TEXT("*.json"), true, false);
	JsonFiles.Sort();

	int32 RegisteredCount = 0;
	for (const FString& JsonPath : JsonFiles)
	{
		if (FPaths::GetCleanFilename(JsonPath).Equals(TEXT("SheetRecipe.json"), ESearchCase::IgnoreCase))
		{
			continue;
		}
		if (TableAssetGeneratorIsDirectManagedJsonPath(JsonPath))
		{
			continue;
		}

		RegisteredCount += BuildCsvOriginManifestRowsFromJsonFile(JsonPath, OutTableManifestRows, OutErrors);
	}

	return RegisteredCount;
}

int32 UMVTableAssetGenerator::BuildCsvOriginManifestRowsFromJsonFile(
	const FString& JsonPath,
	TArray<FMVTableManifestRow>& OutTableManifestRows,
	TArray<FString>& OutErrors)
{
	FString Raw;
	if (!FFileHelper::LoadFileToString(Raw, *JsonPath))
	{
		OutErrors.Add(FString::Printf(TEXT("Cannot read JSON: %s"), *JsonPath));
		return 0;
	}

	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Raw);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		OutErrors.Add(FString::Printf(TEXT("Invalid JSON: %s"), *JsonPath));
		return 0;
	}

	TSharedPtr<FJsonObject> TablesObject;
	if (!TryGetObjectField(Root, TEXT("tables"), TablesObject))
	{
		return 0;
	}

	int32 RegisteredCount = 0;
	for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : TablesObject->Values)
	{
		if (!Pair.Value.IsValid() || Pair.Value->Type != EJson::Object)
		{
			OutErrors.Add(FString::Printf(TEXT("%s: table '%s' is not an object."), *JsonPath, *Pair.Key));
			continue;
		}

		const TSharedPtr<FJsonObject> TableObject = Pair.Value->AsObject();
		FString KeyColumnName;
		if (!TryGetStringField(TableObject, TEXT("key"), KeyColumnName))
		{
			OutErrors.Add(FString::Printf(TEXT("%s: table '%s' has no key field."), *JsonPath, *Pair.Key));
			continue;
		}

		const FString AssetName = ToTableAssetName(Pair.Key);
		const FString PackagePath = ToTablePackagePath(Pair.Key);
		const FString AssetPath = FString::Printf(TEXT("%s.%s"), *PackagePath, *AssetName);
		UDataTable* DataTable = LoadObject<UDataTable>(nullptr, *AssetPath);
		if (!DataTable)
		{
			OutErrors.Add(FString::Printf(
				TEXT("CSV-originated table asset not found for '%s': %s"),
				*Pair.Key,
				*AssetPath));
			continue;
		}

		FMVTableManifestRow ManifestRow;
		FString Error;
		if (BuildManifestRowForDataTable(Pair.Key, DataTable, KeyColumnName, ManifestRow, Error)
			&& TableAssetGeneratorAddManifestRow(OutTableManifestRows, ManifestRow, OutErrors))
		{
			++RegisteredCount;
		}
		else if (!Error.IsEmpty())
		{
			OutErrors.Add(Error);
		}
	}

	return RegisteredCount;
}

int32 UMVTableAssetGenerator::ImportJsonFile(
	const FString& JsonPath,
	TArray<FMVTableManifestRow>& OutTableManifestRows,
	TArray<FString>& OutErrors)
{
	FString Raw;
	if (!FFileHelper::LoadFileToString(Raw, *JsonPath))
	{
		OutErrors.Add(FString::Printf(TEXT("Cannot read JSON: %s"), *JsonPath));
		return 0;
	}

	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Raw);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		OutErrors.Add(FString::Printf(TEXT("Invalid JSON: %s"), *JsonPath));
		return 0;
	}

	TSharedPtr<FJsonObject> TablesObject;
	if (TryGetObjectField(Root, TEXT("tables"), TablesObject))
	{
		int32 ImportedCount = 0;
		for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : TablesObject->Values)
		{
			if (!Pair.Value.IsValid() || Pair.Value->Type != EJson::Object)
			{
				OutErrors.Add(FString::Printf(TEXT("%s: table '%s' is not an object."), *JsonPath, *Pair.Key));
				continue;
			}

			const TSharedPtr<FJsonObject> TableObject = Pair.Value->AsObject();
			FString KeyColumnName;
			const TArray<TSharedPtr<FJsonValue>>* JsonRows = nullptr;
			if (!TryGetStringField(TableObject, TEXT("key"), KeyColumnName))
			{
				OutErrors.Add(FString::Printf(TEXT("%s: table '%s' has no key field."), *JsonPath, *Pair.Key));
				continue;
			}
			if (!TryGetArrayField(TableObject, TEXT("rows"), JsonRows) || !JsonRows)
			{
				OutErrors.Add(FString::Printf(TEXT("%s: table '%s' has no rows array."), *JsonPath, *Pair.Key));
				continue;
			}

			FMVSheetSpec InlineSpec = FMVSheetSpec{ FMVGenericTableRow::StaticStruct(), KeyColumnName, true };
			const FMVSheetSpec* Spec = FMVSheetSpecs::Find(Pair.Key);
			if (!Spec)
			{
				Spec = &InlineSpec;
			}

			FString Error;
			FMVTableManifestRow ManifestRow;
			if (BuildAndSaveDataTable(Pair.Key, *Spec, *JsonRows, ManifestRow, Error))
			{
				if (TableAssetGeneratorAddManifestRow(OutTableManifestRows, ManifestRow, OutErrors))
				{
					++ImportedCount;
				}
			}
			else
			{
				OutErrors.Add(Error);
			}
		}
		return ImportedCount;
	}

	int32 ImportedCount = 0;
	for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : Root->Values)
	{
		if (!Pair.Value.IsValid() || Pair.Value->Type != EJson::Array)
		{
			continue;
		}

		const FMVSheetSpec* Spec = FMVSheetSpecs::Find(Pair.Key);
		if (!Spec)
		{
			OutErrors.Add(FString::Printf(TEXT("%s: legacy table '%s' has no sheet spec."), *JsonPath, *Pair.Key));
			continue;
		}

		FString Error;
		FMVTableManifestRow ManifestRow;
		if (BuildAndSaveDataTable(Pair.Key, *Spec, Pair.Value->AsArray(), ManifestRow, Error))
		{
			if (TableAssetGeneratorAddManifestRow(OutTableManifestRows, ManifestRow, OutErrors))
			{
				++ImportedCount;
			}
		}
		else
		{
			OutErrors.Add(Error);
		}
	}

	return ImportedCount;
}

int32 UMVTableAssetGenerator::ScanDirectManagedDataTables(
	TArray<FMVTableManifestRow>& OutTableManifestRows,
	TArray<FString>& OutErrors)
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	int32 RegisteredCount = 0;
	for (const TCHAR* Root : DirectManagedTablePackageRoots)
	{
		TArray<FString> PathsToScan;
		PathsToScan.Add(Root);
		AssetRegistryModule.Get().ScanPathsSynchronous(PathsToScan, true);

		FARFilter Filter;
		Filter.PackagePaths.Add(FName(Root));
		Filter.ClassPaths.Add(UDataTable::StaticClass()->GetClassPathName());
		Filter.bRecursivePaths = true;

		TArray<FAssetData> Assets;
		AssetRegistryModule.Get().GetAssets(Filter, Assets);
		Assets.Sort([](const FAssetData& Left, const FAssetData& Right)
		{
			return Left.PackageName.LexicalLess(Right.PackageName);
		});

		for (const FAssetData& Asset : Assets)
		{
			const FString AssetName = Asset.AssetName.ToString();
			if (AssetName.Equals(ManifestAssetName, ESearchCase::IgnoreCase))
			{
				continue;
			}

			FString TableName = AssetName;
			TableName.RemoveFromStart(TEXT("DT_"));
			if (TableName.IsEmpty())
			{
				OutErrors.Add(FString::Printf(TEXT("Direct-managed DataTable has invalid asset name: %s"), *AssetName));
				continue;
			}

			UDataTable* DataTable = Cast<UDataTable>(Asset.GetAsset());
			if (!DataTable)
			{
				OutErrors.Add(FString::Printf(
					TEXT("Failed to load direct-managed DataTable '%s'."),
					*Asset.GetSoftObjectPath().ToString()));
				continue;
			}

			FMVTableManifestRow ManifestRow;
			FString Error;
			if (BuildManifestRowForDataTable(TableName, DataTable, FString(), ManifestRow, Error)
				&& TableAssetGeneratorAddManifestRow(OutTableManifestRows, ManifestRow, OutErrors))
			{
				++RegisteredCount;
			}
			else if (!Error.IsEmpty())
			{
				OutErrors.Add(Error);
			}
		}
	}

	return RegisteredCount;
}

bool UMVTableAssetGenerator::BuildManifestRowForDataTable(
	const FString& TableName,
	UDataTable* DataTable,
	const FString& KeyColumnName,
	FMVTableManifestRow& OutManifestRow,
	FString& OutError)
{
	if (TableName.IsEmpty())
	{
		OutError = TEXT("TableName is empty.");
		return false;
	}
	if (!DataTable)
	{
		OutError = FString::Printf(TEXT("%s: DataTable is null."), *TableName);
		return false;
	}
	if (!DataTable->GetRowStruct())
	{
		OutError = FString::Printf(TEXT("%s: RowStruct is null."), *TableName);
		return false;
	}

	OutManifestRow.TableName = FName(*TableName);
	OutManifestRow.AssetPath = DataTable->GetPathName();
	OutManifestRow.KeyColumnName = TableAssetGeneratorResolveKeyColumnName(DataTable->GetRowStruct(), KeyColumnName);
	OutManifestRow.RowStructName = DataTable->GetRowStruct()->GetName();
	OutManifestRow.bGenericFallback = DataTable->GetRowStruct() == FMVGenericTableRow::StaticStruct();
	return true;
}

bool UMVTableAssetGenerator::BuildAndSaveDataTable(
	const FString& TableName,
	const FMVSheetSpec& Spec,
	const TArray<TSharedPtr<FJsonValue>>& JsonRows,
	FMVTableManifestRow& OutManifestRow,
	FString& OutError)
{
	if (!Spec.RowStruct)
	{
		OutError = FString::Printf(TEXT("%s: RowStruct is null."), *TableName);
		return false;
	}

	const FString AssetName = ToTableAssetName(TableName);
	const FString PackagePath = ToTablePackagePath(TableName);
	const FString SourceHash = TableAssetGeneratorBuildGeneratedTableHash(TableName, Spec, JsonRows);
	if (UDataTable* ExistingDataTable = FindExistingDataTable(AssetName, PackagePath))
	{
		if (TableAssetGeneratorCanReuseDataTable(ExistingDataTable, Spec.RowStruct, SourceHash))
		{
			OutManifestRow.TableName = FName(*TableName);
			OutManifestRow.AssetPath = FString::Printf(TEXT("%s.%s"), *PackagePath, *AssetName);
			OutManifestRow.KeyColumnName = Spec.KeyColumnName;
			OutManifestRow.RowStructName = Spec.RowStruct->GetName();
			OutManifestRow.bGenericFallback = Spec.bGenericFallback;
			return true;
		}
	}

	UDataTable* DataTable = CreateOrResetDataTable(AssetName, PackagePath, Spec.RowStruct);
	if (!DataTable)
	{
		OutError = FString::Printf(TEXT("%s: CreatePackage failed."), *PackagePath);
		return false;
	}

	for (const TSharedPtr<FJsonValue>& Element : JsonRows)
	{
		if (!Element.IsValid() || Element->Type != EJson::Object)
		{
			UE_LOG(LogTemp, Warning, TEXT("[MVTableAssetGenerator] %s: skipped non-object row."), *TableName);
			continue;
		}

		const TSharedPtr<FJsonObject> RowObject = Element->AsObject();
		const TSharedPtr<FJsonValue>* KeyValue = RowObject->Values.Find(Spec.KeyColumnName);
		if (!KeyValue || !KeyValue->IsValid())
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("[MVTableAssetGenerator] %s: row missing key column '%s'."),
				*TableName,
				*Spec.KeyColumnName);
			continue;
		}

		FString KeyString;
		if (!JsonValueToGeneratorString(*KeyValue, KeyString) || KeyString.IsEmpty())
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("[MVTableAssetGenerator] %s: invalid key column '%s'."),
				*TableName,
				*Spec.KeyColumnName);
			continue;
		}

		const FName RowName(*KeyString);
		if (Spec.bGenericFallback || Spec.RowStruct == FMVGenericTableRow::StaticStruct())
		{
			FString RowJson;
			if (!JsonObjectToCompactString(RowObject, RowJson))
			{
				UE_LOG(LogTemp, Warning, TEXT("[MVTableAssetGenerator] %s: failed to serialize generic row '%s'."), *TableName, *KeyString);
				continue;
			}

			FMVGenericTableRow GenericRow;
			GenericRow.Key = KeyString;
			GenericRow.RowJson = RowJson;
			DataTable->AddRow(RowName, GenericRow);
			continue;
		}

		const int32 RowSize = Spec.RowStruct->GetStructureSize();
		void* RowMemory = FMemory::Malloc(RowSize);
		Spec.RowStruct->InitializeStruct(RowMemory);

		const bool bConverted = FJsonObjectConverter::JsonObjectToUStruct(
			RowObject.ToSharedRef(),
			Spec.RowStruct,
			RowMemory,
			0,
			0);

		if (bConverted)
		{
			DataTable->AddRow(RowName, *static_cast<FMVTableRowBase*>(RowMemory));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[MVTableAssetGenerator] %s: JsonObjectToUStruct failed for row '%s'."), *TableName, *KeyString);
		}

		Spec.RowStruct->DestroyStruct(RowMemory);
		FMemory::Free(RowMemory);
	}

	TableAssetGeneratorWriteStoredSourceHash(DataTable, SourceHash);
	DataTable->MarkPackageDirty();

	if (!SaveDataTableAsset(DataTable, PackagePath, OutError))
	{
		return false;
	}

	OutManifestRow.TableName = FName(*TableName);
	OutManifestRow.AssetPath = FString::Printf(TEXT("%s.%s"), *PackagePath, *AssetName);
	OutManifestRow.KeyColumnName = Spec.KeyColumnName;
	OutManifestRow.RowStructName = Spec.RowStruct->GetName();
	OutManifestRow.bGenericFallback = Spec.bGenericFallback;
	return true;
}

bool UMVTableAssetGenerator::SaveManifest(const TArray<FMVTableManifestRow>& ManifestRows, FString& OutError)
{
	const FString SourceHash = TableAssetGeneratorBuildManifestHash(ManifestRows);
	if (UDataTable* ExistingManifest = FindExistingDataTable(ManifestAssetName, ManifestPackagePath))
	{
		if (TableAssetGeneratorCanReuseDataTable(ExistingManifest, FMVTableManifestRow::StaticStruct(), SourceHash))
		{
			return true;
		}
	}

	UDataTable* Manifest = CreateOrResetDataTable(
		ManifestAssetName,
		ManifestPackagePath,
		FMVTableManifestRow::StaticStruct());
	if (!Manifest)
	{
		OutError = TEXT("Failed to create table manifest package.");
		return false;
	}

	for (const FMVTableManifestRow& Row : ManifestRows)
	{
		Manifest->AddRow(Row.TableName, Row);
	}

	TableAssetGeneratorWriteStoredSourceHash(Manifest, SourceHash);
	Manifest->MarkPackageDirty();
	return SaveDataTableAsset(Manifest, ManifestPackagePath, OutError);
}

void UMVTableAssetGenerator::DeleteStaleGeneratedAssets(
	const TArray<FMVTableManifestRow>& NewManifestRows,
	TArray<FString>& OutErrors)
{
	const FString ManifestObjectPath = FString::Printf(TEXT("%s.%s"), ManifestPackagePath, ManifestAssetName);
	UDataTable* ExistingManifest = LoadObject<UDataTable>(nullptr, *ManifestObjectPath);
	if (!ExistingManifest || ExistingManifest->GetRowStruct() != FMVTableManifestRow::StaticStruct())
	{
		return;
	}

	TSet<FString> NewAssetPaths;
	for (const FMVTableManifestRow& Row : NewManifestRows)
	{
		NewAssetPaths.Add(Row.AssetPath);
	}

	TArray<UObject*> ObjectsToDelete;
	for (const TPair<FName, uint8*>& Pair : ExistingManifest->GetRowMap())
	{
		const FMVTableManifestRow* ExistingRow = reinterpret_cast<const FMVTableManifestRow*>(Pair.Value);
		if (!ExistingRow || ExistingRow->AssetPath.IsEmpty() || NewAssetPaths.Contains(ExistingRow->AssetPath))
		{
			continue;
		}
		if (TableAssetGeneratorIsDirectManagedObjectPath(ExistingRow->AssetPath))
		{
			continue;
		}

		if (UObject* Asset = LoadObject<UObject>(nullptr, *ExistingRow->AssetPath))
		{
			ObjectsToDelete.Add(Asset);
		}
	}

	if (!ObjectsToDelete.IsEmpty())
	{
		ObjectTools::DeleteObjectsUnchecked(ObjectsToDelete);
	}
}
#endif
