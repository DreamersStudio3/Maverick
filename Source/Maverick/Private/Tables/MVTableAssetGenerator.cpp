#include "Tables/MVTableAssetGenerator.h"

#include "Tables/MVSheetSpecs.h"
#include "Tables/MVTableTypes.h"

#if WITH_EDITOR
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/DataTable.h"
#include "HAL/FileManager.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformMisc.h"
#include "HAL/PlatformProcess.h"
#include "JsonObjectConverter.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "ObjectTools.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"
#endif

namespace
{
	const TCHAR* GeneratedTablesPackageRoot = TEXT("/Game/Table");
	const TCHAR* ManifestAssetName = TEXT("DT_MVTableManifest");
	const TCHAR* ManifestPackagePath = TEXT("/Game/Table/DT_MVTableManifest");

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
		return FString::Printf(TEXT("%s/%s"), GeneratedTablesPackageRoot, *ToTableAssetName(TableName));
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

	bool JsonObjectToCompactString(const TSharedPtr<FJsonObject>& Object, FString& OutJson)
	{
		if (!Object.IsValid())
		{
			return false;
		}

		const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutJson, 0);
		return FJsonSerializer::Serialize(Object.ToSharedRef(), Writer);
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

	UDataTable* CreateOrResetDataTable(const FString& AssetName, const FString& PackagePath, UScriptStruct* RowStruct)
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
		DataTable->MarkPackageDirty();

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

	TArray<FMVTableManifestRow> TableManifestRows;
	TArray<FString> Errors;
	const int32 ImportedCount = ImportAllJsonFiles(TableManifestRows, Errors);

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
		TEXT("Converter output:\n%s\nGenerated %d table asset(s)."),
		*ConverterLog,
		ImportedCount);

	if (!Errors.IsEmpty())
	{
		OutReport += LINE_TERMINATOR;
		OutReport += FString::Join(Errors, LINE_TERMINATOR);
	}

	return Errors.IsEmpty();
#else
	OutReport = TEXT("Table generation is editor-only.");
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

int32 UMVTableAssetGenerator::ImportAllJsonFiles(TArray<FMVTableManifestRow>& OutTableManifestRows, TArray<FString>& OutErrors)
{
	const FString JsonDir = GetJsonDir();
	if (!IFileManager::Get().DirectoryExists(*JsonDir))
	{
		OutErrors.Add(FString::Printf(TEXT("Json directory not found: %s"), *JsonDir));
		return 0;
	}

	TArray<FString> JsonFiles;
	IFileManager::Get().FindFiles(JsonFiles, *(JsonDir / TEXT("*.json")), true, false);
	JsonFiles.Sort();

	int32 ImportedCount = 0;
	for (const FString& FileName : JsonFiles)
	{
		if (FileName.Equals(TEXT("SheetRecipe.json"), ESearchCase::IgnoreCase))
		{
			continue;
		}

		ImportedCount += ImportJsonFile(JsonDir / FileName, OutTableManifestRows, OutErrors);
	}

	return ImportedCount;
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
				OutTableManifestRows.Add(ManifestRow);
				++ImportedCount;
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
			OutTableManifestRows.Add(ManifestRow);
			++ImportedCount;
		}
		else
		{
			OutErrors.Add(Error);
		}
	}

	return ImportedCount;
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
		if (!JsonValueToString(*KeyValue, KeyString) || KeyString.IsEmpty())
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
