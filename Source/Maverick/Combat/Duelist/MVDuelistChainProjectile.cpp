#include "Combat/Duelist/MVDuelistChainProjectile.h"

#include "Character/MVCharacterBase.h"
#include "Combat/Duelist/MVDuelistChainPullAbility.h"
#include "Combat/MVHitResolverSubsystem.h"
#include "Components/CapsuleComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/MVActionComponent.h"
#include "Components/MVStatComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/RootMotionSource.h"
#include "UObject/ConstructorHelpers.h"

AMVDuelistChainProjectile::AMVDuelistChainProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	PrimaryActorTick.TickGroup = TG_PostPhysics;
	UStaticMeshComponent* Tip = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ChainTip"));
	SetRootComponent(Tip);
	Tip->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> TipMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	Tip->SetStaticMesh(TipMesh.Object);
	Tip->SetRelativeScale3D(FVector(0.16f));
	Links = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ChainLinks"));
	Links->SetupAttachment(Tip);
	Links->SetAbsolute(false, false, true);
	Links->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> LinkMesh(
		TEXT("/Game/DuelistComplete/Skills/SM_DuelistChainLink.SM_DuelistChainLink"));
	Links->SetStaticMesh(LinkMesh.Object);
	if (LinkMesh.Object) Tip->SetMaterial(0, LinkMesh.Object->GetMaterial(0));
}

void AMVDuelistChainProjectile::Initialize(UMVDuelistChainPullAbility* InAbility, const FVector& Direction)
{
	Ability = InAbility;
	FlightDirection = Direction;
	SetActorTickEnabled(true);
	SetLifeSpan(InAbility->MaxRange / FMath::Max(1.0f, InAbility->ProjectileSpeed) + InAbility->PullDuration + 0.25f);
	UpdateLinks();
}

void AMVDuelistChainProjectile::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UMVDuelistChainPullAbility* Skill = Ability.Get();
	AMVCharacterBase* Boss = Skill ? Skill->GetOwnerCharacter() : nullptr;
	if (!Skill || !Skill->bAbilityActive || !IsValid(Boss)
		|| (Boss->StatComponent && Boss->StatComponent->IsDead()))
	{
		Destroy();
		return;
	}
	if (PullSourceId != 0)
	{
		AMVCharacterBase* Player = Victim.Get();
		if (!IsValid(Player) || (Player->StatComponent && Player->StatComponent->IsDead()))
		{
			Destroy();
			return;
		}
		PullTime += DeltaSeconds;
		const FVector Location = Player->GetActorLocation();
		StalledTime = FVector::DistSquared2D(Location, LastVictimLocation) < 0.25f ? StalledTime + DeltaSeconds : 0.0f;
		LastVictimLocation = Location;
		SetActorLocation(Location);
		if (FVector::DistSquared2D(Location, PullDestination) < 100.0f
			|| PullTime >= Skill->PullDuration + 0.05f || StalledTime >= 0.2f)
		{
			Destroy();
			return;
		}
	}
	else
	{
		const float Step = FMath::Min(Skill->ProjectileSpeed * DeltaSeconds, Skill->MaxRange - TravelDistance);
		const FVector Start = GetActorLocation();
		const FVector End = Start + FlightDirection * Step;
		FCollisionQueryParams Params(SCENE_QUERY_STAT(DuelistChainFlight), false, Boss);
		Params.AddIgnoredActor(this);
		FCollisionObjectQueryParams Objects;
		Objects.AddObjectTypesToQuery(ECC_WorldStatic);
		Objects.AddObjectTypesToQuery(ECC_WorldDynamic);
		Objects.AddObjectTypesToQuery(ECC_Pawn);
		FHitResult Hit;
		if (GetWorld()->SweepSingleByObjectType(Hit, Start, End, FQuat::Identity,
			Objects, FCollisionShape::MakeSphere(8.0f), Params))
		{
			SetActorLocation(Hit.Location);
			HitPlayer(Cast<AMVCharacterBase>(Hit.GetActor()), Hit);
		}
		else
		{
			SetActorLocation(End);
			TravelDistance += Step;
			if (TravelDistance >= Skill->MaxRange) Destroy();
		}
	}
	if (!IsActorBeingDestroyed()) UpdateLinks();
}

