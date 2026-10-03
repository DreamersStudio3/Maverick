#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Animation/AnimSingleNodeInstance.h"
#include "Animation/AnimSequence.h"
#include "AudioMixerBlueprintLibrary.h"
#include "AudioDeviceManager.h"
#include "Camera/CameraActor.h"
#include "Character/NPC/Training/MVTrainingDummy.h"
#include "Components/AudioComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/MVInputManagerComponent.h"
#include "Components/MVStatComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Editor.h"
#include "EngineUtils.h"
#include "Engine/SkeletalMesh.h"
#include "GameFramework/PlayerController.h"
#include "HighResScreenshot.h"
#include "Misc/Paths.h"
#include "Sound/SoundSubmix.h"
#include "Tags/MVGameplayTags.h"
#include "Tests/AutomationEditorCommon.h"
#include "UObject/StrongObjectPtr.h"

namespace
{
// 실제 레벨·플레이어·몽타주 Notify·무기 판정 사용; 다른 NPC만 PIE 사본에서 제거
class FMVTrainingDummyPIECommand : public IAutomationLatentCommand
{
public:
	explicit FMVTrainingDummyPIECommand(FAutomationTestBase* InTest) : Test(InTest) {}

	virtual bool Update() override
	{
		UWorld* World = GEditor ? GEditor->PlayWorld : nullptr;
		const double RealNow = FPlatformTime::Seconds();
		if (!Started) { Started = RealNow; }
		if (RealNow - Started > 180.0)
		{
			Test->AddError(TEXT("TrainingDummy PIE timeout"));
			return true;
		}
		if (!World || !World->HasBegunPlay()) { return false; }
		const double Now = World->GetTimeSeconds();
		if (Stage == 0)
		{
			APlayerController* PC = World->GetFirstPlayerController();
			Player = PC ? Cast<AMVCharacterBase>(PC->GetPawn()) : nullptr;
			if (!Player.IsValid() || World->GetTimeSeconds() < 3.0f) { return false; }
			int32 Count = 0;
			for (TActorIterator<AMVTrainingDummy> It(World); It; ++It) { Dummy = *It; ++Count; }
			if (!Test->TestEqual(TEXT("Exactly one placed dummy"), Count, 1)) { return true; }
			for (TActorIterator<AMVCharacterBase> It(World); It; ++It)
			{
				if (*It != Player.Get() && *It != Dummy.Get()) { It->Destroy(); }
			}
			const float FloorZ = Dummy->GetActorLocation().Z - Dummy->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
			// 기본 검의 강공격을 정면 근거리에서 재현
			AttackPosition = FVector(Dummy->GetActorLocation().X - 120.0f, Dummy->GetActorLocation().Y,
				FloorZ + Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 2.0f);
			Player->SetActorLocation(AttackPosition, false, nullptr, ETeleportType::TeleportPhysics);
			Player->SetActorRotation(FRotator::ZeroRotator);
			PC->SetControlRotation(FRotator::ZeroRotator);
			const FVector CameraPosition = Dummy->GetActorLocation() + FVector(-450.0f, -450.0f, 180.0f);
			ACameraActor* Camera = World->SpawnActor<ACameraActor>(CameraPosition,
				(Dummy->GetActorLocation() - CameraPosition).Rotation());
			PC->SetViewTarget(Camera);
			Test->AddInfo(FString::Printf(TEXT("TrainingDummyBounds Origin=%s Extent=%s Mesh=%s"),
				*Dummy->GetMesh()->Bounds.Origin.ToString(), *Dummy->GetMesh()->Bounds.BoxExtent.ToString(),
				*Dummy->GetMesh()->GetSkeletalMeshAsset()->GetName()));
			Player->BeginInvincibility();
			InitialHP = Dummy->StatComponent->CurrentHP;
			Submix.Reset(NewObject<USoundSubmix>());
			Submix->bAutoDisable = false;
			GEngine->GetAudioDeviceManager()->RegisterSoundSubmix(Submix.Get());
			Dummy->HitAudio->SetSubmixSend(Submix.Get(), 1.0f);
			UAudioMixerBlueprintLibrary::StartRecordingOutput(World, 15.0f, Submix.Get());
			Deadline = Now + 2.0;
			Stage = 1;
			return false;
		}
		if (!Dummy.IsValid() || !Player.IsValid()) { Test->AddError(TEXT("PIE actor lost")); return true; }
		UAnimSingleNodeInstance* Instance = Dummy->GetMesh()->GetSingleNodeInstance();
		if (Stage == 1 && Now >= Deadline)
		{
			Test->TestTrue(TEXT("Idle animation playing before attack"), Instance && Instance->GetCurrentAsset() == Dummy->IdleAnimation);
			Test->TestEqual(TEXT("No synthetic damage before attack"), Dummy->HitCount, 0);
			BaselineSpine = Dummy->GetMesh()->GetBoneQuaternion(TEXT("Bip001-Spine1"));
			Capture(TEXT("training-idle"));
			Deadline = Now + 0.25;
			Stage = 7;
		}
		else if (Stage == 7 && Now >= Deadline)
		{
			SubmitAttack();
			Deadline = Now + 5.0;
			Stage = 2;
		}
		else if (Stage == 2 || Stage == 4)
		{
			const int32 ExpectedHits = Stage == 2 ? 1 : 2;
			if (Dummy->HitCount >= ExpectedHits)
			{
				Test->TestEqual(TEXT("One resolved hit per swing"), Dummy->HitCount, ExpectedHits);
				Test->TestTrue(TEXT("Resolved melee hit starts reaction"), Instance && Instance->GetCurrentAsset() == Dummy->HitAnimation);
				Test->TestTrue(TEXT("Resolved melee hit starts sound"), Dummy->HitAudio->IsPlaying());
				Test->AddInfo(FString::Printf(TEXT("TrainingDummyHit Count=%d Player=%s Sound=%s Position=%s"),
					Dummy->HitCount, *Player->GetName(), *GetNameSafe(Dummy->HitAudio->Sound), *Player->GetActorLocation().ToString()));
				Deadline = Now + 2.5;
				++Stage;
			}
			else if (Now > Deadline)
			{
				Test->AddError(TEXT("Player attack did not reach dummy through weapon trace"));
				StopRecording(World);
				return true;
			}
		}
		else if (Stage == 3 || Stage == 5)
		{
			if (Instance && Instance->GetCurrentAsset() == Dummy->HitAnimation && Instance->GetCurrentTime() > 0.1f)
			{
				bPoseChanged |= !BaselineSpine.Equals(Dummy->GetMesh()->GetBoneQuaternion(TEXT("Bip001-Spine1")), 0.001f);
				if (!bCapturedHit) { Capture(TEXT("training-hit")); bCapturedHit = true; }
			}
			if (Now >= Deadline)
			{
				Test->TestTrue(TEXT("Reaction moved spine bones"), bPoseChanged);
				Test->TestTrue(TEXT("Reaction returns to idle"), Instance && Instance->GetCurrentAsset() == Dummy->IdleAnimation);
				Test->TestEqual(TEXT("Training target retains HP"), Dummy->StatComponent->CurrentHP, InitialHP);
				Test->TestFalse(TEXT("Training target survives"), Dummy->StatComponent->IsDead());
				if (Stage == 3)
				{
					Player->SetActorLocation(AttackPosition, false, nullptr, ETeleportType::TeleportPhysics);
					Player->SetActorRotation(FRotator::ZeroRotator);
					Deadline = Now + 0.5;
					Stage = 8;
				}
				else { StopRecording(World); Deadline = Now + 2.0; Stage = 6; }
			}
		}
		else if (Stage == 6 && Now > Deadline)
		{
			return true;
		}
		else if (Stage == 8 && Now > Deadline)
		{
			SubmitAttack(); Deadline = Now + 5.0; Stage = 4;
		}
		return false;
	}

private:
	void SubmitAttack()
	{
		Dummy->HitAudio->SetSubmixSend(Submix.Get(), 1.0f);
		bPoseChanged = false;
		BaselineSpine = Dummy->GetMesh()->GetBoneQuaternion(TEXT("Bip001-Spine1"));
		Test->TestTrue(TEXT("Existing player input router accepts heavy attack"),
			Player->InputManagerComponent->SubmitActionInput(MVGameplayTags::Action_Input_HeavyAttack));
	}
	void Capture(const TCHAR* Name)
	{
		FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("AssetValidation/TrainingDummy") / Name, false, false);
	}
	void StopRecording(UWorld* World)
	{
		UAudioMixerBlueprintLibrary::StopRecordingOutput(World, EAudioRecordingExportType::WavFile,
			TEXT("training-hit-output"), FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() / TEXT("AssetValidation/TrainingDummy")), Submix.Get());
	}
	FAutomationTestBase* Test;
	TWeakObjectPtr<AMVTrainingDummy> Dummy;
	TWeakObjectPtr<AMVCharacterBase> Player;
	TStrongObjectPtr<USoundSubmix> Submix;
	FQuat BaselineSpine;
	FVector AttackPosition;
	double Started = 0.0, Deadline = 0.0;
	float InitialHP = 0.0f;
	int32 Stage = 0;
	bool bPoseChanged = false, bCapturedHit = false;
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMVTrainingDummyPIETest, "Maverick.Combat.TrainingDummy.PlayerMeleePIE",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMVTrainingDummyPIETest::RunTest(const FString& Parameters)
{
	// 기존 기본 Chooser의 빈 바인딩 진단 1건; 실제 장착 무기 Chooser의 공격 실행은 아래에서 별도 검증
	AddExpectedError(TEXT("CHT_Attack_Player.uasset: Missing property binding."), EAutomationExpectedErrorFlags::Contains, 1);
	ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(TEXT("/Game/Characters/NPC/BaseBoss/Level/BaseBossTestLevel")));
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(FMVTrainingDummyPIECommand(this));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}
#endif
