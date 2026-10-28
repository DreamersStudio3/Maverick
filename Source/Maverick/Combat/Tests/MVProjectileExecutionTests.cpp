#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "Character/NPC/Enemy/MVEnemy.h"
#include "Character/PC/MVPlayerCharacter.h"
#include "Combat/MVAbilityBase.h"
#include "Combat/MVHitResolverSubsystem.h"
#include "Combat/Projectile/MVProjectileBase.h"
#include "Components/MVCombatComponent.h"
#include "Components/MVStatComponent.h"
#include "Components/MVWeaponComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMVProjectileExecutionTest,
	"Maverick.Combat.Projectile.ExecutionLifetime",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMVProjectileExecutionTest::RunTest(const FString& Parameters)
{
	const UWorld::InitializationValues Values = UWorld::InitializationValues()
		.AllowAudioPlayback(false).CreatePhysicsScene(true).CreateNavigation(false)
		.CreateAISystem(false).ShouldSimulatePhysics(false);
	UWorld* World = UWorld::CreateWorld(
		EWorldType::Game, false, NAME_None, nullptr, true,
		ERHIFeatureLevel::Num, &Values);
	if (!TestNotNull(TEXT("Projectile execution world"), World))
	{
		return false;
	}
	GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	World->BeginPlay();

	const FTransform PlayerTransform(FRotator::ZeroRotator, FVector::ZeroVector);
	AMVPlayerCharacter* Player = World->SpawnActorDeferred<AMVPlayerCharacter>(
		AMVPlayerCharacter::StaticClass(), PlayerTransform,
		nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (Player && Player->WeaponComponent)
	{
		Player->WeaponComponent->bManageWeaponMesh = false;
		Player->WeaponComponent->BareHandWeapon.AttackPower = 20.0f;
		Player->FinishSpawning(PlayerTransform);
	}

	FActorSpawnParameters Spawn;
	Spawn.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AMVEnemy* Enemy = World->SpawnActor<AMVEnemy>(
		FVector(500.0f, 0.0f, 0.0f), FRotator::ZeroRotator, Spawn);
	UMVHitResolverSubsystem* Resolver = UMVHitResolverSubsystem::Get(World);
	const bool bReady = TestNotNull(TEXT("Projectile execution player"), Player)
		&& TestNotNull(TEXT("Projectile execution enemy"), Enemy)
		&& TestNotNull(TEXT("Projectile execution resolver"), Resolver)
		&& TestNotNull(TEXT("Projectile execution combat"),
			Player ? Player->CombatComponent.Get() : nullptr)
		&& TestNotNull(TEXT("Projectile execution weapon"),
			Player ? Player->WeaponComponent.Get() : nullptr)
		&& TestNotNull(TEXT("Projectile execution player stats"),
			Player ? Player->StatComponent.Get() : nullptr)
		&& TestNotNull(TEXT("Projectile execution enemy stats"),
			Enemy ? Enemy->StatComponent.Get() : nullptr);

	if (bReady)
	{
		Player->DispatchBeginPlay();
		Enemy->DispatchBeginPlay();
		Player->StatComponent->SetCurrentHP(100.0f);
		Player->StatComponent->SetMaxMP(100.0f);
		Player->StatComponent->SetCurrentMP(50.0f);
		Player->StatComponent->CriticalPercent = 0.0f;
		Enemy->StatComponent->SetMaxHP(100.0f);
		Enemy->StatComponent->SetCurrentHP(100.0f);
		Enemy->StatComponent->SetSkillDefence(0.0f);

		UMVCombatComponent* Combat = Player->CombatComponent.Get();
		const FMVWeaponHitSnapshot WeaponSnapshot =
			Player->WeaponComponent->CaptureWeaponHitSnapshot();
		if (TestTrue(TEXT("Bare-hand launch snapshot is valid"),
			WeaponSnapshot.bValid && WeaponSnapshot.ItemTag.IsValid()
			&& WeaponSnapshot.AttackPower > 0.0f))
		{
			UMVAbilityBase* SourceAbility = NewObject<UMVAbilityBase>(Combat);
			SourceAbility->SetOwner(Combat);
			FMVSkillEntry& Skill = Combat->SkillMap.FindOrAdd(EMVSkillSlot::QSkill);
			Skill.AbilityInstances.Add(SourceAbility);
			Skill.LoadoutRevision = 1;
			Skill.MainCooldownDuration = 3.0f;

			const FName ActionTableName(TEXT("ProjectileExecutionTest"));
			const FName ActionRowName(TEXT("Q"));
			const auto CreateRegisteredShot =
				[World, Player, Combat, SourceAbility, &Skill, &WeaponSnapshot,
				 ActionTableName, ActionRowName](const int32 AttackId,
				 const uint64 UseId) -> AMVProjectileBase*
			{
				Skill.CurrentUseId = UseId;
				Skill.bMainCooldownRefundedForCurrentUse = false;
				Skill.MainCooldownEndTime = -1.0f;
				SourceAbility->SetAttackInstanceId(AttackId);
				SourceAbility->SetExecutionSource(0, WeaponSnapshot.ItemTag);
				SourceAbility->bAbilityActive = true;
				SourceAbility->bAbilityCostConsumed = true;
				SourceAbility->ConsumedMPThisExecution = 12.0f;
				Combat->CurrentAbilityInstance = SourceAbility;
				Combat->CurrentAbilityActionTableName = ActionTableName;
				Combat->CurrentAbilityActionRowName = ActionRowName;
				Combat->CurrentAttackInstanceId = AttackId;
				Combat->CurrentAttackLifeId = Combat->CurrentLifeId;
				Combat->CurrentPreparedSkillSlot = EMVSkillSlot::QSkill;
				Combat->CurrentPreparedChainStageIndex = 0;
				Combat->CurrentPreparedSlotRevision = Skill.LoadoutRevision;
				Combat->CurrentPreparedSkillUseId = UseId;
				Combat->bCurrentAbilityAwaitingCompletion = true;

				FMVProjectileLaunchContext Context;
				Context.Attacker = Player;
				Context.Team = EMVProjectileTeam::Player;
				Context.AttackInstanceId = AttackId;
				Context.AttackTypes = EMVAttackTypes::SkillAttack;
				Context.LaunchDirection = FVector::ForwardVector;
				Context.WeaponSnapshot = WeaponSnapshot;
				const FTransform Transform(
					FRotator::ZeroRotator, FVector(0.0f, 0.0f, 300.0f));
				AMVProjectileBase* Projectile = World->SpawnActorDeferred<AMVProjectileBase>(
					AMVProjectileBase::StaticClass(), Transform, Player, Player,
					ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
				if (!Projectile)
				{
				return nullptr;
				}
				const bool bInitialized = Projectile->InitializeProjectile(Context);
				Projectile->FinishSpawning(Transform);
				if (!bInitialized || !Combat->RegisterLaunchedProjectile(Projectile, SourceAbility)
					|| !Projectile->ActivateProjectile())
				{
				Projectile->Destroy();
				return nullptr;
				}
				return Projectile;
			};

			// 충돌 자체는 InitialOverlapAndTeamFilter가 검증한다. 여기서는
			// 같은 Resolver 사건을 동작 종료 뒤에 보내 실행 기록의 수명을 검사한다.
			const auto ResolveShot =
				[Resolver, Player, Enemy, &WeaponSnapshot](
					const int32 AttackId, FMVResolvedHitData& OutHit) -> bool
			{
				FMVHitResolveRequest Request;
				Request.Attacker = Player;
				Request.Victim = Enemy;
				Request.AttackInstanceId = AttackId;
				Request.AttackTypes = EMVAttackTypes::SkillAttack;
				Request.bUseSourceWeaponSnapshot = true;
				Request.SourceWeaponSnapshot = WeaponSnapshot;
				return Resolver->ResolveAttackHit(Request, OutHit);
			};

			AMVProjectileBase* FirstShot = CreateRegisteredShot(101, 1);
			if (TestNotNull(TEXT("First registered projectile"), FirstShot))
			{
				AMVProjectileBase* SecondShot = CreateRegisteredShot(101, 1);
				TestNotNull(TEXT("Second projectile shares one attack"), SecondShot);
				Combat->HandleActionEnded(ActionTableName, ActionRowName, false);
				TestTrue(TEXT("First projectile remains after action end"),
					Combat->ProjectileExecutions.Contains(101));
				TestTrue(TEXT("Original skill cooldown started"),
					Skill.GetMainCooldownRemaining(World->GetTimeSeconds())
					> KINDA_SMALL_NUMBER);
				Combat->CurrentAttackInstanceId = 999;
				FMVResolvedHitData FirstHit;
				const float EnemyHPBeforeHit = Enemy->StatComponent->CurrentHP;
				if (TestTrue(TEXT("Late first hit resolves"),
					ResolveShot(101, FirstHit)))
				{
					TestTrue(TEXT("Late first hit damages victim"),
						Enemy->StatComponent->CurrentHP < EnemyHPBeforeHit);
					const FMVProjectileExecutionRecord* FirstRecord =
						Combat->ProjectileExecutions.Find(101);
					if (TestNotNull(TEXT("Late hit keeps execution record"), FirstRecord))
					{
						UMVAbilityBase* ExecutionAbility = FirstRecord->ExecutionAbility.Get();
						if (TestNotNull(TEXT("Late execution ability"), ExecutionAbility))
						{
							TestTrue(TEXT("Original skill use refunds after action end"),
								Combat->RefundSkillMainCooldownForExecution(0, ExecutionAbility));
							TestFalse(TEXT("Same attack cannot refund twice"),
								Combat->RefundSkillMainCooldownForExecution(0, ExecutionAbility));
							TestTrue(TEXT("Main cooldown is ready after refund"),
								Skill.GetMainCooldownRemaining(World->GetTimeSeconds())
								<= KINDA_SMALL_NUMBER);
						}
					}
				}
				// 자동화 월드에서는 Destroy가 EndPlay를 즉시 보내지 않을 수 있다.
				// 실제 EndPlay가 보내는 종료 사건을 먼저 전달해 기록 정리를 검증한다.
				FirstShot->OnProjectileFinished.Broadcast(FirstShot, 101);
				FirstShot->Destroy();
				if (SecondShot)
				{
					TestTrue(TEXT("Record remains while another projectile flies"),
						Combat->ProjectileExecutions.Contains(101));
					SecondShot->OnProjectileFinished.Broadcast(SecondShot, 101);
					SecondShot->Destroy();
				}
				TestTrue(TEXT("First record is released after all projectiles finish"),
					Combat->ProjectileExecutions.IsEmpty());
			}

			AMVProjectileBase* StaleShot = CreateRegisteredShot(103, 3);
			if (TestNotNull(TEXT("Stale-use projectile"), StaleShot))
			{
				Combat->HandleActionEnded(ActionTableName, ActionRowName, false);
				Skill.CurrentUseId = 4;
				FMVResolvedHitData StaleHit;
				const float EnemyHPBeforeStaleHit = Enemy->StatComponent->CurrentHP;
				if (TestTrue(TEXT("Old use still deals its launch damage"),
					ResolveShot(103, StaleHit)))
				{
					TestTrue(TEXT("Old use reduces victim health"),
						Enemy->StatComponent->CurrentHP < EnemyHPBeforeStaleHit);
					const FMVProjectileExecutionRecord* StaleRecord =
						Combat->ProjectileExecutions.Find(103);
					if (TestNotNull(TEXT("Stale-use record remains"), StaleRecord))
					{
						TestFalse(TEXT("Old use cannot refund the new skill use"),
							Combat->RefundSkillMainCooldownForExecution(
								0, StaleRecord->ExecutionAbility.Get()));
					}
				}
				StaleShot->OnProjectileFinished.Broadcast(StaleShot, 103);
				StaleShot->Destroy();
				TestTrue(TEXT("Stale-use record is released"),
					Combat->ProjectileExecutions.IsEmpty());
			}

			AMVProjectileBase* DeathShot = CreateRegisteredShot(102, 2);
			if (TestNotNull(TEXT("Death-boundary projectile"), DeathShot))
			{
				Combat->HandleActionEnded(ActionTableName, ActionRowName, false);
				Player->StatComponent->SetCurrentHP(0.0f);
				TestTrue(TEXT("Shooter died after launch"),
					Player->StatComponent->IsDead());
				FMVResolvedHitData DeathHit;
				const float EnemyHPBeforeDeathHit = Enemy->StatComponent->CurrentHP;
				if (TestTrue(TEXT("Pre-death projectile still resolves"),
					ResolveShot(102, DeathHit)))
				{
					TestTrue(TEXT("Pre-death projectile retains base damage"),
						Enemy->StatComponent->CurrentHP < EnemyHPBeforeDeathHit);
					const FMVProjectileExecutionRecord* DeathRecord =
						Combat->ProjectileExecutions.Find(102);
					if (TestNotNull(TEXT("Death-boundary record remains"), DeathRecord))
					{
						TestFalse(TEXT("Dead shooter receives no hit reward"),
							DeathRecord->bHadHit);
						TestFalse(TEXT("Dead shooter cannot refund"),
							Combat->RefundSkillMainCooldownForExecution(
								0, DeathRecord->ExecutionAbility.Get()));
					}
				}
				DeathShot->OnProjectileHit.Broadcast(DeathShot, DeathHit);
				DeathShot->OnProjectileFinished.Broadcast(DeathShot, 102);
				DeathShot->Destroy();
				TestTrue(TEXT("Death-boundary record is released"),
					Combat->ProjectileExecutions.IsEmpty());
			}
		}
	}

	World->EndPlay(EEndPlayReason::Quit);
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return !HasAnyErrors();
}

#endif
