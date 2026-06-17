// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/MVEnemy.h"

#include "Kismet/GameplayStatics.h"

AMVEnemy::AMVEnemy()
{
}

bool AMVEnemy::Attack(int EAttackDirection)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    

	if (AnimInstance && AttackMontage)
	{
		FName SectionName;
 
		
		switch (EAttackDirection)
		{
		case 0: SectionName = FName("Front"); break;
		case 1: SectionName = FName("Back");  break;
		case 2: SectionName = FName("Left");  break;
		case 3: SectionName = FName("Right"); break;
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

