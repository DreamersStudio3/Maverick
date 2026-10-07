#include "Combat/Duelist/MVDuelistChainPullAbility.h"

#include "AI/Controller/MVAIController.h"
#include "Character/MVCharacterBase.h"
#include "Combat/Duelist/MVDuelistChainProjectile.h"
#include "Components/MVStatComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

void UMVDuelistChainPullAbility::StartAbility_Implementation(int32 AbilityIndex)
{
	if (bAbilityActive) return;
	AMVCharacterBase* Character = GetOwnerCharacter();
	const AMVAIController* Controller = Character ? Cast<AMVAIController>(Character->GetController()) : nullptr;
	Target = Cast<AMVCharacterBase>(Controller ? Controller->TargetActor : nullptr);
	if (!Target.IsValid() && Character)
	{
		Target = Cast<AMVCharacterBase>(UGameplayStatics::GetPlayerCharacter(Character, 0));
	}
	if (!IsValid(Character) || !Target.IsValid() || !Target->IsPlayerControlled()
		|| (Target->StatComponent && Target->StatComponent->IsDead())) return;

	Super::StartAbility_Implementation(AbilityIndex);
	if (!bAbilityActive) return;
	bFired = false;
	const FVector Direction = (Target->GetActorLocation() - Character->GetActorLocation()).GetSafeNormal2D();
	if (!Direction.IsNearlyZero()) Character->SetActorRotation(Direction.Rotation());
}

void UMVDuelistChainPullAbility::FireChain()
{
	AMVCharacterBase* Character = GetOwnerCharacter();
	if (!bAbilityActive || bFired || !IsValid(Character) || !Target.IsValid()
		|| (Character->StatComponent && Character->StatComponent->IsDead())
		|| (Target->StatComponent && Target->StatComponent->IsDead())) return;
	bFired = true;
	USkeletalMeshComponent* Mesh = Character->GetMesh();
	const FVector Start = Mesh && Mesh->DoesSocketExist(LaunchSocket)
		? Mesh->GetSocketLocation(LaunchSocket)
		: Character->GetActorLocation() + Character->GetActorForwardVector() * 60.0f;
	const FVector Offset = Target->GetActorLocation() - Start;
	if (Offset.SizeSquared() > FMath::Square(MaxRange) || Offset.IsNearlyZero()) return;
	FActorSpawnParameters Spawn;
	Spawn.Owner = Character;
	Spawn.Instigator = Character;
	Spawn.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AMVDuelistChainProjectile* Chain = Character->GetWorld()->SpawnActor<AMVDuelistChainProjectile>(
		Start, Offset.Rotation(), Spawn);
	Projectile = Chain;
	if (Chain) Chain->Initialize(this, Offset.GetSafeNormal());
}

void UMVDuelistChainPullAbility::ClearProjectile()
{
	if (AMVDuelistChainProjectile* Chain = Projectile.Get()) Chain->Destroy();
	Projectile.Reset();
	Target.Reset();
}

void UMVDuelistChainPullAbility::EndAbility_Implementation()
{
	ClearProjectile();
	Super::EndAbility_Implementation();
}

void UMVDuelistChainPullAbility::BeginDestroy()
{
	ClearProjectile();
	Super::BeginDestroy();
}
