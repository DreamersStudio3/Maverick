#include "MVAnimNotify_AttackNotice.h"

#include "Character/NPC/Enemy/MVEnemy.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "Struct/MVAIDodgeTypes.h"

DEFINE_LOG_CATEGORY_STATIC(LogMVAttackNotice, Log, All);

void UMVAnimNotify_AttackNotice::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	///UE_LOG(LogTemp, Log, TEXT("Attack Notice Play"));

	Super::Notify(MeshComp, Animation, EventReference);

	AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;
	UWorld* World = Owner ? Owner->GetWorld() : nullptr;
	if (!World)
	{
		return;
	}

	const FQuat Rotation = Owner->GetActorQuat();
	const FVector Center = Owner->GetActorLocation()
		+ Owner->GetActorForwardVector() * FMath::Max(0.0f, ForwardDistance);
	const FVector HalfExtent = BoxSize.ComponentMax(FVector(1.0f)) * 0.5f;

	if (bDrawDebug)
	{
		DrawDebugBox(World, Center, HalfExtent, Rotation, FColor::Yellow,
			false, FMath::Max(0.0f, DebugDuration), 0, 2.0f);
	}

	if (!World->IsGameWorld())
	{
		return;
	}

	TArray<FOverlapResult> Overlaps;
	const FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(MVAttackNotice), false, Owner);
	World->OverlapMultiByObjectType(Overlaps, Center, Rotation,
		FCollisionObjectQueryParams::AllDynamicObjects,
		FCollisionShape::MakeBox(HalfExtent), QueryParams);

	TSet<AMVEnemy*> NotifiedEnemies;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AMVEnemy* Enemy = Cast<AMVEnemy>(Overlap.GetActor());
		if (!IsValid(Enemy) || NotifiedEnemies.Contains(Enemy))
		{
			continue;
		}

		NotifiedEnemies.Add(Enemy);
		FMVAIDodgeRequest Notice;
		Notice.ThreatActor = Owner;
		Notice.ThreatLocation = Owner->GetActorLocation();
		const FVector ToThreat = Notice.ThreatLocation - Enemy->GetActorLocation();
		Notice.DistanceToThreat = ToThreat.Size();
		const FVector LocalThreat = Enemy->GetActorQuat().UnrotateVector(ToThreat);
		Notice.AngleToThreat = FMath::RadiansToDegrees(FMath::Atan2(LocalThreat.Y, LocalThreat.X));

		const bool bSent = Enemy->ReceiveAttackNotice(Notice);
		UE_LOG(LogMVAttackNotice, Log, TEXT("[AttackNotice] Owner=%s Enemy=%s EventSent=%d"),
			*Owner->GetName(), *Enemy->GetName(), bSent);
	}
}
