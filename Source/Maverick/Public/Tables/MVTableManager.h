#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "Tables/MVTableTypes.h"
#include "MVTableManager.generated.h"

class UDataTable;

UCLASS()
class MAVERICK_API UMVTableManager : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	static UMVTableManager* Get(const UObject* WorldContext = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Table")
	void ReloadAllTables();

	UFUNCTION(BlueprintPure, Category = "Maverick|Table")
	bool IsLoaded() const { return bLoaded; }

	UFUNCTION(BlueprintPure, Category = "Maverick|Table")
	int32 GetTableCount() const { return LoadedTables.Num(); }

	UFUNCTION(BlueprintPure, Category = "Maverick|Table")
	bool HasTable(FName TableName) const;

	UFUNCTION(BlueprintPure, Category = "Maverick|Table")
	bool HasRow(FName TableName, const FString& RowKey) const;

	UFUNCTION(BlueprintCallable, Category = "Maverick|Table")
	TArray<FName> GetTableNames() const;

	UFUNCTION(BlueprintCallable, Category = "Maverick|Table")
	TArray<FString> GetRowKeys(FName TableName) const;

	UFUNCTION(BlueprintCallable, Category = "Maverick|Table")
	TArray<FString> GetLastLoadErrors() const { return LastLoadErrors; }

	const UDataTable* FindDataTable(FName TableName) const;
	const FMVTableManifestRow* FindManifestRow(FName TableName) const;

	template<typename RowType>
	const RowType* FindRow(FName TableName, const FString& RowKey) const
	{
		static_assert(std::is_base_of_v<FTableRowBase, RowType>, "RowType must inherit from FTableRowBase.");

		const UDataTable* DataTable = FindDataTable(TableName);
		if (!DataTable || !DataTable->GetRowStruct() || !DataTable->GetRowStruct()->IsChildOf(RowType::StaticStruct()))
		{
			return nullptr;
		}

		return DataTable->FindRow<RowType>(FName(*RowKey), TEXT("MVTableManager"), false);
	}

	template<typename RowType, typename KeyType = int32>
	bool BuildPropTable(FName TableName, TMVPropTable<RowType, KeyType>& OutTable) const
	{
		static_assert(std::is_base_of_v<FMVTableRowBase, RowType>, "RowType must inherit from FMVTableRowBase.");

		const UDataTable* DataTable = FindDataTable(TableName);
		if (!DataTable || !DataTable->GetRowStruct() || !DataTable->GetRowStruct()->IsChildOf(RowType::StaticStruct()))
		{
			return false;
		}

		OutTable.Build(DataTable);
		OutTable.PostRead();
		return true;
	}

	UFUNCTION(BlueprintCallable, Category = "Maverick|Table", meta = (DisplayName = "Get Table String"))
	bool GetString(FName TableName, const FString& RowKey, const FString& FieldName, FString& OutValue) const;

	UFUNCTION(BlueprintCallable, Category = "Maverick|Table", meta = (DisplayName = "Get Table Int"))
	bool GetInt(FName TableName, const FString& RowKey, const FString& FieldName, int32& OutValue) const;

	UFUNCTION(BlueprintCallable, Category = "Maverick|Table", meta = (DisplayName = "Get Table Float"))
	bool GetFloat(FName TableName, const FString& RowKey, const FString& FieldName, float& OutValue) const;

	UFUNCTION(BlueprintCallable, Category = "Maverick|Table", meta = (DisplayName = "Get Table Bool"))
	bool GetBool(FName TableName, const FString& RowKey, const FString& FieldName, bool& OutValue) const;

	UFUNCTION(BlueprintCallable, Category = "Maverick|Table", meta = (DisplayName = "Get Table Row JSON"))
	bool GetRowJson(FName TableName, const FString& RowKey, FString& OutValue) const;

private:
	bool LoadAllTables();
	void AddLoadError(const FString& Message);
	const uint8* FindRawRow(FName TableName, const FString& RowKey, const UScriptStruct*& OutRowStruct) const;
	const FProperty* FindProperty(FName TableName, const FString& RowKey, const FString& FieldName, const uint8*& OutRowData) const;
	bool TryGetGenericField(FName TableName, const FString& RowKey, const FString& FieldName, TSharedPtr<class FJsonValue>& OutValue) const;

private:
	UPROPERTY()
	TObjectPtr<UDataTable> ManifestTable;

	UPROPERTY()
	TMap<FName, TObjectPtr<UDataTable>> LoadedTables;

	TMap<FName, FMVTableManifestRow> ManifestRows;
	TArray<FString> LastLoadErrors;
	bool bLoaded = false;
};
