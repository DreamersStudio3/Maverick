#include "Combat/Duelist/MVDuelistResonanceComponent.h"

#include "AIController.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Character/MVCharacterBase.h"
#include "Combat/MVHitResolverSubsystem.h"
#include "Components/CapsuleComponent.h"
#include "Components/MVStatComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "UI/BossHUD/MVResonanceScreenWidget.h"

UMVDuelistResonanceComponent::UMVDuelistResonanceComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UMVDuelistResonanceComponent::BeginPlay()
{
	Super::BeginPlay();
	Character = Cast<AMVCharacterBase>(GetOwner());
	Stat = Character ? Character->StatComponent : nullptr;
	if (!Stat) return;
	LastObservedHP = Stat->CurrentHP;
	Stat->OnHPChanged.AddUniqueDynamic(this, &ThisClass::HandleHPChanged);
	Stat->OnGroggyStarted.AddUniqueDynamic(this, &ThisClass::HandleGroggy);
	Stat->OnDead.AddUniqueDynamic(this, &ThisClass::HandleDead);
}

void UMVDuelistResonanceComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	CancelResonance();
	if (Stat)
	{
		Stat->OnHPChanged.RemoveDynamic(this, &ThisClass::HandleHPChanged);
		Stat->OnGroggyStarted.RemoveDynamic(this, &ThisClass::HandleGroggy);
		Stat->OnDead.RemoveDynamic(this, &ThisClass::HandleDead);
	}
	Super::EndPlay(Reason);
}

void UMVDuelistResonanceComponent::HandleHPChanged(float HP, float MaxHP)
{
	const bool bHPDecreased = HP < LastObservedHP;
	LastObservedHP = HP;
	if (bHPDecreased && !bUsed && !bPending && HP > 0.0f && MaxHP > 0.0f && HP <= MaxHP * 0.5f)
	{
		bPending = true;
	}
}

bool UMVDuelistResonanceComponent::IsResonanceActive() const
{
	return Phase == EMVDuelistResonancePhase::Jump || Phase == EMVDuelistResonancePhase::Channel || Phase == EMVDuelistResonancePhase::Groggy;
}

bool UMVDuelistResonanceComponent::StartResonance()
{
	return BeginResonance(false);
}

bool UMVDuelistResonanceComponent::StartResonanceForTest()
{
	return BeginResonance(true);
}

