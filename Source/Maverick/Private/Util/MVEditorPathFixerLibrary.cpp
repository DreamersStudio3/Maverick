// Fill out your copyright notice in the Description page of Project Settings.


#include "Util/MVEditorPathFixerLibrary.h"

#if WITH_EDITOR
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Animation/AnimBlueprint.h"
#include "Engine/Blueprint.h"
#include "K2Node_Variable.h"
#endif

void UMVEditorPathFixerLibrary::FixBlueprintParent(UObject* BlueprintAssetToFix, UClass* NewParentClass)
{
#if WITH_EDITOR
	UBlueprint* BlueprintToFix = Cast<UBlueprint>(BlueprintAssetToFix);

	if (!BlueprintToFix || !NewParentClass)
	{
		UE_LOG(LogTemp, Error, TEXT("FixBlueprintParent: Invalid Blueprint or Parent Class Provided"));
		return;
	}

	// Reparent
	BlueprintToFix->ParentClass = NewParentClass;
	

	//// Re-Link Variables
	//// Call All Graphs in Blueprint(Eventgraph, functions, macros...)
	//TArray<UEdGraph*> AllGraphs;
	//BlueprintToFix->GetAllGraphs(AllGraphs);

	//UClass* NewScope = BlueprintAssetToFix->StaticClass();
	//for (UEdGraph* Graph : AllGraphs)
	//{
	//	// Iterate All Nodes in Graph
	//	for (UEdGraphNode* Node : Graph->Nodes)
	//	{
	//		if (UK2Node_Variable* VarNode = Cast<UK2Node_Variable>(Node))
	//		{
	//			FName VarName = VarNode->VariableReference.GetMemberName();

	//			// Manually Inject the new scope to break the link with the old project
	//			VarNode->VariableReference.SetExternalMember(VarName, );



	//			// Reconstruct the node to refresh the pins and data types
	//			VarNode->ReconstructNode();

	//		}

	//	}

	//}


	// 
	BlueprintToFix->Status = BS_Dirty;


	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(BlueprintToFix);

	FKismetEditorUtilities::CompileBlueprint(BlueprintToFix, EBlueprintCompileOptions::None);

	BlueprintToFix->MarkPackageDirty();

	UE_LOG(
		LogTemp,
		Log,
		TEXT("SuccessFully Reparented %s to %s. Please Save All in the Content Browser"),
		*BlueprintToFix->GetName(), *NewParentClass->GetName()
	);
	FBlueprintEditorUtils::RefreshVariables(BlueprintToFix);

#endif
}

void UMVEditorPathFixerLibrary::FixAnimationBlueprintParent(UObject* BlueprintAssetToFix, UClass* NewParentClass, USkeleton* NewSkeleton)
{
#if WITH_EDITOR
	UAnimBlueprint* AnimBlueprintToFix = Cast<UAnimBlueprint>(BlueprintAssetToFix);

	if (!AnimBlueprintToFix || !NewParentClass)
	{
		UE_LOG(LogTemp, Error, TEXT("FixAnimationBlueprintParent: Invalid Blueprint or Parent Class Provided"));
		return;
	}

	if (!NewSkeleton)
	{
		UE_LOG(
			LogTemp, 
			Error, 
			TEXT("FixAnimationBlueprintParent(%s): Attempt to set to Invalid Skeleton"),
			*BlueprintAssetToFix->GetName()
		);
		return;
	}

	AnimBlueprintToFix->TargetSkeleton = NewSkeleton;
	UE_LOG(
		LogTemp, 
		Log, 
		TEXT("Updating Skeleton for %s to %s"), 
		*AnimBlueprintToFix->GetName(),
		*NewSkeleton->GetName());

	AnimBlueprintToFix->ParentClass = NewParentClass;

	AnimBlueprintToFix->Status = BS_Dirty;

	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(AnimBlueprintToFix);

	FKismetEditorUtilities::CompileBlueprint(AnimBlueprintToFix, EBlueprintCompileOptions::None);

	AnimBlueprintToFix->MarkPackageDirty();

	UE_LOG(
		LogTemp,
		Log,
		TEXT("SuccessFully Reparented %s to %s. Please Save All in the Content Browser"),
		*AnimBlueprintToFix->GetName(), *NewParentClass->GetName()
	);


#endif

}
