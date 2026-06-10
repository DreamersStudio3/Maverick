// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MVEditorPathFixerLibrary.generated.h"

/**
 * 
 */
UCLASS()
class MAVERICK_API UMVEditorPathFixerLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Maverick|EditorUtils")
	static void FixBlueprintParent(UObject* BlueprintAssetToFix, UClass* NewParentClass);
	UFUNCTION(BlueprintCallable, Category = "Maverick|EditorUtils")
	static void FixAnimationBlueprintParent(UObject* BlueprintAssetToFix, UClass* NewParentClass, USkeleton* NewSkeleton);

};
