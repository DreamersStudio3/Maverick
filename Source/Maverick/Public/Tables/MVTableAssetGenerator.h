#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MVTableAssetGenerator.generated.h"

class FJsonValue;
struct FMVSheetSpec;
struct FMVTableManifestRow;

UCLASS()
class MAVERICK_API UMVTableAssetGenerator : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Maverick|Table|Editor", meta = (DisplayName = "Generate Maverick DataTables"))
	static bool GenerateDataTables(FString& OutReport);

#if WITH_EDITOR
private:
	static bool RunCsvConverter(FString& OutLog);
	static int32 ImportAllJsonFiles(TArray<struct FMVTableManifestRow>& OutTableManifestRows, TArray<FString>& OutErrors);
	static int32 ImportJsonFile(
		const FString& JsonPath,
		TArray<struct FMVTableManifestRow>& OutTableManifestRows,
		TArray<FString>& OutErrors);
	static bool BuildAndSaveDataTable(
		const FString& TableName,
		const struct FMVSheetSpec& Spec,
		const TArray<TSharedPtr<FJsonValue>>& JsonRows,
		struct FMVTableManifestRow& OutManifestRow,
		FString& OutError);
	static bool SaveManifest(const TArray<struct FMVTableManifestRow>& ManifestRows, FString& OutError);
	static void DeleteStaleGeneratedAssets(
		const TArray<struct FMVTableManifestRow>& NewManifestRows,
		TArray<FString>& OutErrors);
#endif
};
