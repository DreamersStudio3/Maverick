#pragma once

#include "CoreMinimal.h"

struct FMVSheetSpec
{
	UScriptStruct* RowStruct = nullptr;
	FString KeyColumnName;
	bool bGenericFallback = false;
};

class MAVERICK_API FMVSheetSpecs
{
public:
	static const FMVSheetSpec* Find(const FString& SheetName);
	static TArray<FString> GetAllSheetNames();
	static void Invalidate();
};
