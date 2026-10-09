#include "Combat/Projectile/MVProjectileBase.h"

#include "Character/MVCharacterBase.h"
#include "Character/NPC/Enemy/MVEnemy.h"
#include "Character/PC/MVPlayerCharacter.h"
#include "Combat/MVHitResolverSubsystem.h"
#include "Combat/Projectile/MVProjectileTargeting.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/ProjectileMovementComponent.h"

AMVProjectileBase::AMVProjectileBase()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	SetRootComponent(Collision);
	Collision->InitSphereRadius(12.0f);
	Collision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Collision->SetCollisionObjectType(ECC_WorldDynamic);
	Collision->SetCollisionResponseToAllChannels(ECR_Ignore);
	Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Collision->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	Collision->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	Collision->SetGenerateOverlapEvents(true);
	Collision->OnComponentBeginOverlap.AddDynamic(this, &AMVProjectileBase::HandleOverlap);

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(Collision);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(
		TEXT("ProjectileMovement"));
	ProjectileMovement->SetUpdatedComponent(Collision);
	ProjectileMovement->bAutoActivate = false;
	ProjectileMovement->InitialSpeed = 1200.0f;
	ProjectileMovement->MaxSpeed = 1200.0f;
	ProjectileMovement->ProjectileGravityScale = 0.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bSweepCollision = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->HomingAccelerationMagnitude = 3000.0f;
	ProjectileMovement->OnProjectileStop.AddDynamic(
		this, &AMVProjectileBase::HandleProjectileStop);
}

bool AMVProjectileBase::InitializeProjectile(const FMVProjectileLaunchContext& Context)
{
	if (bInitialized || bActive || bEnding)
	{
		return false;
	}

	AMVCharacterBase* Attacker = Context.Attacker.Get();
	const FVector& Direction = Context.LaunchDirection;

	const bool bValidDirection =
		FMath::IsFinite(Direction.X) &&
		FMath::IsFinite(Direction.Y) &&
		FMath::IsFinite(Direction.Z) &&
		!Direction.IsNearlyZero();

	const bool bValidTeam =
		(Context.Team == EMVProjectileTeam::Player &&
			Cast<AMVPlayerCharacter>(Attacker) != nullptr) ||
		(Context.Team == EMVProjectileTeam::Enemy &&
			Cast<AMVEnemy>(Attacker) != nullptr);

	if (!IsValid(Attacker) ||
		!bValidTeam ||
		Context.AttackInstanceId == INDEX_NONE ||
		!Context.WeaponSnapshot.bValid ||
		!bValidDirection)
	{
		return false;
	}

	LaunchContext = Context;
	LaunchContext.LaunchDirection = Direction.GetSafeNormal();

	SetOwner(Attacker);
	SetInstigator(Attacker);
	Collision->IgnoreActorWhenMoving(Attacker, true);

	bInitialized = true;
	return true;
}

bool AMVProjectileBase::ActivateProjectile()
{
	if (!bInitialized ||
		bActive ||
		bEnding ||
		!IsValid(LaunchContext.Attacker.Get()) ||
		ProjectileMovement->InitialSpeed <= 0.0f ||
		MaxLifeSeconds <= 0.0f)
	{
		return false;
	}

	SetActorRotation(LaunchContext.LaunchDirection.Rotation());
	ProjectileMovement->Velocity =
		LaunchContext.LaunchDirection * ProjectileMovement->InitialSpeed;

	ProjectileMovement->bIsHomingProjectile = false;
	ProjectileMovement->HomingTargetComponent.Reset();

	if (bEnableHoming)
	{
		AMVCharacterBase* Target = Cast<AMVCharacterBase>(LaunchContext.HomingTarget.Get());

		if (MVProjectileTargeting::IsLivingHostile(
				LaunchContext.Attacker.Get(), LaunchContext.Team, Target))
		{
			ProjectileMovement->HomingTargetComponent = Target->GetRootComponent();
			ProjectileMovement->bIsHomingProjectile = true;
		}
		else
		{
			ClearHomingTarget();
		}
	}

	bActive = true;
	ProjectileMovement->Activate(true);
	SetActorTickEnabled(true);
	SetLifeSpan(MaxLifeSeconds);

	Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Collision->UpdateOverlaps();

	// 활성화 전에 이미 발사 위치에 겹쳐 있던 캐릭터를 처리한다.
	TArray<FOverlapResult> InitialOverlaps;
	const FCollisionQueryParams QueryParams(
		SCENE_QUERY_STAT(MVProjectileInitialOverlap), false, this);

	if (UWorld* World = GetWorld())
	{
		World->OverlapMultiByObjectType(
			InitialOverlaps,
			Collision->GetComponentLocation(),
			Collision->GetComponentQuat(),
			FCollisionObjectQueryParams::AllDynamicObjects,
			FCollisionShape::MakeSphere(Collision->GetScaledSphereRadius()),
			QueryParams);
	}

	for (const FOverlapResult& Overlap : InitialOverlaps)
	{
		if (!bActive || bEnding)
		{
			break;
		}

		UPrimitiveComponent* OtherComponent = Overlap.GetComponent();
		AActor* OtherActor = Overlap.GetActor();
		if (!OtherComponent ||
			!Cast<AMVCharacterBase>(OtherActor) ||
			!OtherComponent->GetGenerateOverlapEvents() ||
			Collision->GetCollisionResponseToChannel(
				OtherComponent->GetCollisionObjectType()) == ECR_Ignore ||
			OtherComponent->GetCollisionResponseToChannel(
				Collision->GetCollisionObjectType()) == ECR_Ignore)
		{
			continue;
		}

		HandleOverlap(
			Collision, OtherActor, OtherComponent,
			INDEX_NONE, false, FHitResult());
	}

	return true;
}

void AMVProjectileBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bActive || bEnding)
	{
		return;
	}

	if (!IsValid(LaunchContext.Attacker.Get()))
	{
		FinishProjectile();
		return;
	}

	if (ProjectileMovement->bIsHomingProjectile)
	{
		AMVCharacterBase* Target =
			Cast<AMVCharacterBase>(LaunchContext.HomingTarget.Get());

		if (!MVProjectileTargeting::IsLivingHostile(
				LaunchContext.Attacker.Get(), LaunchContext.Team, Target) ||
			ProjectileMovement->HomingTargetComponent.Get() !=
				Target->GetRootComponent())
		{
			ClearHomingTarget();
		}
	}
}

void AMVProjectileBase::ClearHomingTarget()
{
	ProjectileMovement->bIsHomingProjectile = false;
	ProjectileMovement->HomingTargetComponent.Reset();
	LaunchContext.HomingTarget.Reset();
}

bool AMVProjectileBase::CanHitCharacter(const AMVCharacterBase* Victim) const
{
	return MVProjectileTargeting::IsLivingHostile(
		LaunchContext.Attacker.Get(), LaunchContext.Team, Victim) &&
		!Victim->IsInvincible();
}

void AMVProjectileBase::HandleOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!bActive || bEnding || bResolvingHit)
	{
		return;
	}

	AMVCharacterBase* Victim = Cast<AMVCharacterBase>(OtherActor);
	const bool bCanHit = CanHitCharacter(Victim);
	if (!bCanHit)
	{
		return;
	}

	const TWeakObjectPtr<AMVCharacterBase> VictimKey(Victim);

	if (HitCharacters.Contains(VictimKey))
	{
		return;
	}

	AMVCharacterBase* Attacker = LaunchContext.Attacker.Get();
	if (!IsValid(Attacker))
	{
		FinishProjectile();
		return;
	}

	UMVHitResolverSubsystem* Resolver = UMVHitResolverSubsystem::Get(this);
	if (!IsValid(Resolver))
	{
		return;
	}

	FMVHitResolveRequest Request;
	Request.Attacker = Attacker;
	Request.Victim = Victim;
	Request.AttackInstanceId = LaunchContext.AttackInstanceId;
	Request.AttackTypes = LaunchContext.AttackTypes;
	Request.DamageMultiplier = LaunchContext.DamageMultiplier;
	Request.GroggyDamageMultiplier = LaunchContext.GroggyDamageMultiplier;
	Request.PoiseDamage = LaunchContext.PoiseDamage;
	Request.HitReactionType = LaunchContext.HitReactionType;
	Request.HitLaunchData = LaunchContext.HitLaunchData;
	if (bFromSweep)
	{
		Request.HitLocation = SweepResult.ImpactPoint;
		Request.ImpactNormal = SweepResult.ImpactNormal;
	}
	else
	{
		Request.HitLocation = Victim->GetActorLocation();
		Request.ImpactNormal = FVector::ZeroVector;
	}
	Request.bUseSourceWeaponSnapshot = true;
	Request.SourceWeaponSnapshot = LaunchContext.WeaponSnapshot;
	Request.bUseIncomingDirection = true;
	Request.IncomingDirection = ProjectileMovement->Velocity.IsNearlyZero()
		                            ? LaunchContext.LaunchDirection
		                            : ProjectileMovement->Velocity;

	// Resolver와 피격 이벤트가 다른 겹침을 다시 발생시켜도 중복 처리하지 않는다.
	HitCharacters.Add(VictimKey);
	bResolvingHit = true;
	if (!bPierceCharacters)
	{
		bActive = false;
	}

	FMVResolvedHitData ResolvedHit;
	const bool bResolved = Resolver->ResolveAttackHit(Request, ResolvedHit);

	bResolvingHit = false;

	if (!bResolved)
	{
		HitCharacters.Remove(VictimKey);
		if (!bPierceCharacters && !bEnding)
		{
			bActive = true;
		}
		return;
	}

	if (bPierceCharacters &&
		Victim == LaunchContext.HomingTarget.Get())
	{
		ClearHomingTarget();
	}

	OnProjectileHit.Broadcast(this, ResolvedHit);

	if (!bPierceCharacters)
	{
		FinishProjectile();
	}
}

void AMVProjectileBase::HandleProjectileStop(const FHitResult& /*ImpactResult*/)
{
	if (bActive && !bEnding)
	{
		FinishProjectile();
	}
}

void AMVProjectileBase::FinishProjectile()
{
	if (bEnding)
	{
		return;
	}

	bEnding = true;
	bActive = false;
	Collision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ProjectileMovement->StopMovementImmediately();
	ProjectileMovement->Deactivate();
	SetActorTickEnabled(false);
	Destroy();
}

void AMVProjectileBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (bInitialized && !bFinishNotified)
	{
		bFinishNotified = true;
		OnProjectileFinished.Broadcast(this, LaunchContext.AttackInstanceId);
	}

	Super::EndPlay(EndPlayReason);
}
