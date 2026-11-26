#include "System/MVFieldEnemySpawnPoint.h"

#include "Character/NPC/Enemy/MVEnemy.h"
#include "Cinematics/Boss/MVBossIntroTrigger.h"
#include "Combat/Projectile/MVProjectileBase.h"
#include "Components/SceneComponent.h"
#include "Components/MVStatComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Interaction/ItemPickup/MVItemPickup.h"
#include "System/MVEnemyRewardSubsystem.h"
#include "System/MVWorldStateSubsystem.h"
#include "Tables/MVEnemyDropTableTypes.h"

AMVFieldEnemySpawnPoint::AMVFieldEnemySpawnPoint()
{
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("SpawnAnchor")));
}

void AMVFieldEnemySpawnPoint::BeginPlay()
{
	Super::BeginPlay();
	if (!HasAuthority() || FieldId.IsNone() || SpawnId.IsNone())
	{
		return;
	}

	EnemyTransform = GetActorTransform();
	if (IsValid(InitialEnemy))
	{
		for (TActorIterator<AMVBossIntroTrigger> It(GetWorld()); It; ++It)
		{
			if (It->Boss == InitialEnemy)
			{
				bBoss = true;
				break;
			}
		}
		if (IsObservationOnly())
		{
			if (!IsConsumed())
			{
				ObserveEnemy(*InitialEnemy);
			}
			InitialEnemy = nullptr;
			return;
		}
		EnemyClass = InitialEnemy->GetClass();
		EnemyTransform = InitialEnemy->GetActorTransform();
		PatrolPath = InitialEnemy->PatrolPath;
		if (IsConsumed())
		{
			InitialEnemy->Destroy();
			InitialEnemy = nullptr;
			return;
		}
		ObserveEnemy(*InitialEnemy);
		InitialEnemy = nullptr;
	}
	else if (!IsObservationOnly() && !IsConsumed() && CanResetField(*GetWorld(), FieldId))
	{
		SpawnEnemy();
	}
}

void AMVFieldEnemySpawnPoint::EndPlay(const EEndPlayReason::Type Reason)
{
	StopObservingEnemy();
	if (bOwnsSpawnedEnemy && (Reason == EEndPlayReason::Destroyed || Reason == EEndPlayReason::RemovedFromWorld))
	{
		if (AMVEnemy* Enemy = ActiveEnemy.Get())
		{
			Enemy->Destroy();
		}
	}
	if (Reason == EEndPlayReason::Destroyed || Reason == EEndPlayReason::RemovedFromWorld)
	{
		ClearEnemyDrops();
	}
	Super::EndPlay(Reason);
}

bool AMVFieldEnemySpawnPoint::GatherResetPoints(
	UWorld& World, const FName Field, TArray<AMVFieldEnemySpawnPoint*>& OutPoints)
{
	TSet<FName> SpawnIds;
	TSet<const AMVEnemy*> TrackedEnemies;
	for (TActorIterator<AMVFieldEnemySpawnPoint> It(&World); It; ++It)
	{
		AMVFieldEnemySpawnPoint& Point = **It;
		if (Point.FieldId != Field || Point.IsObservationOnly() || Point.IsConsumed())
		{
			continue;
		}
		const UClass* SpawnClass = IsValid(Point.InitialEnemy) ? Point.InitialEnemy->GetClass() : Point.EnemyClass.Get();
		const AMVEnemy* Tracked = IsValid(Point.InitialEnemy) ? Point.InitialEnemy.Get() : Point.ActiveEnemy.Get();
		if (Point.SpawnId.IsNone() || SpawnIds.Contains(Point.SpawnId) || !SpawnClass
			|| SpawnClass->HasAnyClassFlags(CLASS_Abstract) || (Tracked && TrackedEnemies.Contains(Tracked)))
		{
			UE_LOG(LogTemp, Warning, TEXT("Field reset: invalid or duplicate spawn point %s in %s."), *Point.SpawnId.ToString(), *Field.ToString());
			return false;
		}
		SpawnIds.Add(Point.SpawnId);
		if (Tracked)
		{
			TrackedEnemies.Add(Tracked);
		}
		OutPoints.Add(&Point);
	}
	return true;
}

