// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/NPC/Enemy/MVEnemy.h"

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
		MainHUD->HideBossHPBar();
	}

	BoundBossHUD.Reset();
}

void AMVEnemy::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	HideBoundBossHUD();

	Super::EndPlay(EndPlayReason);
}

void AMVEnemy::BindBossHUDToMainHUD()
{
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

		GetWorldTimerManager().ClearTimer(BossHUDBindRetryTimerHandle);
	}
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
