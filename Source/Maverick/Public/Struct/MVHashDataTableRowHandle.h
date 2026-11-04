#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"

#include "MVHashDataTableRowHandle.generated.h"

// A wrapper struct for FDataTableRowHandle that can be used in TSet and TMap as a key.
USTRUCT(BlueprintType)
struct MAVERICK_API FMVHashDataTableRowHandle
{
	GENERATED_BODY()

	FMVHashDataTableRowHandle()
		: RowHandle()
	{
	}
	
	FMVHashDataTableRowHandle(const FDataTableRowHandle& Input)
		: RowHandle(Input)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DataTable")
	FDataTableRowHandle RowHandle;

	bool operator==(const FMVHashDataTableRowHandle& Other) const
	{
		return RowHandle == Other.RowHandle;
	}
	
};

// Hash function for FMVHashDataTableRowHandle to be used in TSet and TMap
FORCEINLINE uint32 GetTypeHash(const FMVHashDataTableRowHandle& Input)
{
	uint32 Hash = 0;
	Hash = HashCombine(Hash, GetTypeHash(Input.RowHandle.DataTable));
	Hash = HashCombine(Hash, GetTypeHash(Input.RowHandle.RowName));
	return Hash;
}