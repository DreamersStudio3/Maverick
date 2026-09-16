// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/NPC/Enemy/MVEnemy.h"

#include "AI/Controller/MVAIController.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/CapsuleComponent.h"
#include "Character/NPC/Enemy/MVEnemyWeapon.h"
#include "Components/MVActionComponent.h"
#include "Components/MVCombatComponent.h"
#include "Components/MVDeathComponent.h"
#include "Components/MVEnemyDodgeTokenComponent.h"
#include "Components/MVHitReactionComponent.h"
#include "Components/MVStatComponent.h"
#include "Engine/World.h"
#include "Enum/MVCombatActionTypes.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StateTreeComponent.h"
#include "Tags/MVGameplayTags.h"
#include "TargetComponent.h"
#include "TimerManager.h"
#include "UI/HUD/MVMainHUDWidget.h"
#include "UI/System/MVUISubsystem.h"

AMVEnemy::AMVEnemy()
{
	EnemyDodgeTokenComponent = CreateDefaultSubobject<UMVEnemyDodgeTokenComponent>(TEXT("EnemyDodgeTokenComponent"));
	CharacterIndexCode = MVGameplayTags::Character_NPC_Enemy_E1;
}

void AMVEnemy::BeginPlay()
{
	Super::BeginPlay();

	BossHUDBindAttemptsRemaining = 20;
	ScheduleBossHUDBindRetry(0.0f);
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

bool AMVEnemy::TryHeavyAttack_Implementation(const int32 ActionIndex, const FName StartSection)
{
	UMVCombatComponent* EnemyCombatComponent = FindComponentByClass<UMVCombatComponent>();
	return EnemyCombatComponent
		&& EnemyCombatComponent->TryCombatAction(EMVCombatActionTypes::HeavyAttack, ActionIndex, StartSection);
}

bool AMVEnemy::TrySkillAttack_Implementation(const int32 SkillIndex, const FName StartSection)
{
	UMVCombatComponent* EnemyCombatComponent = FindComponentByClass<UMVCombatComponent>();
	return EnemyCombatComponent
		&& EnemyCombatComponent->TryCombatAction(EMVCombatActionTypes::Skill, SkillIndex, StartSection);
}

AMVEnemyWeapon* AMVEnemy::GetWeaponActor() const
{
	return WeaponActor;
}

void AMVEnemy::DestroyWeaponActor()
{
	if (WeaponActor)
	{
		WeaponActor->Destroy();
		WeaponActor = nullptr;
	}
}

void AMVEnemy::HideBoundBossHUD()
{
	BossHUDBindAttemptsRemaining = 0;

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

bool AMVEnemy::TryChooseHitReactionRecovery(
	const FMVHitReactionRecoveryDecisionContext& Context,
	FMVHitReactionRecoveryDecision& OutDecision)
{
	OutDecision = FMVHitReactionRecoveryDecision();

	if (!bUseAirborneRecoveryDecision
		|| Context.Owner != this
		|| Context.HitReactionType != EMVActionHitReactionType::Airborne)
	{
		return false;
	}

	const AActor* Target = ResolveHitReactionRecoveryTarget();
	if (!Target)
	{
		OutDecision.Type = EMVHitReactionRecoveryDecisionType::Getup;
		return true;
	}

	const float DistanceSquared = FVector::DistSquared2D(GetActorLocation(), Target->GetActorLocation());
	if (DistanceSquared > FMath::Square(AirborneEscapeDodgeDistance))
	{
		OutDecision.Type = EMVHitReactionRecoveryDecisionType::Getup;
		return true;
	}

	OutDecision.Type = EMVHitReactionRecoveryDecisionType::EscapeDodge;
	OutDecision.EscapeDirection = ResolveEscapeDirectionAwayFromTarget(*Target);
	return true;
}

EMVFieldTransitionResetPolicy AMVEnemy::GetFieldTransitionResetPolicy_Implementation() const
{
	return EMVFieldTransitionResetPolicy::ResetEveryTransition;
}

FName AMVEnemy::GetFieldTransitionResetFieldId_Implementation() const
{
	return NAME_None;
}

FName AMVEnemy::GetFieldTransitionResetObjectId_Implementation() const
{
	return NAME_None;
}

void AMVEnemy::HandleFieldTransitionReset_Implementation(
	const FMVFieldTransitionResetContext& ResetContext)
{
	if (ResetContext.bIsConsumed)
	{
		return;
	}

	ResetEnemyForFieldTransition();
}

void AMVEnemy::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	HideBoundBossHUD();

	if (StatComponent)
	{
		StatComponent->OnGroggyStarted.RemoveDynamic(this, &AMVEnemy::HandleEnemyGroggyStarted);
		StatComponent->OnGroggyEnded.RemoveDynamic(this, &AMVEnemy::HandleEnemyGroggyEnded);
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

void AMVEnemy::ResetEnemyForFieldTransition()
{
	if (UMVActionComponent* EnemyActionComponent = FindComponentByClass<UMVActionComponent>())
	{
		EnemyActionComponent->CancelActiveAction(0.0f);
	}

	if (DeathComponent)
	{
		DeathComponent->ResetDeathPresentationForRespawn();
	}

	if (StatComponent)
	{
		StatComponent->ResetDeathState();
		StatComponent->ResetGroggyState();
		StatComponent->SetCurrentHP(StatComponent->MaxHP);
		StatComponent->SetCurrentStamina(StatComponent->MaxStamina);
		StatComponent->SetCurrentMP(StatComponent->MaxMP);
	}

	if (EnemyDodgeTokenComponent)
	{
		EnemyDodgeTokenComponent->ResetForFieldTransition();
	}

	if (UTargetComponent* TargetComponent = FindComponentByClass<UTargetComponent>())
	{
		TargetComponent->SetCanBeCaptured(true);
	}

	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	SetActorTickEnabled(true);

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}

	if (USkeletalMeshComponent* MeshComponent = GetMesh())
	{
		MeshComponent->SetHiddenInGame(false);
		MeshComponent->SetVisibility(true, true);
	}

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
		MovementComponent->SetMovementMode(MOVE_Walking);
	}

	BindDamageHandlers();
	RestoreWeaponActor();
	RestartStateTreeLogicForFieldTransition();
	BossHUDBindAttemptsRemaining = 20;
	ScheduleBossHUDBindRetry(0.0f);
}

void AMVEnemy::RestoreWeaponActor()
{
	if (IsValid(WeaponActor) || !WeaponClass)
	{
		return;
	}

	UWorld* World = GetWorld();
	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (!World || !MeshComponent)
	{
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.Instigator = this;
	WeaponActor = World->SpawnActor<AMVEnemyWeapon>(
		WeaponClass,
		GetActorTransform(),
		SpawnParameters);
	if (!WeaponActor)
	{
		return;
	}

	if (bUseDualWeapon)
	{
		WeaponActor->AttachDualToHands(MeshComponent);
	}
	else
	{
		WeaponActor->AttachCombinedToHand(MeshComponent);
	}
}

void AMVEnemy::RestartStateTreeLogicForFieldTransition()
{
	TSet<UStateTreeComponent*> RestartedComponents;
	auto RestartStateTreeComponent = [&RestartedComponents](UStateTreeComponent* StateTreeComponent)
	{
		if (!StateTreeComponent || RestartedComponents.Contains(StateTreeComponent))
		{
			return;
		}

		RestartedComponents.Add(StateTreeComponent);
		StateTreeComponent->StopLogic(TEXT("Enemy field transition reset"));
		StateTreeComponent->RestartLogic();
	};

	if (AController* OwningController = GetController())
	{
		if (AMVAIController* AIController = Cast<AMVAIController>(OwningController))
		{
			AIController->TargetActor = nullptr;
		}

		if (AAIController* AIController = Cast<AAIController>(OwningController))
		{
			AIController->StopMovement();
			AIController->ClearFocus(EAIFocusPriority::Gameplay);

			if (UStateTreeComponent* StateTreeComponent = Cast<UStateTreeComponent>(AIController->GetBrainComponent()))
			{
				RestartStateTreeComponent(StateTreeComponent);
			}
		}

		TArray<UStateTreeComponent*> ControllerStateTreeComponents;
		OwningController->GetComponents<UStateTreeComponent>(ControllerStateTreeComponents);
		for (UStateTreeComponent* StateTreeComponent : ControllerStateTreeComponents)
		{
			RestartStateTreeComponent(StateTreeComponent);
		}
	}

	TArray<UStateTreeComponent*> PawnStateTreeComponents;
	GetComponents<UStateTreeComponent>(PawnStateTreeComponents);
	for (UStateTreeComponent* StateTreeComponent : PawnStateTreeComponents)
	{
		RestartStateTreeComponent(StateTreeComponent);
	}
}

void AMVEnemy::BindDamageHandlers()
{
	if (StatComponent)
	{
		OnDamaged.RemoveDynamic(StatComponent, &UMVStatComponent::HandleDamaged);
		OnDamaged.AddUniqueDynamic(StatComponent, &UMVStatComponent::HandleDamaged);

		StatComponent->OnGroggyStarted.RemoveDynamic(this, &AMVEnemy::HandleEnemyGroggyStarted);
		StatComponent->OnGroggyStarted.AddUniqueDynamic(this, &AMVEnemy::HandleEnemyGroggyStarted);
		StatComponent->OnGroggyEnded.RemoveDynamic(this, &AMVEnemy::HandleEnemyGroggyEnded);
		StatComponent->OnGroggyEnded.AddUniqueDynamic(this, &AMVEnemy::HandleEnemyGroggyEnded);
	}

	if (HitReactionComponent)
	{
		//OnDamaged.RemoveDynamic(HitReactionComponent, &UMVHitReactionComponent::HandleDamaged);
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

AActor* AMVEnemy::ResolveHitReactionRecoveryTarget() const
{
	if (const AMVAIController* AIController = Cast<AMVAIController>(GetController()))
	{
		if (IsValid(AIController->TargetActor))
		{
			return AIController->TargetActor;
		}
	}

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	return IsValid(PlayerPawn) ? PlayerPawn : nullptr;
}

EMVActionInputDirection AMVEnemy::ResolveEscapeDirectionAwayFromTarget(const AActor& Target) const
{
	const FVector ToTarget = FVector(
		Target.GetActorLocation().X - GetActorLocation().X,
		Target.GetActorLocation().Y - GetActorLocation().Y,
		0.0f);
	if (ToTarget.IsNearlyZero())
	{
		return EMVActionInputDirection::Back;
	}

	const FVector TargetDirection = ToTarget.GetSafeNormal2D();
	const FVector Forward = GetActorForwardVector().GetSafeNormal2D();
	const FVector Right = GetActorRightVector().GetSafeNormal2D();
	const float ForwardDot = FVector::DotProduct(TargetDirection, Forward);
	const float RightDot = FVector::DotProduct(TargetDirection, Right);

	if (FMath::Abs(ForwardDot) >= FMath::Abs(RightDot))
	{
		return ForwardDot >= 0.0f
			? EMVActionInputDirection::Back
			: EMVActionInputDirection::Forward;
	}

	return RightDot >= 0.0f
		? EMVActionInputDirection::Left
		: EMVActionInputDirection::Right;
}
