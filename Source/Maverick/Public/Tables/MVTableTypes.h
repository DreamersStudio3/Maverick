#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include <type_traits>
#include "MVTableTypes.generated.h"

class IMVPostRead
{
public:
	virtual ~IMVPostRead() = default;
	virtual void PostRead() = 0;
};

USTRUCT(BlueprintType)
struct MAVERICK_API FMVTableRowBase : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Table")
	int32 RowId = 0;

	int32 GetKey() const { return RowId; }

	virtual void PostRead() {}
};

USTRUCT(BlueprintType)
struct MAVERICK_API FMVGenericTableRow : public FMVTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table")
	FString Key;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table", meta = (MultiLine = "true"))
	FString RowJson;

	virtual void PostRead() override
	{
		LexTryParseString(RowId, *Key);
	}
};

USTRUCT(BlueprintType)
struct MAVERICK_API FMVTableManifestRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table")
	FName TableName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table")
	FString AssetPath;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table")
	FString KeyColumnName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table")
	FString RowStructName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table")
	bool bGenericFallback = false;
};

template<typename RowType, typename KeyType = int32>
class TMVPropTable : public IMVPostRead
{
public:
	static_assert(std::is_base_of_v<FMVTableRowBase, RowType>,
		"RowType must inherit from FMVTableRowBase.");

	void Build(const UDataTable* InTable)
	{
		SourceTable = InTable;
		Rows.Reset();
		Map.Reset();

		if (!InTable)
		{
			return;
		}

		const TMap<FName, uint8*>& RowMap = InTable->GetRowMap();
		Rows.Reserve(RowMap.Num());
		for (const TPair<FName, uint8*>& Pair : RowMap)
		{
			Rows.Add(reinterpret_cast<RowType*>(Pair.Value));
		}
	}

	virtual void PostRead() override
	{
		for (RowType* Row : Rows)
		{
			if (Row)
			{
				Row->PostRead();
			}
		}

		Map.Reset();
		Map.Reserve(Rows.Num());
		for (RowType* Row : Rows)
		{
			if (Row)
			{
				Map.Add(static_cast<KeyType>(Row->GetKey()), Row);
			}
		}
	}

	int32 Num() const { return Map.Num(); }
	bool Has(const KeyType& Key) const { return Map.Contains(Key); }

	const RowType* Find(const KeyType& Key) const
	{
		const RowType* const* Found = Map.Find(Key);
		return Found ? *Found : nullptr;
	}

	const TMap<KeyType, RowType*>& GetMap() const { return Map; }
	const UDataTable* GetSource() const { return SourceTable.Get(); }

private:
	TArray<RowType*> Rows;
	TMap<KeyType, RowType*> Map;
	TWeakObjectPtr<const UDataTable> SourceTable;
};
