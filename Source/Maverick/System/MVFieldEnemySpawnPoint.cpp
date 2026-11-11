#include "System/MVFieldEnemySpawnPoint.h"

#include "Character/NPC/Enemy/MVEnemy.h"
#include "Cinematics/Boss/MVBossIntroTrigger.h"
#include "Combat/Projectile/MVProjectileBase.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "System/MVWorldStateSubsystem.h"

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
				return;
			}
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
		ActiveEnemy = InitialEnemy;
		InitialEnemy->SetResetFieldId(FieldId);
		InitialEnemy = nullptr;
	}
	else if (!IsConsumed() && CanResetField(*GetWorld(), FieldId))
	{
		SpawnEnemy();
	}
}

void AMVFieldEnemySpawnPoint::EndPlay(const EEndPlayReason::Type Reason)
{
	if (bOwnsSpawnedEnemy && (Reason == EEndPlayReason::Destroyed || Reason == EEndPlayReason::RemovedFromWorld))
	{
		if (AMVEnemy* Enemy = ActiveEnemy.Get())
		{
			Enemy->Destroy();
		}
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
		if (Point.FieldId != Field || Point.bBoss || Point.IsConsumed())
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
		if (!It->FieldId.IsNone() && !It->bBoss)
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
	Enemy->SetResetFieldId(FieldId);
	ActiveEnemy = Enemy;
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
	ActiveEnemy.Reset();
	return SpawnEnemy();
}