bool UMVDuelistResonanceComponent::BeginResonance(bool bForTest)
{
	if ((!bForTest && !IsResonancePending()) || IsResonanceActive() || !Character || !Stat || Stat->IsDead() || Stat->IsGroggy()) return false;
	if (!bForTest)
	{
		bPending = false;
		bUsed = true;
	}
	Phase = EMVDuelistResonancePhase::Jump;
	JumpElapsed = 0.0f;
	JumpStart = Character->GetActorLocation();
	JumpEnd = ArenaCenter + FVector(0, 0, Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
	FHitResult Floor;
	FCollisionQueryParams Query(SCENE_QUERY_STAT(MVDuelistResonanceFloor), false, Character);
	if (GetWorld()->LineTraceSingleByObjectType(Floor, ArenaCenter + FVector(0, 0, 1000), ArenaCenter - FVector(0, 0, 1000), FCollisionObjectQueryParams(ECC_WorldStatic), Query))
		JumpEnd.Z = Floor.ImpactPoint.Z + Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 1.0f;
	if (AAIController* AI = Cast<AAIController>(Character->GetController())) AI->StopMovement();
	UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
	SavedMovementMode = Movement->MovementMode;
	SavedCustomMovementMode = Movement->CustomMovementMode;
	bMovementSaved = true;
	Movement->StopMovementImmediately();
	Movement->DisableMovement();
	PlayPose(JumpMontage);
	SetComponentTickEnabled(true);
	return true;
}

void UMVDuelistResonanceComponent::PlayPose(UAnimMontage* Montage)
{
	UAnimInstance* Anim = Character->GetMesh()->GetAnimInstance();
	if (!Anim) return;
	if (PlayingMontage) Anim->Montage_Stop(0.15f, PlayingMontage);
	PlayingMontage = Montage;
	if (Montage) Anim->Montage_Play(Montage);
}

void UMVDuelistResonanceComponent::BeginChannel()
{
	Phase = EMVDuelistResonancePhase::Channel;
	ChannelRemaining = FMath::Max(0.1f, ChannelDuration);
	ChannelDeadline = GetWorld()->GetTimeSeconds() + ChannelRemaining;
	Stat->BeginPoiseBreakGroggyWindow(ChannelPoise);
	PlayPose(ChannelMontage);
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC || !PC->IsLocalController() || !PC->GetLocalPlayer()) continue;
		UMVResonanceScreenWidget* Screen = CreateWidget<UMVResonanceScreenWidget>(PC, UMVResonanceScreenWidget::StaticClass());
		Screen->Duration = ChannelRemaining;
		Screen->Remaining = ChannelRemaining;
		Screen->Poise = ChannelPoise;
		Screen->SetVisibility(ESlateVisibility::HitTestInvisible);
		Screen->AddToPlayerScreen(1000);
		Screens.Add(Screen);
	}
	if (WarningMaterial)
	{
		WarningDisc = NewObject<UStaticMeshComponent>(GetOwner());
		WarningDisc->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder")));
		WarningDisc->SetMaterial(0, WarningMaterial);
		WarningDisc->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		WarningDisc->RegisterComponent();
		WarningDisc->SetWorldLocation(JumpEnd - FVector(0, 0, Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() - 3));
		WarningDisc->SetWorldScale3D(FVector(ArenaRadius / 50.0f, ArenaRadius / 50.0f, 0.02f));
	}
	if (EnergyMaterial)
	{
		for (int32 Index = 0; Index < 6; ++Index)
		{
			UStaticMeshComponent* Orb = NewObject<UStaticMeshComponent>(GetOwner());
			Orb->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere")));
			Orb->SetMaterial(0, EnergyMaterial);
			Orb->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Orb->RegisterComponent();
			Orb->SetWorldScale3D(FVector(0.2f));
			EnergyOrbs.Add(Orb);
		}
	}
}

void UMVDuelistResonanceComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* TickFunction)
{
	Super::TickComponent(DeltaTime, TickType, TickFunction);
	if (!Character || !Stat || Stat->IsDead()) { CancelResonance(); return; }
	if (Phase == EMVDuelistResonancePhase::Jump)
	{
		JumpElapsed += DeltaTime;
		const float Alpha = FMath::Clamp(JumpElapsed / FMath::Max(0.1f, JumpDuration), 0.0f, 1.0f);
		Character->SetActorLocation(FMath::Lerp(JumpStart, JumpEnd, Alpha) + FVector(0, 0, 4 * JumpHeight * Alpha * (1 - Alpha)));
		if (Alpha >= 1.0f) BeginChannel();
	}
	else if (Phase == EMVDuelistResonancePhase::Channel)
	{
		ChannelRemaining = FMath::Max(0.0, ChannelDeadline - GetWorld()->GetTimeSeconds());
		for (UMVResonanceScreenWidget* Screen : Screens) { Screen->Remaining = ChannelRemaining; Screen->Poise = Stat->ConstantPoise; }
		const float Progress = 1.0f - ChannelRemaining / FMath::Max(0.1f, ChannelDuration);
		for (int32 Index = 0; Index < EnergyOrbs.Num(); ++Index)
		{
			const float Angle = GetWorld()->GetTimeSeconds() * 3 + Index * PI / 3;
			const float Radius = FMath::Lerp(250.0f, 55.0f, Progress);
			EnergyOrbs[Index]->SetWorldLocation(Character->GetActorLocation() + FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 120 + FMath::Sin(Angle * 2) * 50));
			EnergyOrbs[Index]->SetWorldScale3D(FVector(0.2f + Progress * 0.45f));
		}
		if (ChannelRemaining <= 0.0f) Explode();
	}
	else if (Phase == EMVDuelistResonancePhase::Groggy && !Stat->IsGroggy()) CancelResonance();
}