void AMVDuelistChainProjectile::HitPlayer(AMVCharacterBase* Player, const FHitResult& Hit)
{
	UMVDuelistChainPullAbility* Skill = Ability.Get();
	if (!IsValid(Player) || !Player->IsPlayerControlled() || Player->IsInvincible()
		|| (Player->StatComponent && Player->StatComponent->IsDead()))
	{
		Destroy();
		return;
	}
	FMVHitResolveRequest Request;
	Request.Attacker = Skill->GetOwnerCharacter();
	Request.Victim = Player;
	Request.AttackInstanceId = Skill->GetAttackInstanceId();
	Request.DamageMultiplier = Skill->AbilityData.DamageMultiplier;
	Request.GroggyDamageMultiplier = Skill->AbilityData.GroggyDamageMultiplier;
	Request.HitLocation = Hit.ImpactPoint;
	Request.ImpactNormal = Hit.ImpactNormal;
	FMVResolvedHitData Resolved;
	UMVHitResolverSubsystem* Resolver = UMVHitResolverSubsystem::Get(this);
	if (!Resolver || !Resolver->ResolveAttackHit(Request, Resolved) || IsActorBeingDestroyed()
		|| !Skill->bAbilityActive || (Player->StatComponent && Player->StatComponent->IsDead()))
	{
		Destroy();
		return;
	}
	UCharacterMovementComponent* Movement = Player->GetCharacterMovement();
	const FVector Start = Player->GetActorLocation();
	const FVector BossLocation = Request.Attacker->GetActorLocation();
	const float Distance = FVector::Dist2D(Start, BossLocation);
	if (!Movement || Distance <= Skill->StopDistance)
	{
		Destroy();
		return;
	}
	PullDestination = BossLocation + (Start - BossLocation).GetSafeNormal2D() * Skill->StopDistance;
	PullDestination.Z = Start.Z;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(DuelistChainPull), false, Player);
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(Request.Attacker);
	FHitResult Obstacle;
	const UCapsuleComponent* Capsule = Player->GetCapsuleComponent();
	if (GetWorld()->SweepSingleByChannel(Obstacle, Start, PullDestination, FQuat::Identity,
		Capsule->GetCollisionObjectType(), FCollisionShape::MakeCapsule(
			Capsule->GetScaledCapsuleRadius(), Capsule->GetScaledCapsuleHalfHeight()), Params))
	{
		PullDestination = FMath::Lerp(Start, PullDestination, FMath::Max(0.0f, Obstacle.Time - 0.02f));
	}
	if (Player->ActionComponent) Player->ActionComponent->CancelActiveAction(0.0f);
	Movement->StopMovementImmediately();
	TSharedPtr<FRootMotionSource_MoveToForce> Pull = MakeShared<FRootMotionSource_MoveToForce>();
	Pull->InstanceName = FName(*FString::Printf(TEXT("DuelistChain_%d"), Request.AttackInstanceId));
	Pull->Priority = 500;
	Pull->AccumulateMode = ERootMotionAccumulateMode::Override;
	Pull->StartLocation = Start;
	Pull->TargetLocation = PullDestination;
	Pull->Duration = Skill->PullDuration;
	Pull->bRestrictSpeedToExpected = true;
	Pull->FinishVelocityParams.Mode = ERootMotionFinishVelocityMode::SetVelocity;
	PullSourceId = Movement->ApplyRootMotionSource(Pull);
	Victim = Player;
	LastVictimLocation = Start;
	LockedController = Player->GetController();
	if (LockedController.IsValid()) LockedController->SetIgnoreMoveInput(true);
	SetActorLocation(Start);
}

void AMVDuelistChainProjectile::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (AMVCharacterBase* Player = Victim.Get())
	{
		UCharacterMovementComponent* Movement = Player->GetCharacterMovement();
		if (Movement && PullSourceId != 0 && Movement->GetRootMotionSourceByID(PullSourceId).IsValid())
		{
			Movement->RemoveRootMotionSourceByID(PullSourceId);
			Movement->StopMovementImmediately();
		}
	}
	if (LockedController.IsValid()) LockedController->SetIgnoreMoveInput(false);
	Super::EndPlay(EndPlayReason);
}

void AMVDuelistChainProjectile::UpdateLinks()
{
	UMVDuelistChainPullAbility* Skill = Ability.Get();
	AMVCharacterBase* Boss = Skill ? Skill->GetOwnerCharacter() : nullptr;
	if (!Boss || !Links->GetStaticMesh()) return;
	const USkeletalMeshComponent* Mesh = Boss->GetMesh();
	const FVector Start = Mesh && Mesh->DoesSocketExist(Skill->LaunchSocket)
		? Mesh->GetSocketLocation(Skill->LaunchSocket) : Boss->GetActorLocation();
	const FVector Offset = GetActorLocation() - Start;
	const int32 Count = FMath::Clamp(FMath::CeilToInt(Offset.Size() / 14.0f), 1, 180);
	while (Links->GetInstanceCount() < Count) Links->AddInstance(FTransform::Identity);
	while (Links->GetInstanceCount() > Count) Links->RemoveInstance(Links->GetInstanceCount() - 1);
	const FQuat Direction = Offset.Rotation().Quaternion();
	for (int32 Index = 0; Index < Count; ++Index)
	{
		const FQuat Rotation = Direction * FQuat(FVector::ForwardVector, Index % 2 ? HALF_PI : 0.0f);
		const FVector Location = Start + Offset * ((Index + 0.5f) / Count);
		Links->UpdateInstanceTransform(Index, FTransform(Rotation, Location), true, Index == Count - 1);
	}
}