FName AMVFieldEnemySpawnPoint::ResolveDeathResetField(UWorld& World, const FName PreferredField)
{
	TSet<FName> Fields;
	for (TActorIterator<AMVFieldEnemySpawnPoint> It(&World); It; ++It)
	{
		if (!It->FieldId.IsNone() && !It->IsObservationOnly())
		{
			Fields.Add(It->FieldId);
		}
	}
	if (Fields.Contains(PreferredField))
	{
		return PreferredField;
	}
	return Fields.Num() == 1 ? *Fields.CreateConstIterator() : NAME_None;
}

bool AMVFieldEnemySpawnPoint::CanResetField(UWorld& World, const FName Field)
{
	TArray<AMVFieldEnemySpawnPoint*> Points;
	return !Field.IsNone() && GatherResetPoints(World, Field, Points);
}

bool AMVFieldEnemySpawnPoint::ResetField(UWorld& World, const FName Field, const AActor* Player)
{
	TArray<AMVFieldEnemySpawnPoint*> Points;
	if (!Field.IsNone() && !GatherResetPoints(World, Field, Points))
	{
		return false;
	}

	bool bSuccess = true;
	for (TActorIterator<AMVFieldEnemySpawnPoint> It(&World); !Field.IsNone() && It; ++It)
	{
		if (It->FieldId == Field)
		{
			bSuccess &= It->ClearEnemyDrops();
		}
	}

	for (TActorIterator<AMVProjectileBase> It(&World); It; ++It)
	{
		if ((Player && It->GetLaunchAttacker() == Player) || (!Field.IsNone() && It->GetResetFieldId() == Field))
		{
			bSuccess &= It->Destroy();
		}
	}

	for (AMVFieldEnemySpawnPoint* Point : Points)
	{
		if (!IsValid(Point) || !Point->ReplaceEnemy())
		{
			bSuccess = false;
		}
	}
	return bSuccess;
}

bool AMVFieldEnemySpawnPoint::IsConsumed() const
{
	const UMVWorldStateSubsystem* State = UMVWorldStateSubsystem::Get(this);
	return State && State->IsOneTimeSpawnConsumed(FieldId, SpawnId);
}

bool AMVFieldEnemySpawnPoint::SpawnEnemy()
{
	AMVEnemy* Enemy = GetWorld()->SpawnActorDeferred<AMVEnemy>(
		EnemyClass, EnemyTransform, nullptr, nullptr,
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding);
	if (!Enemy)
	{
		UE_LOG(LogTemp, Warning, TEXT("Field reset: could not spawn %s in %s."), *SpawnId.ToString(), *FieldId.ToString());
		return false;
	}
	Enemy->PatrolPath = PatrolPath.Get();
	ObserveEnemy(*Enemy);
	bOwnsSpawnedEnemy = true;
	Enemy->FinishSpawning(EnemyTransform);
	if (IsValid(Enemy) && !Enemy->GetController())
	{
		Enemy->SpawnDefaultController();
	}
	return IsValid(Enemy);
}

bool AMVFieldEnemySpawnPoint::ReplaceEnemy()
{
	if (AMVEnemy* Enemy = ActiveEnemy.Get(); Enemy && !Enemy->Destroy())
	{
		UE_LOG(LogTemp, Warning, TEXT("Field reset: could not remove previous enemy at %s."), *SpawnId.ToString());
		return false;
	}
	StopObservingEnemy();
	ActiveEnemy.Reset();
	return SpawnEnemy();
}

void AMVFieldEnemySpawnPoint::ObserveEnemy(AMVEnemy& Enemy)
{
	ActiveEnemy = &Enemy;
	if (!IsObservationOnly())
	{
		Enemy.SetResetFieldId(FieldId);
	}
	bRewardHandledForLife = false;
	if (EnemyDropRow.IsNull() || !Enemy.StatComponent)
	{
		return;
	}

	for (UObject* Listener : Enemy.StatComponent->OnDeathStarted.GetAllObjects())
	{
		auto* OtherPoint = Cast<AMVFieldEnemySpawnPoint>(Listener);
		if (OtherPoint && OtherPoint != this
			&& Enemy.StatComponent->OnDeathStarted.IsAlreadyBound(OtherPoint, &ThisClass::HandleEnemyDeath))
		{
			UE_LOG(LogTemp, Warning, TEXT("Enemy reward at %s: %s is already observed by %s."),
				*GetPathName(), *Enemy.GetPathName(), *OtherPoint->GetPathName());
			return;
		}
	}
	Enemy.StatComponent->OnDeathStarted.AddUniqueDynamic(this, &ThisClass::HandleEnemyDeath);
}

