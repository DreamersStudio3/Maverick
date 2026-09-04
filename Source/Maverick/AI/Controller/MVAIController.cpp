// Fill out your copyright notice in the Description page of Project Settings.


#include "MVAIController.h"
#include "Components/StateTreeAIComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Perception/AIPerceptionComponent.h"
#include "UObject/UnrealType.h"

AMVAIController::AMVAIController()
{
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));

	AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &AMVAIController::OnPerceptionUpdated);
}

void AMVAIController::BeginPlay()
{
	Super::BeginPlay();
	InitializeBossDebugGate();
}

void AMVAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	InitializeBossDebugGate();
}

void AMVAIController::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bBossDebugGateInitialized)
	{
		return;
	}

	APlayerController* PlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PlayerController)
	{
		return;
	}

	if (PlayerController->WasInputKeyJustPressed(EKeys::K))
	{
		SetBossPlayStartForDebug(true);
	}

	if (PlayerController->WasInputKeyJustPressed(EKeys::L))
	{
		SetBossAttackTargetForDebug(PlayerController->GetPawn());
	}
}

void AMVAIController::InitializeBossDebugGate()
{
	if (!bBossDebugGateInitialized && IsBossDebugController())
	{
		bBossDebugGateInitialized = true;
		if (UStateTreeAIComponent* StateTreeComponent = FindBossStateTreeComponent())
		{
			StateTreeComponent->StopLogic(TEXT("BossPlayStart is false"));
		}

		UE_LOG(
			LogTemp,
			Log,
			TEXT("BossStateTrace Frame=%llu Controller=%s Pawn=%s BossPlayStart=false StateTree=Stopped"),
			GFrameCounter,
			*GetNameSafe(this),
			*GetNameSafe(GetPawn()));
	}
}

void AMVAIController::SetBossPlayStartForDebug(const bool bInBossPlayStart)
{
	if (!IsBossDebugController() || BossPlayStart == bInBossPlayStart)
	{
		return;
	}

	BossPlayStart = bInBossPlayStart;
	if (UStateTreeAIComponent* StateTreeComponent = FindBossStateTreeComponent())
	{
		if (BossPlayStart)
		{
			StateTreeComponent->StartLogic();
		}
		else
		{
			StateTreeComponent->StopLogic(TEXT("BossPlayStart is false"));
		}
	}

	UE_LOG(
		LogTemp,
		Log,
		TEXT("BossStateTrace Frame=%llu Controller=%s Pawn=%s BossPlayStart=%s StateTree=%s"),
		GFrameCounter,
		*GetNameSafe(this),
		*GetNameSafe(GetPawn()),
		BossPlayStart ? TEXT("true") : TEXT("false"),
		BossPlayStart ? TEXT("Started") : TEXT("Stopped"));
}

void AMVAIController::SetBossAttackTargetForDebug(AActor* InAttackTarget)
{
	if (!IsBossDebugController())
	{
		return;
	}

	TargetActor = InAttackTarget;
	const bool bTargetWritten = WriteBossAttackTarget(InAttackTarget);
	UE_LOG(
		LogTemp,
		Log,
		TEXT("BossStateTrace Frame=%llu Controller=%s Pawn=%s AttackTarget=%s PropertyWrite=%s"),
		GFrameCounter,
		*GetNameSafe(this),
		*GetNameSafe(GetPawn()),
		*GetNameSafe(InAttackTarget),
		bTargetWritten ? TEXT("Succeeded") : TEXT("Failed"));
}

bool AMVAIController::IsBossDebugController() const
{
	const APawn* ControlledPawn = GetPawn();
	return ControlledPawn
		&& FindFProperty<FObjectPropertyBase>(ControlledPawn->GetClass(), TEXT("AttackTarget"));
}

UStateTreeAIComponent* AMVAIController::FindBossStateTreeComponent() const
{
	return FindComponentByClass<UStateTreeAIComponent>();
}

bool AMVAIController::WriteBossAttackTarget(AActor* InAttackTarget) const
{
	APawn* ControlledPawn = GetPawn();
	FObjectPropertyBase* AttackTargetProperty = ControlledPawn
		? FindFProperty<FObjectPropertyBase>(ControlledPawn->GetClass(), TEXT("AttackTarget"))
		: nullptr;
	if (!AttackTargetProperty
		|| (InAttackTarget && !InAttackTarget->IsA(AttackTargetProperty->PropertyClass)))
	{
		return false;
	}

	AttackTargetProperty->SetObjectPropertyValue_InContainer(ControlledPawn, InAttackTarget);
	return true;
}

void AMVAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
	for (AActor* Actor : UpdatedActors)
	{
		if (!Actor)
		{
			continue;
		}

		const FActorPerceptionInfo* info = AIPerceptionComponent->GetActorInfo(*Actor);

		if (info->LastSensedStimuli.Num() > 0 && info->LastSensedStimuli[0].WasSuccessfullySensed())
		{
			TargetActor = Actor; //적 발견 
		}
		else
		{
			TargetActor = nullptr; //적 놓침
		}
		
	}
}
