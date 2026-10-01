#include "Editor/MaverickBossStateTreeCommandlet.h"

#include "AI/AITask/FMVBossExecuteAttackTask.h"
#include "AI/AITask/MVBossSelectAttackTask.h"
#include "AI/Controller/MVAIController.h"
#include "AssetToolsModule.h"
#include "Engine/AssetManager.h"
#include "Engine/DataTable.h"
#include "AI/AITask/FMVBossExecuteAttackTask.h"
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
void BossSelectAttackDumpState(UStateTreeState* State)
{
	UE_LOG(LogTemp, Display, TEXT("[BossSelectSetup] State=%s Tasks=%d Selection=%d"),
		*State->Name.ToString(), State->Tasks.Num(), static_cast<int32>(State->SelectionBehavior));
	for (const FStateTreeTransition& Transition : State->Transitions)
	{
		UE_LOG(LogTemp, Display, TEXT("[BossSelectSetup] Transition=%s -> %s Type=%d Trigger=%d"),
			*State->Name.ToString(), *Transition.State.Name.ToString(),
			static_cast<int32>(Transition.State.LinkType), static_cast<int32>(Transition.Trigger));
	}
	for (UStateTreeState* Child : State->Children)
	{
		if (Child) BossSelectAttackDumpState(Child);
	}
}

bool BossSelectAttackConfigure()
{
	UStateTree* Tree = LoadObject<UStateTree>(nullptr,
		TEXT("/Game/Characters/NPC/Boss/TutorialBoss/ST_TutorialBoss_Attack.ST_TutorialBoss_Attack"));
	FObjectProperty* Property = FindFProperty<FObjectProperty>(UStateTree::StaticClass(), TEXT("EditorData"));
	UStateTreeEditorData* Data = Tree && Property
		? Cast<UStateTreeEditorData>(Property->GetObjectPropertyValue_InContainer(Tree)) : nullptr;
	if (!Data) return false;
	UStateTreeState* BossAttack = nullptr;
	TFunction<void(UStateTreeState*)> FindState = [&](UStateTreeState* State)
	{
		if (State->Name == TEXT("BossAttack")) BossAttack = State;
		for (UStateTreeState* Child : State->Children) if (Child) FindState(Child);
	};
	for (UStateTreeState* Root : Data->SubTrees)
	{
		BossSelectAttackDumpState(Root);
		FindState(Root);
	}
	if (!BossAttack) return false;
	TArray<FMVBossAttackChoice> Choices;
	for (UStateTreeState* Child : BossAttack->Children)
	{
		if (!Child) continue;
		bool bAttackTask = false;
		for (const FStateTreeEditorNode& Node : Child->Tasks)
		{
			bAttackTask |= Node.Node.GetScriptStruct() == FMVBossExecuteAttackTask::StaticStruct();
		}
		if (!bAttackTask) continue;
		FMVBossAttackChoice& Choice = Choices.AddDefaulted_GetRef();
		Choice.AttackName = Child->Name;
		Choice.AttackState = Child->GetLinkToState();
	}
	if (Choices.IsEmpty()) return false;
	Tree->Modify();
	BossAttack->Modify();
	BossAttack->Tasks.RemoveAll([](const FStateTreeEditorNode& Node)
	{
		return Node.Node.GetScriptStruct() == FMVBossExecuteAttackTask::StaticStruct()
			|| Node.Node.GetScriptStruct() == FMVBossSelectAttackTask::StaticStruct();
	});
	BossAttack->SelectionBehavior = EStateTreeStateSelectionBehavior::TryEnterState;
	auto& Selector = BossAttack->AddTask<FMVBossSelectAttackTask>();
	Selector.GetInstanceData().Choices = Choices;
	UStateTreeEditingSubsystem::ValidateStateTree(Tree);
	FStateTreeCompilerLog Log;
	if (!UStateTreeEditingSubsystem::CompileStateTree(Tree, Log)) return false;
	FString Filename;
	if (!FPackageName::TryConvertLongPackageNameToFilename(Tree->GetOutermost()->GetName(),
		Filename, FPackageName::GetAssetPackageExtension())) return false;
	FSavePackageArgs Args;
	Args.TopLevelFlags = RF_Public | RF_Standalone;
	Args.Error = GError;
	if (!UPackage::SavePackage(Tree->GetOutermost(), Tree, *Filename, Args)) return false;
	UE_LOG(LogTemp, Display, TEXT("[BossSelectSetup] Saved: Choices=%d Asset=%s"), Choices.Num(), *Tree->GetPathName());
	return true;
}

