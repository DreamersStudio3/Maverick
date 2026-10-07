#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Character/MVCharacterBase.h"
#include "Combat/MVAbilityBase.h"
#include "Combat/MVHitResolverSubsystem.h"
#include "Components/MVCombatComponent.h"
#include "Components/MVStatComponent.h"
#include "Components/MVStatusEffectComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "StatusEffects/MVStatusEffectDefinition.h"

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

		FMVHitResolveRequest Request;
		Request.Attacker = Attacker;
		Request.Victim = Victim;
		Request.AttackInstanceId = 17;
		FMVResolvedHitData SkillHit;
		if (TestTrue(TEXT("Active skill hit resolves"), Resolver->ResolveAttackHit(Request, SkillHit)))
		{
			TestEqual(TEXT("Active skill uses skill defence"), SkillHit.VictimDefence, 8.0f);
			const float AfterDefence = FMath::TruncToFloat(
				FMath::Max(0.0f, SkillHit.WeaponAttackPower - 4.0f) * 10.0f) / 10.0f;
			TestTrue(TEXT("Skill damage applies defence before outgoing modifiers"),
				FMath::IsNearlyEqual(SkillHit.FinalDamage, AfterDefence));
		}

		Attacker->StatComponent->CriticalPercent = 1.0f;
		Attacker->StatComponent->CriticalDamage = 2.0f;
		FMVResolvedHitData CriticalHit;
		if (TestTrue(TEXT("Critical skill hit resolves"), Resolver->ResolveAttackHit(Request, CriticalHit)))
		{
			const float AfterDefence = FMath::TruncToFloat(
				FMath::Max(0.0f, CriticalHit.WeaponAttackPower - 4.0f) * 10.0f) / 10.0f;
			TestTrue(TEXT("Critical multiplier follows skill defence"),
				FMath::IsNearlyEqual(CriticalHit.FinalDamage, AfterDefence * 2.0f));
		}

		UMVStatusEffectDefinition* Passive = LoadObject<UMVStatusEffectDefinition>(nullptr,
			TEXT("/Game/Miscellaneous/Weapon/Passive/GreatSword/DA_Passive_WeightOfRuin.DA_Passive_WeightOfRuin"));
		if (TestNotNull(TEXT("Weight of Ruin passive asset"), Passive))
		{
			FMVStatusEffectSpec Spec;
			Spec.Definition = Passive;
			Spec.SourceActor = Attacker;
			const FMVStatusEffectHandle Handle = Attacker->StatusEffectComponent->ApplyStatusEffect(Spec);
			if (TestTrue(TEXT("Weight of Ruin applies"), Handle.IsValid()))
			{
				Attacker->CombatComponent->bCurrentAbilityAwaitingCompletion = true;
				Attacker->CombatComponent->CurrentAttackInstanceId = 17;
				FMVResolvedHitData PassiveHit;
				if (TestTrue(TEXT("Passive skill hit resolves"), Resolver->ResolveAttackHit(Request, PassiveHit)))
				{
					const float AfterDefence = FMath::TruncToFloat(
						FMath::Max(0.0f, PassiveHit.WeaponAttackPower - 4.0f) * 10.0f) / 10.0f;
					TestTrue(TEXT("Passive multiplies damage after skill defence and critical"),
						FMath::IsNearlyEqual(PassiveHit.FinalDamage, AfterDefence * 2.0f * 1.15f));
				}
				Attacker->StatusEffectComponent->RemoveStatusEffect(Handle);
			}
		}

		Request.AttackInstanceId = 18;
		FMVResolvedHitData OtherHit;
		if (TestTrue(TEXT("Unrelated hit resolves"), Resolver->ResolveAttackHit(Request, OtherHit)))
		{
			TestEqual(TEXT("Unrelated hit keeps normal defence"), OtherHit.VictimDefence, 2.0f);
		}
	}

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return !HasAnyErrors();
}
#endif
