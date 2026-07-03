// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/NPC/Enemy/MVEnemy.h"

#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Character/NPC/Enemy/MVEnemyWeapon.h"
#include "Components/MVHitReactionComponent.h"
#include "Components/MVStatComponent.h"
#include "TimerManager.h"
#include "UI/HUD/MVMainHUDWidget.h"
#include "UI/System/MVUISubsystem.h"

AMVEnemy::AMVEnemy()
{
}

void AMVEnemy::BeginPlay()
{
	Super::BeginPlay();

	BossHUDBindAttemptsRemaining = 20;
	ScheduleBossHUDBindRetry(0.0f);

	if (!WeaponClass || !GetWorld() || !GetMesh())
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;

	WeaponActor = GetWorld()->SpawnActor<AMVEnemyWeapon>(
		WeaponClass,
		GetActorTransform(),
		SpawnParams);

	if (!WeaponActor)
	{
		return;
	}

	if (bUseDualWeapon)
	{
		WeaponActor->AttachDualToHands(GetMesh());
	}
	else
	{
		WeaponActor->AttachCombinedToHand(GetMesh());
	}
}

bool AMVEnemy::Attack(const EMVAttackDirection AttackDirection)
{
	int32 UnusedAttackInstanceId = INDEX_NONE;
	return Attack(AttackDirection, UnusedAttackInstanceId);
}

bool AMVEnemy::Attack(const EMVAttackDirection AttackDirection, int32& OutAttackInstanceId)
{
	OutAttackInstanceId = INDEX_NONE;

	UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;


	if (AnimInstance && AttackMontage)
	{
		FName SectionName;


		switch (AttackDirection)
		{
		case EMVAttackDirection::Forward: SectionName = FName("Front");
			break;
		case EMVAttackDirection::Backward: SectionName = FName("Back");
			break;
		case EMVAttackDirection::Left: SectionName = FName("Left");
			break;
		case EMVAttackDirection::Right: SectionName = FName("Right");
			break;
		default: SectionName = FName("Front");
			break;
		}

		const float PlayDuration = AnimInstance->Montage_Play(AttackMontage);
		if (PlayDuration <= 0.0f)
		{
			return false;
		}

		OutAttackInstanceId = ++NextAttackInstanceId;

		if (!SectionName.IsNone())
		{
			AnimInstance->Montage_JumpToSection(SectionName, AttackMontage);
		}

		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &AMVEnemy::HandleAttackMontageEnded, OutAttackInstanceId);
		AnimInstance->Montage_SetEndDelegate(EndDelegate, AttackMontage);

		return true;
	}

	return false;
}

void AMVEnemy::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(BossHUDBindRetryTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void AMVEnemy::ScheduleBossHUDBindRetry(float DelaySeconds)
{
	UWorld* World = GetWorld();
	if (!World || BossHUDBindAttemptsRemaining <= 0)
	{
		return;
	}

	FTimerDelegate BossHUDBindDelegate;
	BossHUDBindDelegate.BindUObject(this, &AMVEnemy::BindBossHUDToMainHUD);
	if (DelaySeconds <= 0.0f)
	{
		World->GetTimerManager().SetTimerForNextTick(BossHUDBindDelegate);
		return;
	}

	World->GetTimerManager().SetTimer(
		BossHUDBindRetryTimerHandle,
		BossHUDBindDelegate,
		DelaySeconds,
		false);
}

void AMVEnemy::BindBossHUDToMainHUD()
{
	--BossHUDBindAttemptsRemaining;

	if (!StatComponent)
	{
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	UMVUISubsystem* UISubsystem = GameInstance ? GameInstance->GetSubsystem<UMVUISubsystem>() : nullptr;
	UMVMainHUDWidget* MainHUD = UISubsystem ? Cast<UMVMainHUDWidget>(UISubsystem->GetMainHUD()) : nullptr;

	if (MainHUD && BoundBossHUD.Get() != MainHUD)
	{
		MainHUD->BindBossStatus(StatComponent, FText::FromString(GetName()));
		BoundBossHUD = MainHUD;
	}

	if (BossHUDBindAttemptsRemaining > 0)
	{
		ScheduleBossHUDBindRetry(0.1f);
	}
}

void AMVEnemy::BindDamageHandlers()
{
	if (StatComponent)
	{
		OnDamaged.RemoveDynamic(StatComponent, &UMVStatComponent::HandleDamaged);
		OnDamaged.AddUniqueDynamic(StatComponent, &UMVStatComponent::HandleDamaged);
	}

	if (HitReactionComponent)
	{
		OnDamaged.RemoveDynamic(HitReactionComponent, &UMVHitReactionComponent::HandleDamaged);
	}

	OnDamaged.RemoveDynamic(this, &AMVEnemy::HandleEnemyDamaged);
	OnDamaged.AddUniqueDynamic(this, &AMVEnemy::HandleEnemyDamaged);
}

void AMVEnemy::HandleAttackMontageEnded(UAnimMontage* Montage, const bool bInterrupted, const int32 AttackInstanceId)
{
	if (Montage != AttackMontage || AttackInstanceId == INDEX_NONE)
	{
		return;
	}

	OnAttackMontageEnded.Broadcast(AttackInstanceId, Montage, bInterrupted);
}

void AMVEnemy::HandleEnemyDamaged(const FMVResolvedHitData& HitData)
{
	OnEnemyDamaged.Broadcast(HitData);
}

void AMVEnemy::HandleEnemyGroggyStarted()
{
	OnEnemyGroggyStarted.Broadcast();
}

void AMVEnemy::HandleEnemyGroggyEnded()
{
	OnEnemyGroggyEnded.Broadcast();
}
