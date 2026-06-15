// Fill out your copyright notice in the Description page of Project Settings.


#include "MVAIController.h"
#include "Perception/AIPerceptionComponent.h"

AMVAIController::AMVAIController()
{
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));

	AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &AMVAIController::OnPerceptionUpdated);
}

void AMVAIController::BeginPlay()
{
	Super::BeginPlay();
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
