#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "AI/AITask/FMVBossExecuteAttackTask.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/NotifyStates/MVAnimNotifyState_Ability.h"
#include "Character/MVCharacterBase.h"
#include "Combat/MVTutorialBossBasicAttackAbility.h"
#include "Components/MVActionComponent.h"
#include "Components/MVCombatComponent.h"
#include "Components/MVStatComponent.h"
#include "Components/MVWeaponComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "TimerManager.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMVTutorialBossBasicAttackTest,
	"Maverick.Combat.TutorialBoss.BasicAttack1",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMVTutorialBossBasicAttackTest::RunTest(const FString& Parameters)
{
	UDataTable* Selection = LoadObject<UDataTable>(nullptr,
		TEXT("/Game/Characters/NPC/Boss/TutorialBoss/DT_NewDataTable.DT_NewDataTable"));
	const FMVTutorialBossSkillRow* Row = Selection
		? Selection->FindRow<FMVTutorialBossSkillRow>(TEXT("BasicAttack1"), TEXT("BasicAttackTest")) : nullptr;
	if (!TestTrue(TEXT("BasicAttack1 has a combat row"), Row && Row->CombatAttackRow.DataTable)) return false;
	const UWorld::InitializationValues Values = UWorld::InitializationValues().AllowAudioPlayback(false)
		.CreatePhysicsScene(true).CreateNavigation(false).CreateAISystem(false).ShouldSimulatePhysics(false);
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false, NAME_None, nullptr, true, ERHIFeatureLevel::Num, &Values);
	GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	World->BeginPlay();
	FActorSpawnParameters Spawn;
	Spawn.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AMVCharacterBase* Boss = World->SpawnActor<AMVCharacterBase>(FVector::ZeroVector, FRotator::ZeroRotator, Spawn);
	AMVCharacterBase* Player = World->SpawnActor<AMVCharacterBase>(FVector(1000.0f, 0, 0), FRotator::ZeroRotator, Spawn);
	APlayerController* Controller = World->SpawnActor<APlayerController>();
	Boss->WeaponComponent->bManageWeaponMesh = false;
	Boss->WeaponComponent->DefaultWeaponRow.DataTable = LoadObject<UDataTable>(nullptr,
		TEXT("/Game/Table/Weapons/NPC/E1/DT_Weapon_E1.DT_Weapon_E1"));
	Boss->WeaponComponent->DefaultWeaponRow.RowName = TEXT("E1_Weapon_R");
	Boss->DispatchBeginPlay();
	Player->DispatchBeginPlay();
	APlayerState* PlayerState = World->SpawnActor<APlayerState>();
	PlayerState->SetIsABot(false);
	Controller->PlayerState = PlayerState;
	Player->SetPlayerState(PlayerState);
	Player->SetController(Controller);
	Controller->SetPawn(Player);
	AddInfo(FString::Printf(TEXT("Victim controller=%s PlayerController=%d"),
		*GetNameSafe(Player->GetController()), Controller->IsPlayerController()));
	USkeletalMesh* MeshAsset = LoadObject<USkeletalMesh>(nullptr,
		TEXT("/Game/Characters/NPC/Enemy/NamelessPuppet/SK/SK_CH_MOB_PCBase_12_PinoProto_P1_UE.SK_CH_MOB_PCBase_12_PinoProto_P1_UE"));
	Boss->GetMesh()->SetSkeletalMeshAsset(MeshAsset);
	Boss->GetMesh()->SetAnimInstanceClass(UAnimInstance::StaticClass());
	Boss->GetMesh()->RefreshBoneTransforms();
	TestNotNull(TEXT("Native animation instance"), Boss->GetMesh()->GetAnimInstance());
	TestTrue(TEXT("Player controlled victim"), Player->IsPlayerControlled());
	Player->StatComponent->SetCurrentHP(100.0f);
	TestEqual(TEXT("Existing E1 weapon provides attack power"), Boss->WeaponComponent->GetEquippedWeaponAttackPower(), 7.0f);
	const bool bStarted = Boss->CombatComponent->TryStartCombatActionFromRowHandle(Row->CombatAttackRow);
	TestTrue(TEXT("Combat prepares Ability and starts montage"), bStarted);
	UMVTutorialBossBasicAttackAbility* Ability = Cast<UMVTutorialBossBasicAttackAbility>(Boss->CombatComponent->CurrentAbilityInstance);
	if (Ability)
	{
		TestFalse(TEXT("Running attack rejects a second combat action"),
			Boss->CombatComponent->TryStartCombatActionFromRowHandle(Row->CombatAttackRow));
		TestTrue(TEXT("Rejected action preserves current Ability"), Boss->CombatComponent->CurrentAbilityInstance == Ability);
		Ability->bDrawDebug = false;
		const FVector Start = Boss->GetMesh()->GetSocketLocation(Ability->StartSocket);
		const FVector End = Boss->GetMesh()->GetSocketLocation(Ability->EndSocket);
		AddInfo(FString::Printf(TEXT("Weapon segment Start=%s End=%s Length=%.1f"), *Start.ToString(), *End.ToString(), FVector::Distance(Start, End)));
		Player->SetActorLocation((Start + End) * 0.5f);
		TestEqual(TEXT("No damage before Notify Begin"), Player->StatComponent->CurrentHP, 100.0f);
		UAnimMontage* Montage = Row->Montage.LoadSynchronous();
		UMVAnimNotifyState_Ability* Notify = nullptr;
		for (const FAnimNotifyEvent& Event : Montage->Notifies)
		{
			if ((Notify = Cast<UMVAnimNotifyState_Ability>(Event.NotifyStateClass))) break;
		}
		if (TestNotNull(TEXT("Existing montage Ability Notify"), Notify))
		{
			const FAnimNotifyEventReference Reference;
			Notify->NotifyBegin(Boss->GetMesh(), Montage, 0.2f, Reference);
			const float HPAfterHit = Player->StatComponent->CurrentHP;
			TestTrue(TEXT("Real socket overlap damages player"), HPAfterHit < 100.0f);
			// 타이머 검사에 필요한 테스트 프레임 번호 임시 진행; 종료 시 원래 값 복원
			const TGuardValue<uint64> FrameGuard(GFrameCounter, GFrameCounter);
			for (int32 Index = 0; Index < 4; ++Index)
			{
				++GFrameCounter;
				World->GetTimerManager().Tick(1.0f / 60.0f);
			}
			TestEqual(TEXT("One hit per victim per Notify window"), Player->StatComponent->CurrentHP, HPAfterHit);
			Notify->NotifyEnd(Boss->GetMesh(), Montage, Reference);
			++GFrameCounter;
			World->GetTimerManager().Tick(0.1f);
			TestFalse(TEXT("Notify End deactivates Ability"), Ability->bAbilityActive);
			TestEqual(TEXT("No damage after Notify End"), Player->StatComponent->CurrentHP, HPAfterHit);
			Player->BeginInvincibility();
			Notify->NotifyBegin(Boss->GetMesh(), Montage, 0.2f, Reference);
			TestEqual(TEXT("Invincibility rejects damage"), Player->StatComponent->CurrentHP, HPAfterHit);
			Player->EndInvincibility();
			++GFrameCounter;
			World->GetTimerManager().Tick(0.1f);
			TestTrue(TEXT("New Notify window allows next hit"), Player->StatComponent->CurrentHP < HPAfterHit);
			Boss->ActionComponent->CancelActiveAction(0.0f);
			TestFalse(TEXT("Action cancel ends Ability"), Ability->bAbilityActive);
			const float HPAfterCancel = Player->StatComponent->CurrentHP;
			++GFrameCounter;
			World->GetTimerManager().Tick(0.1f);
			TestEqual(TEXT("No damage after action cancellation"), Player->StatComponent->CurrentHP, HPAfterCancel);

			TestTrue(TEXT("Next attack starts after cancellation"),
				Boss->CombatComponent->TryStartCombatActionFromRowHandle(Row->CombatAttackRow));
			UMVTutorialBossBasicAttackAbility* NextAbility = Cast<UMVTutorialBossBasicAttackAbility>(Boss->CombatComponent->CurrentAbilityInstance);
			if (TestNotNull(TEXT("Next prepared Ability"), NextAbility))
			{
				NextAbility->bDrawDebug = false;
				bool bNotifyActivated = false;
				const int32 Frames = FMath::CeilToInt((Montage->GetPlayLength() + 1.0f) * 60.0f);
				for (int32 Frame = 0; Frame < Frames; ++Frame)
				{
					++GFrameCounter;
					Boss->GetMesh()->TickAnimation(1.0f / 60.0f, false);
					Boss->GetMesh()->RefreshBoneTransforms();
					Boss->GetMesh()->ConditionallyDispatchQueuedAnimEvents();
					bNotifyActivated |= NextAbility->bAbilityActive;
					World->GetTimerManager().Tick(1.0f / 60.0f);
				}
				TestTrue(TEXT("Montage playback automatically activates Notify"), bNotifyActivated);
				TestFalse(TEXT("Montage completion ends action"), Boss->ActionComponent->IsActionRunning());
				TestFalse(TEXT("Montage completion deactivates Ability"), NextAbility->bAbilityActive);
				TestNull(TEXT("Montage completion clears current Ability"), Boss->CombatComponent->CurrentAbilityInstance.Get());
			}
		}
	}
	else AddError(TEXT("Expected TutorialBoss basic attack Ability instance"));
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return !HasAnyErrors();
}
#endif