void UMVDuelistResonanceComponent::HandleGroggy()
{
	if (Phase != EMVDuelistResonancePhase::Channel) return;
	Phase = EMVDuelistResonancePhase::Groggy;
	Stat->EndPoiseBreakGroggyWindow();
	StopPresentation();
	PlayPose(GroggyMontage);
}

void UMVDuelistResonanceComponent::Explode()
{
	// 실패 기믹의 전멸 피해: 투사체·방어력·회피 판정 없이 전장 내 플레이어에게 고정 피해 1회
	UMVHitResolverSubsystem* Resolver = UMVHitResolverSubsystem::Get(GetWorld());
	if (EnergyMaterial)
	{
		AActor* Burst = GetWorld()->SpawnActor<AActor>();
		UStaticMeshComponent* Sphere = NewObject<UStaticMeshComponent>(Burst);
		Burst->SetRootComponent(Sphere);
		Sphere->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere")));
		Sphere->SetMaterial(0, EnergyMaterial);
		Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Sphere->RegisterComponent();
		Sphere->SetWorldLocation(Character->GetActorLocation());
		Sphere->SetWorldScale3D(FVector(ArenaRadius / 50.0f));
		Burst->SetLifeSpan(0.35f);
	}
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		AMVCharacterBase* Player = PC ? Cast<AMVCharacterBase>(PC->GetPawn()) : nullptr;
		if (!Resolver || !Player || !Player->StatComponent || Player->StatComponent->IsDead()
			|| FVector::DistSquared2D(Player->GetActorLocation(), ArenaCenter) > FMath::Square(ArenaRadius)) continue;
		FMVDirectDamageRequest Request;
		Request.Attacker = Character;
		Request.Victim = Player;
		Request.FinalDamage = Player->StatComponent->MaxHP * FMath::Clamp(FailureDamageRatio, 0.0f, 1.0f);
		Request.HitLocation = Player->GetActorLocation();
		FMVResolvedHitData Resolved;
		if (!Resolver->ResolveDirectDamage(Request, Resolved)) continue;
		for (UMVResonanceScreenWidget* Screen : Screens)
			if (Screen->GetOwningPlayer() == PC) Screen->StartBlackout(BlackoutDuration);
	}
	CancelResonance();
}

void UMVDuelistResonanceComponent::StopPresentation()
{
	for (UMVResonanceScreenWidget* Screen : Screens) if (Screen && !Screen->bBlackout) Screen->RemoveFromParent();
	Screens.Reset();
	if (WarningDisc) WarningDisc->DestroyComponent();
	WarningDisc = nullptr;
	for (UStaticMeshComponent* Orb : EnergyOrbs) if (Orb) Orb->DestroyComponent();
	EnergyOrbs.Reset();
	if (Character && Character->GetMesh()->GetAnimInstance() && PlayingMontage)
		Character->GetMesh()->GetAnimInstance()->Montage_Stop(0.15f, PlayingMontage);
	PlayingMontage = nullptr;
}

void UMVDuelistResonanceComponent::RestoreMovement()
{
	if (!bMovementSaved || !Character) return;
	bMovementSaved = false;
	if (!Stat || !Stat->IsDead()) Character->GetCharacterMovement()->SetMovementMode(static_cast<EMovementMode>(SavedMovementMode), SavedCustomMovementMode);
}

void UMVDuelistResonanceComponent::CancelResonance()
{
	if (!IsResonanceActive()) return;
	Phase = EMVDuelistResonancePhase::Finished;
	if (Stat) Stat->EndPoiseBreakGroggyWindow();
	StopPresentation();
	RestoreMovement();
	SetComponentTickEnabled(false);
}

void UMVDuelistResonanceComponent::HandleDead() { bPending = false; CancelResonance(); }