void PopulateTutorialBossAttackTable()
{
	UDataTable* AttackTable = LoadObject<UDataTable>(
		nullptr,
		TEXT("/Game/Characters/NPC/Boss/TutorialBoss/DT_NewDataTable.DT_NewDataTable"));
	if (!AttackTable)
	{
		UE_LOG(LogTemp, Error, TEXT("TutorialBoss attack table load failed"));
		return;
	}

	if (AttackTable->GetRowStruct() != FMVTutorialBossSkillRow::StaticStruct())
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("TutorialBoss attack table row struct must be FMVTutorialBossSkillRow. Actual=%s"),
			AttackTable->GetRowStruct() ? *AttackTable->GetRowStruct()->GetPathName() : TEXT("None"));
		return;
	}

	struct FAttackDefinition
	{
		FName RowName;
		const TCHAR* MontagePath;
		float PlayRate;
	};

	const FAttackDefinition Definitions[] = {
		{TEXT("BasicAttack1"), TEXT("/Game/ArtAssets/Animations/Enemy/NamlessPuppet/AttackMontage/AM_E1_SK1.AM_E1_SK1"), 1.0f},
		{TEXT("BasicAttack2"), TEXT("/Game/ArtAssets/Animations/Enemy/NamlessPuppet/AttackMontage/AM_E1_SK2.AM_E1_SK2"), 1.0f},
		{TEXT("BasicAttack3"), TEXT("/Game/ArtAssets/Animations/Enemy/NamlessPuppet/AttackMontage/AM_E1_SK3.AM_E1_SK3"), 1.0f},
		{TEXT("SkillQ"), TEXT("/Game/ArtAssets/Animations/Enemy/NamlessPuppet/AttackMontage/AM_E1_SK4.AM_E1_SK4"), 1.0f},
		{TEXT("SkillW"), TEXT("/Game/ArtAssets/Animations/Enemy/NamlessPuppet/AttackMontage/AM_E1_SK5.AM_E1_SK5"), 1.0f},
		{TEXT("Resonance"), TEXT("/Game/ArtAssets/Animations/Enemy/NamlessPuppet/AttackMontage/AM_E1_SK6.AM_E1_SK6"), 1.0f},
	};

	for (const FAttackDefinition& Definition : Definitions)
	{
		FMVTutorialBossSkillRow Row;
		Row.Montage = TSoftObjectPtr<UAnimMontage>(FSoftObjectPath(Definition.MontagePath));
		Row.PlayRate = Definition.PlayRate;
		Row.bStopOnExit = true;
		AttackTable->AddRow(Definition.RowName, Row);
	}

	AttackTable->Modify();
	AttackTable->MarkPackageDirty();
	UPackage* Package = AttackTable->GetOutermost();
	FString Filename;
	if (!FPackageName::TryConvertLongPackageNameToFilename(
		Package->GetName(), Filename, FPackageName::GetAssetPackageExtension()))
	{
		UE_LOG(LogTemp, Error, TEXT("TutorialBoss attack table package path conversion failed"));
		return;
	}

	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
	SaveArgs.Error = GError;
	if (!UPackage::SavePackage(Package, AttackTable, *Filename, SaveArgs))
	{
		UE_LOG(LogTemp, Error, TEXT("TutorialBoss attack table save failed: %s"), *Filename);
		return;
	}

	UE_LOG(LogTemp, Display, TEXT("TutorialBoss attack table populated: %s"), *Filename);
}

void AddBossAttackState(const TCHAR* AssetPath, const FName RowName)
{
	const bool bTutorialBoss = AssetPath && FCString::Stristr(AssetPath, TEXT("TutorialBoss")) != nullptr;
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
		TaskNode.GetInstanceData().AttackRow.DataTable = LoadObject<UDataTable>(
			nullptr,
			bTutorialBoss
				? TEXT("/Game/Characters/NPC/Boss/TutorialBoss/DT_NewDataTable.DT_NewDataTable")
				: TEXT("/Game/Table/Attack/NPC/E1/DT_E1_Attack.DT_E1_Attack"));
		TaskNode.GetInstanceData().AttackRow.RowName = bTutorialBoss ? TEXT("BasicAttack1") : RowName;
		TaskNode.GetInstanceData().AttackRange = 300.0f;
	}

	const TArray<TPair<FName, FName>> AttackPatterns = {
		{TEXT("BasicAttack1"), TEXT("HeavyAttack1")},
		{TEXT("BasicAttack2"), TEXT("HeavyAttack2")},
		{TEXT("SkillQ"), TEXT("HeavyAttack")},
		{TEXT("SkillW"), TEXT("HeavyAttack3")},
		{TEXT("Resonance"), TEXT("HeavyAttack4")},
	};

	UDataTable* AttackTable = LoadObject<UDataTable>(
		nullptr,
		AssetPath && FCString::Stristr(AssetPath, TEXT("TutorialBoss"))
			? TEXT("/Game/Characters/NPC/Boss/TutorialBoss/DT_NewDataTable.DT_NewDataTable")
			: TEXT("/Game/Table/Attack/NPC/E1/DT_E1_Attack.DT_E1_Attack"));
	for (const TPair<FName, FName>& Pattern : AttackPatterns)
	{
		const FName ResolvedRowName = bTutorialBoss ? Pattern.Key : Pattern.Value;
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
			PatternTask.GetInstanceData().AttackRow.RowName = ResolvedRowName;
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
	if (FParse::Param(*Params, TEXT("SelectAttackOnly")))
	{
		return BossSelectAttackConfigure() ? 0 : 1;
	}
	PopulateTutorialBossAttackTable();
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
