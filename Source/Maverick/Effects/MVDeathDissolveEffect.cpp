#include "Effects/MVDeathDissolveEffect.h"

#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

DEFINE_LOG_CATEGORY_STATIC(LogMVDeathDissolveEffect, Log, All);

namespace
{
constexpr float MVDeathDissolveEffectLegacyDurationSeconds = 1.2f;
constexpr float MVDeathDissolveEffectDefaultDurationSeconds = 3.0f;
}

UWorld* UMVDeathDissolveEffect::GetWorld() const
{
	if (AActor* OwnerActor = CachedOwnerActor.Get())
	{
		return OwnerActor->GetWorld();
	}

	return GetOuter() ? GetOuter()->GetWorld() : nullptr;
}

void UMVDeathDissolveEffect::InitializeEffect(AActor* InOwnerActor)
{
	CachedOwnerActor = InOwnerActor;
	MigrateLegacyDurationIfNeeded();
}

void UMVDeathDissolveEffect::StartDeathDissolve_Implementation(AActor* TargetActor)
{
	if (TargetActor)
	{
		CachedOwnerActor = TargetActor;
	}
	MigrateLegacyDurationIfNeeded();

	if (!bEnableDeathDissolve || DeathDissolveAmountParameterName.IsNone())
	{
		return;
	}

	ResetDeathDissolveVisuals();

	AActor* OwnerActor = TargetActor ? TargetActor : CachedOwnerActor.Get();
	if (!OwnerActor)
	{
		return;
	}

	TArray<USkeletalMeshComponent*> MeshComponents;
	OwnerActor->GetComponents<USkeletalMeshComponent>(MeshComponents);
	for (USkeletalMeshComponent* MeshComponent : MeshComponents)
	{
		if (!IsValid(MeshComponent))
		{
			continue;
		}

		const int32 MaterialCount = MeshComponent->GetNumMaterials();
		if (MaterialCount <= 0)
		{
			continue;
		}

		FMVDeathDissolveMeshState MeshState;
		MeshState.MeshComponent = MeshComponent;
		MeshState.bWasHiddenInGame = MeshComponent->bHiddenInGame;
		MeshState.OriginalMaterials.Reserve(MaterialCount);
		MeshState.DynamicMaterials.Reserve(MaterialCount);

		MeshComponent->SetHiddenInGame(false, true);
		for (int32 MaterialIndex = 0; MaterialIndex < MaterialCount; ++MaterialIndex)
		{
			UMaterialInterface* OriginalMaterial = MeshComponent->GetMaterial(MaterialIndex);
			MeshState.OriginalMaterials.Add(OriginalMaterial);

			UMaterialInstanceDynamic* DynamicMaterial = OriginalMaterial
				? MeshComponent->CreateDynamicMaterialInstance(MaterialIndex, OriginalMaterial)
				: nullptr;
			if (DynamicMaterial)
			{
				DynamicMaterial->SetScalarParameterValue(DeathDissolveAmountParameterName, 0.0f);
			}

			MeshState.DynamicMaterials.Add(DynamicMaterial);
		}

		DeathDissolveMeshStates.Add(MoveTemp(MeshState));
	}

	if (DeathDissolveMeshStates.IsEmpty())
	{
		return;
	}

	ApplyDeathDissolveAmount(0.0f);

	const float ClampedDuration = FMath::Max(0.0f, DeathDissolveDuration);
	if (ClampedDuration <= KINDA_SMALL_NUMBER)
	{
		FinishDeathDissolve();
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		FinishDeathDissolve();
		return;
	}

	DeathDissolveStartTimeSeconds = World->GetTimeSeconds();
	World->GetTimerManager().SetTimer(
		DeathDissolveTimerHandle,
		this,
		&UMVDeathDissolveEffect::UpdateDeathDissolve,
		FMath::Max(0.001f, DeathDissolveUpdateInterval),
		true);
	UpdateDeathDissolve();
}

