#include "Character/NPC/Training/MVTrainingDummy.h"

#include "Animation/AnimSequence.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "Components/AudioComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/MVStatComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Sound/SoundBase.h"

AMVTrainingDummy::AMVTrainingDummy()
{
	PrimaryActorTick.bCanEverTick = true;
	GetCapsuleComponent()->InitCapsuleSize(40.0f, 100.0f);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Block);
	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -100.0f));
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	GetCharacterMovement()->DefaultLandMovementMode = MOVE_None;
	GetCharacterMovement()->GravityScale = 0.0f;
	StatComponent->bLoadStatsOnBeginPlay = false;
	SetCharacterMovementRotationActive(false, false);
	HitAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("HitAudio"));
	HitAudio->SetupAttachment(RootComponent);
	HitAudio->bAutoActivate = false;
	HitAudio->bOverrideAttenuation = true;
	HitAudio->AttenuationOverrides.bAttenuate = true;
	HitAudio->AttenuationOverrides.bSpatialize = true;
	HitAudio->AttenuationOverrides.AttenuationShapeExtents = FVector(200.0f);
	HitAudio->AttenuationOverrides.FalloffDistance = 2620.0f;
}

void AMVTrainingDummy::BeginPlay()
{
	Super::BeginPlay();
	GetCharacterMovement()->DisableMovement();
	PlayIdle();
}

void AMVTrainingDummy::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	OnDamaged.RemoveDynamic(this, &AMVTrainingDummy::HandleTrainingHit);
	Super::EndPlay(EndPlayReason);
}

void AMVTrainingDummy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// hitstop을 포함한 실제 피격 동작 종료 기준
	UAnimSingleNodeInstance* Animation = GetMesh()->GetSingleNodeInstance();
	if (Animation && Animation->GetCurrentAsset() == HitAnimation && !Animation->IsPlaying())
	{
		PlayIdle();
	}
}

void AMVTrainingDummy::BindDamageHandlers()
{
	OnDamaged.AddUniqueDynamic(this, &AMVTrainingDummy::HandleTrainingHit);
}

void AMVTrainingDummy::HandleTrainingHit(const FMVResolvedHitData& HitData)
{
	++HitCount;
	if (HitAnimation)
	{
		GetMesh()->PlayAnimation(HitAnimation, false);
	}
	if (!HitSounds.IsEmpty())
	{
		HitAudio->Stop();
		HitAudio->SetSound(HitSounds[FMath::RandHelper(HitSounds.Num())]);
		HitAudio->SetPitchMultiplier(FMath::FRandRange(0.8f, 1.0f));
		HitAudio->SetVolumeMultiplier(0.8f * FMath::FRandRange(0.9f, 1.0f));
		HitAudio->Play();
	}
}

void AMVTrainingDummy::PlayIdle()
{
	if (IdleAnimation)
	{
		GetMesh()->PlayAnimation(IdleAnimation, true);
	}
}
