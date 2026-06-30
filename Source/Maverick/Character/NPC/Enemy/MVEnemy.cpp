// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/NPC/Enemy/MVEnemy.h"

#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Character/NPC/Enemy/MVEnemyWeapon.h"

AMVEnemy::AMVEnemy()
{
}

void AMVEnemy::BeginPlay()
{
	Super::BeginPlay();
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

void AMVEnemy::HandleAttackMontageEnded(UAnimMontage* Montage, const bool bInterrupted, const int32 AttackInstanceId)
{
	if (Montage != AttackMontage || AttackInstanceId == INDEX_NONE)
	{
		return;
	}

	UE_LOG(
		LogTemp,
		Verbose,
		TEXT("Enemy attack montage ended. Enemy=%s AttackInstanceId=%d Interrupted=%s Montage=%s"),
		*GetNameSafe(this),
		AttackInstanceId,
		bInterrupted ? TEXT("true") : TEXT("false"),
		*GetNameSafe(Montage));

	OnAttackMontageEnded.Broadcast(AttackInstanceId, Montage, bInterrupted);
}