void AMVFieldEnemySpawnPoint::StopObservingEnemy()
{
	if (AMVEnemy* Enemy = ActiveEnemy.Get(); Enemy && Enemy->StatComponent)
	{
		Enemy->StatComponent->OnDeathStarted.RemoveDynamic(this, &ThisClass::HandleEnemyDeath);
	}
}

bool AMVFieldEnemySpawnPoint::ClearEnemyDrops()
{
	EnemyDrops.RemoveAll([](const TWeakObjectPtr<AMVItemPickup>& Drop)
	{
		AMVItemPickup* Pickup = Drop.Get();
		return !Pickup || Pickup->Destroy();
	});
	return EnemyDrops.IsEmpty();
}

void AMVFieldEnemySpawnPoint::HandleEnemyDeath(const FMVDeathContext& DeathContext)
{
	AMVEnemy* Enemy = ActiveEnemy.Get();
	UWorld* World = GetWorld();
	if (!World || !HasAuthority() || !Enemy || Enemy->IsActorBeingDestroyed()
		|| DeathContext.DeadActor != Enemy || bRewardHandledForLife)
	{
		return;
	}
	bRewardHandledForLife = true;

	const FMVEnemyDropTableRow* Row = EnemyDropRow.GetRow<FMVEnemyDropTableRow>(TEXT("Enemy death reward"));
	if (!Row)
	{
		return;
	}
	if (Row->Delivery == EMVEnemyRewardDelivery::DirectToInventory)
	{
		if (auto* Rewards = World->GetSubsystem<UMVEnemyRewardSubsystem>())
		{
			Rewards->SubmitReward(*Row, EnemyDropRow);
		}
		return;
	}
	SpawnWorldDrops(*World, *Enemy, *Row);
}

void AMVFieldEnemySpawnPoint::SpawnWorldDrops(UWorld& World, AMVEnemy& Enemy, const FMVEnemyDropTableRow& Row)
{
	const FCollisionQueryParams Query(SCENE_QUERY_STAT(MVEnemyDropGround), false, &Enemy);
	const auto FindGround = [&World, &Query](const FVector& Origin, FHitResult& Hit)
	{
		return World.LineTraceSingleByChannel(Hit, Origin + FVector(0, 0, 40),
			Origin - FVector(0, 0, 450), ECC_Visibility, Query) && Hit.ImpactNormal.Z >= 0.65;
	};

	for (int32 Index = 0; Index < Row.GuaranteedDrops.Num(); ++Index)
	{
		const FMVGuaranteedEnemyDrop& Loot = Row.GuaranteedDrops[Index];
		const double Offset = (Index - (Row.GuaranteedDrops.Num() - 1) * 0.5) * 42.0;
		const FVector Spread(0, Offset, 0);
		FHitResult Hit;
		if (!FindGround(Enemy.GetActorLocation() + Spread, Hit)
			&& !FindGround(EnemyTransform.GetLocation() + Spread, Hit))
		{
			UE_LOG(LogTemp, Warning, TEXT("Enemy drop %s: no ground near enemy or spawn point %s."),
				*Loot.ItemTag.ToString(), *GetPathName());
			continue;
		}
		if (AMVItemPickup* Pickup = AMVItemPickup::SpawnEnemyDrop(
			World, Row.PickupClass, Hit.ImpactPoint + FVector(0, 0, 1), Loot.ItemTag, Loot.Quantity))
		{
			EnemyDrops.Add(Pickup);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Enemy drop %s x%d: could not create pickup for %s."),
				*Loot.ItemTag.ToString(), Loot.Quantity, *GetPathName());
		}
	}
}
