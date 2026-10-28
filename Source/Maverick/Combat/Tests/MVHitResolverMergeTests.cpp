#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Character/MVCharacterBase.h"
#include "Combat/MVAbilityBase.h"
#include "Combat/MVHitResolverSubsystem.h"
#include "Components/MVCombatComponent.h"
#include "Components/MVStatComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMVHitResolverMergedDamageTest,
	"Maverick.Combat.HitResolver.MergedDamage",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMVHitResolverMergedDamageTest::RunTest(const FString& Parameters)
{
	const UWorld::InitializationValues Values = UWorld::InitializationValues()
		.AllowAudioPlayback(false).CreatePhysicsScene(true).CreateNavigation(false)
		.CreateAISystem(false).ShouldSimulatePhysics(false);
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false, NAME_None, nullptr, true, ERHIFeatureLevel::Num, &Values);
	GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	World->BeginPlay();

	FActorSpawnParameters Spawn;
	Spawn.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AMVCharacterBase* Attacker = World->SpawnActor<AMVCharacterBase>(FVector::ZeroVector, FRotator::ZeroRotator, Spawn);
	AMVCharacterBase* Victim = World->SpawnActor<AMVCharacterBase>(FVector(100.0f, 0.0f, 0.0f), FRotator::ZeroRotator, Spawn);
	UMVHitResolverSubsystem* Resolver = UMVHitResolverSubsystem::Get(World);
	if (TestNotNull(TEXT("Attacker"), Attacker)
		&& TestNotNull(TEXT("Victim"), Victim)
		&& TestNotNull(TEXT("Resolver"), Resolver))
	{
		Attacker->DispatchBeginPlay();
		Victim->DispatchBeginPlay();
		Victim->StatComponent->SetNormalDefence(2.0f);
		Victim->StatComponent->SetSkillDefence(8.0f);
		Attacker->StatComponent->CriticalPercent = 0.0f;

		UMVAbilityBase* Ability = NewObject<UMVAbilityBase>(Attacker);
		Ability->bAbilityActive = true;
		Ability->SetAttackInstanceId(17);
		Ability->SetExecutionSource(0, FGameplayTag());
		Attacker->CombatComponent->CurrentAbilityInstance = Ability;
		Attacker->CombatComponent->bCurrentAbilityAwaitingCompletion = true;
		Attacker->CombatComponent->CurrentAttackInstanceId = 17;
		Attacker->CombatComponent->CurrentAttackLifeId =
			Attacker->CombatComponent->CurrentLifeId;

		int32 OutgoingModifierCalls = 0;
		const FDelegateHandle OutgoingModifierHandle =
			Attacker->CombatComponent->OnModifyOutgoingAttackDamage.AddLambda(
				[&OutgoingModifierCalls](const FMVResolvedHitData&, float& InOutDamage)
				{
					++OutgoingModifierCalls;
					InOutDamage *= 1.15f;
				});

		FMVDirectDamageRequest DirectRequest;
		DirectRequest.Attacker = Attacker;
		DirectRequest.Victim = Victim;
		DirectRequest.AttackInstanceId = 17;
		DirectRequest.FinalDamage = 0.1f;
		FMVResolvedHitData DirectHit;
		if (TestTrue(TEXT("Direct damage resolves"), Resolver->ResolveDirectDamage(DirectRequest, DirectHit)))
		{
			TestEqual(TEXT("Direct damage skips outgoing attack modifiers"), OutgoingModifierCalls, 0);
			TestEqual(TEXT("Direct damage keeps its supplied amount"), DirectHit.FinalDamage, 0.1f);
			TestFalse(TEXT("Direct damage does not confirm the current attack"),
				Attacker->CombatComponent->bCurrentAbilityHitConfirmed);
		}
		Attacker->CombatComponent->OnModifyOutgoingAttackDamage.Remove(OutgoingModifierHandle);

		FMVHitResolveRequest Request;
		Request.Attacker = Attacker;
		Request.Victim = Victim;
		Request.AttackInstanceId = 17;
		Request.bUseSourceWeaponSnapshot = true;
		Request.SourceWeaponSnapshot.AttackPower = 13.0f;
		Request.SourceWeaponSnapshot.bValid = true;
		FMVResolvedHitData SkillHit;
		if (TestTrue(TEXT("Active skill hit resolves"), Resolver->ResolveAttackHit(Request, SkillHit)))
		{
			TestEqual(TEXT("Skill hit uses nonzero launch weapon power"), SkillHit.WeaponAttackPower, 13.0f);
			TestEqual(TEXT("Active skill uses skill defence"), SkillHit.VictimDefence, 8.0f);
			TestEqual(TEXT("Skill damage applies defence before outgoing modifiers"),
				SkillHit.FinalDamage, 9.0f);
		}

		Attacker->StatComponent->CriticalPercent = 1.0f;
		Attacker->StatComponent->CriticalDamage = 2.0f;
		FMVResolvedHitData CriticalHit;
		if (TestTrue(TEXT("Critical skill hit resolves"), Resolver->ResolveAttackHit(Request, CriticalHit)))
		{
			TestEqual(TEXT("Critical multiplier follows skill defence"), CriticalHit.FinalDamage, 18.0f);
		}

		const FDelegateHandle ActiveModifierHandle =
			Attacker->CombatComponent->OnModifyOutgoingAttackDamage.AddLambda(
				[&OutgoingModifierCalls](const FMVResolvedHitData&, float& InOutDamage)
				{
					++OutgoingModifierCalls;
					InOutDamage *= 1.15f;
				});
		FMVResolvedHitData ModifiedHit;
		if (TestTrue(TEXT("Current attack hit resolves"), Resolver->ResolveAttackHit(Request, ModifiedHit)))
		{
			TestEqual(TEXT("Current attack calls outgoing modifier once"), OutgoingModifierCalls, 1);
			TestTrue(TEXT("Outgoing modifier follows skill defence and critical"),
				FMath::IsNearlyEqual(ModifiedHit.FinalDamage, 20.7f, 0.001f));
			TestTrue(TEXT("Outgoing modifier changes positive damage"),
				ModifiedHit.FinalDamage > CriticalHit.FinalDamage);
		}

		Request.AttackInstanceId = 18;
		FMVResolvedHitData OtherHit;
		if (TestTrue(TEXT("Unrelated hit resolves"), Resolver->ResolveAttackHit(Request, OtherHit)))
		{
			TestEqual(TEXT("Unrelated attack skips outgoing modifier"), OutgoingModifierCalls, 1);
			TestEqual(TEXT("Unrelated hit keeps normal defence"), OtherHit.VictimDefence, 2.0f);
			TestEqual(TEXT("Unrelated hit keeps unmodified damage"), OtherHit.FinalDamage, 24.0f);
			TestTrue(
				TEXT("Hit without incoming direction uses attacker-to-victim direction"),
				OtherHit.HitDirection.Equals(FVector::ForwardVector));
		}

		Request.AttackTypes = EMVAttackTypes::SkillAttack;
		Request.bUseIncomingDirection = true;
		Request.IncomingDirection = FVector(0.0f, 10.0f, 4.0f);

		FMVResolvedHitData ProjectileHit;
		if (TestTrue(TEXT("Projectile hit resolves"), Resolver->ResolveAttackHit(Request, ProjectileHit)))
		{
			TestEqual(TEXT("Unrelated projectile skips outgoing modifier"), OutgoingModifierCalls, 1);
			TestEqual(TEXT("Projectile uses launch weapon power"), ProjectileHit.WeaponAttackPower, 13.0f);
			TestEqual(TEXT("Projectile uses skill defence"), ProjectileHit.VictimDefence, 8.0f);
			TestTrue(
				TEXT("Projectile uses attacker-to-victim horizontal direction"),
				ProjectileHit.HitDirection.Equals(FVector::ForwardVector));
		}

		Request.SourceWeaponSnapshot.bValid = false;
		FMVResolvedHitData RejectedHit;
		TestFalse(
			TEXT("Invalid launch weapon snapshot is rejected"),
			Resolver->ResolveAttackHit(Request, RejectedHit));
		Attacker->CombatComponent->OnModifyOutgoingAttackDamage.Remove(ActiveModifierHandle);
	}

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return !HasAnyErrors();
}
#endif
