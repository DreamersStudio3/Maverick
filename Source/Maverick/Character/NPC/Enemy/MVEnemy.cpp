// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/NPC/Enemy/MVEnemy.h"

#include "AI/Controller/MVAIController.h"
#include "Components/MVStatComponent.h"
#include "Engine/World.h"
#include "Tags/MVGameplayTags.h"
#include "TargetComponent.h"
#include "TimerManager.h"
#include "UI/HUD/MVMainHUDWidget.h"
#include "UI/System/MVUISubsystem.h"

AMVEnemy::AMVEnemy()
{
	CreateDefaultSubobject<UTargetComponent>(TEXT("TargetComponent"));
	CharacterIndexCode = MVGameplayTags::Character_NPC_Enemy_E1;
}

void AMVEnemy::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().SetTimer(
		BossHUDBindRetryTimerHandle,
		this,
		&AMVEnemy::BindBossHUDToMainHUD,
		0.1f,
		true,
		0.0f);
}

FRotator AMVEnemy::GetTargetRotation() const
{
	FRotator TargetRotation = GetActorRotation();
	if (!IsValid(AttackTarget.Get()))
	{
		return TargetRotation;
	}

	FVector TargetDirection = AttackTarget->GetActorLocation() - GetActorLocation();
	TargetDirection.Z = 0.0f;
	if (!TargetDirection.IsNearlyZero())
	{
		TargetRotation.Yaw = TargetDirection.Rotation().Yaw;
	}

	return TargetRotation;
}

void AMVEnemy::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	HideBoundBossHUD();

	Super::EndPlay(EndPlayReason);
}

void AMVEnemy::BindBossHUDToMainHUD()
{
	const AMVAIController* AI = Cast<AMVAIController>(GetController());
	const APawn* Player = AI ? Cast<APawn>(AI->TargetActor) : nullptr;
	if (!StatComponent || StatComponent->IsDead() || !Player || !Player->IsPlayerControlled())
	{
		if (auto* HUD = BoundBossHUD.Get(); HUD && HUD->IsShowingBoss(StatComponent)) HUD->HideBossHPBar();
		BoundBossHUD.Reset();
		return;
	}
	if (!StatComponent)
	{
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	UMVUISubsystem* UISubsystem = GameInstance ? GameInstance->GetSubsystem<UMVUISubsystem>() : nullptr;
	UMVMainHUDWidget* MainHUD = UISubsystem ? Cast<UMVMainHUDWidget>(UISubsystem->GetMainHUD()) : nullptr;

	if (MainHUD)
	{
		if (BoundBossHUD.Get() != MainHUD)
		{
			MainHUD->BindBossStatus(StatComponent, FText::FromString(GetName()));
			BoundBossHUD = MainHUD;
		}


	}
}

void AMVEnemy::HideBoundBossHUD()
{
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(BossHUDBindRetryTimerHandle);
	}

	if (World && World->bIsTearingDown)
	{
		BoundBossHUD.Reset();
		return;
	}

	if (UMVMainHUDWidget* MainHUD = BoundBossHUD.Get())
	{
		if (MainHUD->IsShowingBoss(StatComponent)) MainHUD->HideBossHPBar();
	}

	BoundBossHUD.Reset();
}

void AMVEnemy::BindDamageHandlers()
{
	if (StatComponent)
	{
		OnDamaged.RemoveDynamic(StatComponent, &UMVStatComponent::HandleDamaged);
		OnDamaged.AddUniqueDynamic(StatComponent, &UMVStatComponent::HandleDamaged);

	}

	OnDamaged.RemoveDynamic(this, &AMVEnemy::HandleEnemyDamaged);
	OnDamaged.AddUniqueDynamic(this, &AMVEnemy::HandleEnemyDamaged);
}

void AMVEnemy::HandleEnemyDamaged(const FMVResolvedHitData& HitData)
{
	OnEnemyDamaged.Broadcast(HitData);
}
