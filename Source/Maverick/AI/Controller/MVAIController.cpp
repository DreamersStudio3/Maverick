// Fill out your copyright notice in the Description page of Project Settings.


#include "MVAIController.h"
#include "Components/StateTreeAIComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Perception/AIPerceptionComponent.h"
#include "UI/HUD/MVMainHUDWidget.h"
#include "UI/System/MVUISubsystem.h"
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
	NextBossAttackChoiceIndex = 0;
	Super::OnPossess(InPawn);
	InitializeBossDebugGate();
}

void AMVAIController::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bBossDebugGateInitialized || bBossIntroBlocked)
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
		SetBossCombatEnabled(true, TargetActor);
		if (UGameInstance* GameInstance = GetGameInstance())
		{
			if (UMVUISubsystem* UISubsystem = GameInstance->GetSubsystem<UMVUISubsystem>())
			{
				if (UMVMainHUDWidget* MainHUD = Cast<UMVMainHUDWidget>(UISubsystem->GetMainHUD()))
				{
					MainHUD->MarkBossPatrolComplete();

				}
			}
		}
	}

	if (PlayerController->WasInputKeyJustPressed(EKeys::L))
	{
		SetBossCombatEnabled(true, PlayerController->GetPawn());
		if (UGameInstance* GameInstance = GetGameInstance())
		{
			if (UMVUISubsystem* UISubsystem = GameInstance->GetSubsystem<UMVUISubsystem>())
			{
				if (UMVMainHUDWidget* MainHUD = Cast<UMVMainHUDWidget>(UISubsystem->GetMainHUD()))
				{
					MainHUD->MarkBossPlayStarted();

				}
			}
		}
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

void AMVAIController::SetBossIntroBlocked(const bool bBlocked)
{
	bBossIntroBlocked = bBlocked;
	if (bBlocked) SetBossCombatEnabled(false, nullptr);
}

void AMVAIController::SetBossCombatEnabled(const bool bEnabled, AActor* CombatTarget)
{
	if (!IsBossDebugController() || (bEnabled && bBossIntroBlocked)) return;
	OnBossCombatControlRequested.Broadcast();
	BossPlayStart = bEnabled;
	TargetActor = bEnabled ? CombatTarget : nullptr;
	WriteBossAttackTarget(TargetActor);
	if (!bEnabled) StopMovement();
	if (UStateTreeAIComponent* Tree = FindBossStateTreeComponent())
	{
		if (bEnabled)
		{
			if (!Tree->IsRunning()) Tree->StartLogic();
		}
		else Tree->StopLogic(TEXT("Boss intro / combat disabled"));
	}
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