void UMVDeathDissolveEffect::ResetDeathDissolveVisuals_Implementation()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DeathDissolveTimerHandle);
	}

	for (const FMVDeathDissolveMeshState& MeshState : DeathDissolveMeshStates)
	{
		USkeletalMeshComponent* MeshComponent = MeshState.MeshComponent.Get();
		if (!MeshComponent)
		{
			continue;
		}

		const int32 RestoreMaterialCount = FMath::Min(MeshComponent->GetNumMaterials(), MeshState.OriginalMaterials.Num());
		for (int32 MaterialIndex = 0; MaterialIndex < RestoreMaterialCount; ++MaterialIndex)
		{
			MeshComponent->SetMaterial(MaterialIndex, MeshState.OriginalMaterials[MaterialIndex]);
		}
		MeshComponent->SetHiddenInGame(MeshState.bWasHiddenInGame, true);
	}

	DeathDissolveMeshStates.Reset();
	DeathDissolveStartTimeSeconds = 0.0f;
}

void UMVDeathDissolveEffect::ApplyDeathDissolveAmount(const float Amount)
{
	const float ClampedAmount = FMath::Clamp(Amount, 0.0f, 1.0f);
	for (const FMVDeathDissolveMeshState& MeshState : DeathDissolveMeshStates)
	{
		for (UMaterialInstanceDynamic* DynamicMaterial : MeshState.DynamicMaterials)
		{
			if (DynamicMaterial)
			{
				DynamicMaterial->SetScalarParameterValue(DeathDissolveAmountParameterName, ClampedAmount);
			}
		}
	}
}

void UMVDeathDissolveEffect::MigrateLegacyDurationIfNeeded()
{
	if (bLegacyDurationMigrationChecked)
	{
		return;
	}

	bLegacyDurationMigrationChecked = true;
	if (FMath::IsNearlyEqual(DeathDissolveDuration, MVDeathDissolveEffectLegacyDurationSeconds, 0.001f))
	{

		DeathDissolveDuration = MVDeathDissolveEffectDefaultDurationSeconds;
	}
}

void UMVDeathDissolveEffect::UpdateDeathDissolve()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		FinishDeathDissolve();
		return;
	}

	const float ClampedDuration = FMath::Max(0.0f, DeathDissolveDuration);
	if (ClampedDuration <= KINDA_SMALL_NUMBER)
	{
		FinishDeathDissolve();
		return;
	}

	const float ElapsedSeconds = World->GetTimeSeconds() - DeathDissolveStartTimeSeconds;
	const float DissolveAlpha = FMath::Clamp(ElapsedSeconds / ClampedDuration, 0.0f, 1.0f);
	const float TargetMaterialAmount = FMath::Clamp(DeathDissolveMaterialCompleteAmount, 0.0f, 1.0f);
	ApplyDeathDissolveAmount(DissolveAlpha * TargetMaterialAmount);

	if (DissolveAlpha >= 1.0f)
	{
		FinishDeathDissolve();
	}
}

void UMVDeathDissolveEffect::FinishDeathDissolve()
{
	const float ElapsedSeconds = GetWorld()
		? GetWorld()->GetTimeSeconds() - DeathDissolveStartTimeSeconds
		: 0.0f;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DeathDissolveTimerHandle);
	}

	ApplyDeathDissolveAmount(bHideMeshAfterDeathDissolve
		? 1.0f
		: FMath::Clamp(DeathDissolveMaterialCompleteAmount, 0.0f, 1.0f));
	if (bHideMeshAfterDeathDissolve)
	{
		for (const FMVDeathDissolveMeshState& MeshState : DeathDissolveMeshStates)
		{
			if (USkeletalMeshComponent* MeshComponent = MeshState.MeshComponent.Get())
			{
				MeshComponent->SetHiddenInGame(true, true);
			}
		}
	}


}
