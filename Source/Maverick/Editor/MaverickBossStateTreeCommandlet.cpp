#include "Editor/MaverickBossStateTreeCommandlet.h"

#include "AI/AITask/FMVBossExecuteAttackTask.h"
#include "AI/Controller/MVAIController.h"
#include "AssetToolsModule.h"
#include "Engine/AssetManager.h"
#include "StateTree.h"
#include "StateTreeEditorData.h"
#include "StateTreeEditingSubsystem.h"
#include "StateTreeCompilerLog.h"
#include "StateTreeState.h"
#include "UObject/SavePackage.h"
#include "UObject/UnrealType.h"
#include "Components/StateTreeAIComponent.h"

namespace
{
void AddBossAttackState(const TCHAR* AssetPath, const FName RowName)
{
	UStateTree* StateTree = LoadObject<UStateTree>(nullptr, AssetPath);
	if (!StateTree)
	{
		UE_LOG(LogTemp, Error, TEXT("Boss StateTree load failed: %s"), AssetPath);
		return;
	}

	FObjectProperty* EditorDataProperty = FindFProperty<FObjectProperty>(UStateTree::StaticClass(), TEXT("EditorData"));
	UStateTreeEditorData* EditorData = EditorDataProperty
		? Cast<UStateTreeEditorData>(EditorDataProperty->GetObjectPropertyValue_InContainer(StateTree))
		: nullptr;
	if (!EditorData || EditorData->SubTrees.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("Boss StateTree editor data missing: %s"), AssetPath);
		return;
	}

	UStateTreeState* RootState = EditorData->SubTrees[0];
	UStateTreeState* AttackState = nullptr;
	for (UStateTreeState* Child : RootState->Children)
	{
		if (Child && Child->Name == TEXT("BossAttack"))
		{
			AttackState = Child;
			break;
		}
	}

	if (!AttackState)
	{
		AttackState = &RootState->AddChildState(TEXT("BossAttack"));
	}

	if (AttackState->Tasks.IsEmpty())
	{
		TStateTreeEditorNode<FMVBossExecuteAttackTask>& TaskNode = AttackState->AddTask<FMVBossExecuteAttackTask>();
		TaskNode.GetInstanceData().AttackRow.DataTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/Table/Attack/NPC/E1/DT_E1_Attack.DT_E1_Attack"));
		TaskNode.GetInstanceData().AttackRow.RowName = RowName;
		TaskNode.GetInstanceData().AttackRange = 300.0f;
	}

	const TArray<TPair<FName, FName>> AttackPatterns = {
		{TEXT("BasicAttack1"), TEXT("HeavyAttack1")},
		{TEXT("BasicAttack2"), TEXT("HeavyAttack2")},
		{TEXT("SkillQ"), TEXT("HeavyAttack")},
		{TEXT("SkillW"), TEXT("HeavyAttack3")},
		{TEXT("Resonance"), TEXT("HeavyAttack4")},
	};

	UDataTable* AttackTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/Table/Attack/NPC/E1/DT_E1_Attack.DT_E1_Attack"));
	for (const TPair<FName, FName>& Pattern : AttackPatterns)
	{
		UStateTreeState* PatternState = nullptr;
		for (UStateTreeState* Child : AttackState->Children)
		{
			if (Child && Child->Name == Pattern.Key)
			{
				PatternState = Child;
				break;
			}
		}

		if (!PatternState)
		{
			PatternState = &AttackState->AddChildState(Pattern.Key);
		}

		if (PatternState->Tasks.IsEmpty())
		{
			TStateTreeEditorNode<FMVBossExecuteAttackTask>& PatternTask = PatternState->AddTask<FMVBossExecuteAttackTask>();
			PatternTask.GetInstanceData().AttackRow.DataTable = AttackTable;
			PatternTask.GetInstanceData().AttackRow.RowName = Pattern.Value;
			PatternTask.GetInstanceData().AttackRange = 300.0f;
		}
	}

	UStateTreeEditingSubsystem::ValidateStateTree(StateTree);
	FStateTreeCompilerLog CompilerLog;
	if (!UStateTreeEditingSubsystem::CompileStateTree(StateTree, CompilerLog))
	{
		UE_LOG(LogTemp, Error, TEXT("Boss StateTree compile failed: %s"), AssetPath);
		return;
	}

	StateTree->MarkPackageDirty();
	UPackage* Package = StateTree->GetOutermost();
	FString Filename;
	FPackageName::TryConvertLongPackageNameToFilename(
		Package->GetName(),
		Filename,
		FPackageName::GetAssetPackageExtension());
	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
	SaveArgs.Error = GError;
	UPackage::SavePackage(Package, StateTree, *Filename, SaveArgs);
	UE_LOG(LogTemp, Display, TEXT("Boss StateTree updated: %s"), AssetPath);
}
}

UMaverickBossStateTreeCommandlet::UMaverickBossStateTreeCommandlet()
{
	IsServer = false;
	IsClient = false;
	IsEditor = true;
	LogToConsole = true;
}

int32 UMaverickBossStateTreeCommandlet::Main(const FString& Params)
{
	AddBossAttackState(TEXT("/Game/Characters/NPC/Boss/TutorialBoss/ST_TutorialBoss_Attack.ST_TutorialBoss_Attack"), TEXT("HeavyAttack1"));
	AddBossAttackState(TEXT("/Game/Characters/NPC/Boss/OtherBoss/ST_OtherBoss_Attack.ST_OtherBoss_Attack"), TEXT("HeavyAttack1"));

	UClass* ControllerClass = LoadObject<UClass>(nullptr, TEXT("/Game/Characters/NPC/Boss/TutorialBoss/BP_TutorialBossAIController.BP_TutorialBossAIController_C"));
	UStateTree* TutorialStateTree = LoadObject<UStateTree>(nullptr, TEXT("/Game/Characters/NPC/Boss/TutorialBoss/ST_TutorialBossAIStateTree.ST_TutorialBossAIStateTree"));
	if (ControllerClass && TutorialStateTree)
	{
		AActor* ControllerCDO = Cast<AActor>(ControllerClass->GetDefaultObject());
		TArray<UActorComponent*> Components;
		ControllerCDO->GetComponents(Components);
		for (UActorComponent* Component : Components)
		{
			if (UStateTreeAIComponent* StateTreeComponent = Cast<UStateTreeAIComponent>(Component))
			{
				StateTreeComponent->SetStateTree(TutorialStateTree);
				StateTreeComponent->MarkPackageDirty();
				UE_LOG(LogTemp, Display, TEXT("Tutorial StateTree assigned to controller"));
			}
		}
	}

	UClass* BossClass = LoadObject<UClass>(nullptr, TEXT("/Game/Characters/NPC/Boss/TutorialBoss/BP_TutorialBoss.BP_TutorialBoss_C"));
	if (BossClass && TutorialStateTree)
	{
		AActor* BossCDO = Cast<AActor>(BossClass->GetDefaultObject());
		TArray<UActorComponent*> Components;
		BossCDO->GetComponents(Components);
		for (UActorComponent* Component : Components)
		{
			if (UStateTreeAIComponent* StateTreeComponent = Cast<UStateTreeAIComponent>(Component))
			{
				StateTreeComponent->SetStateTree(TutorialStateTree);
				StateTreeComponent->MarkPackageDirty();
				UE_LOG(LogTemp, Display, TEXT("Tutorial StateTree assigned to boss"));
			}
		}
	}
	return 0;
}
