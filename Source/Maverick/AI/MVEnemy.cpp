// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/MVEnemy.h"

#include "Kismet/GameplayStatics.h"

AMVEnemy::AMVEnemy()
{
}

bool AMVEnemy::Attack(const EMVAttackDirection AttackDirection)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    

	if (AnimInstance && AttackMontage)
	{
		FName SectionName;
 
		
		switch (AttackDirection)
		{
		case EMVAttackDirection::Forward: SectionName = FName("Front"); break;
		case EMVAttackDirection::Backward: SectionName = FName("Back"); break;
		case EMVAttackDirection::Left: SectionName = FName("Left"); break;
		case EMVAttackDirection::Right: SectionName = FName("Right"); break;
		default: SectionName = FName("Front"); break;
		}
 
		
		float PlayDuration = AnimInstance->Montage_Play(AttackMontage);
 
		if (PlayDuration > 0.0f)
		{
			
			AnimInstance->Montage_JumpToSection(SectionName, AttackMontage);
			return true; 
		}
	}
    
	return false; 
}

