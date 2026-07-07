// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/MVFinisherComponent.h"

#include "Kismet/kismetSystemLibrary.h"
#include "Components/MVStatComponent.h"

// Sets default values for this component's properties
UMVFinisherComponent::UMVFinisherComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UMVFinisherComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UMVFinisherComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

bool UMVFinisherComponent::TryFinisherMove()
{
	AActor* HitActor = nullptr;

	if(!CanFinisherMove(HitActor))
	{
		return false;
	}

	// Find Animation

	// Set Warp Target

	// Play Animation


	return true;
}

bool UMVFinisherComponent::CanFinisherMove(AActor* OutHitActor) const
{
	if (!MakeSphereTrace(OutHitActor))
	{
		return false;
	}
	
	// Check if the actor is groggy
	if(!CheckThisActorGroggy(OutHitActor))
	{
		return false;
	}
	
	// Check if the actor is within distance and direction
	
	if(!JudgeDistanceAndDirection(OutHitActor))
	{
		return false;
	}

	return true;
}

bool UMVFinisherComponent::MakeSphereTrace(AActor* OutHitActor) const
{
	FVector StartLocation = GetOwner()->GetActorLocation();
	FVector EndLocation = StartLocation + GetOwner()->GetActorForwardVector() * 200.0f; // Check 100 units ahead

	FHitResult HitResult;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(GetOwner());

	bool bHit = UKismetSystemLibrary::SphereTraceSingle(
		this,
		StartLocation,
		EndLocation,
		50.0f,													// Sphere radius
		UEngineTypes::ConvertToTraceType(ECC_Visibility),		// Todo: Trace channel -> Currenytly using Visibility channel, should be changed to a custom channel
		false,
		ActorsToIgnore,
		EDrawDebugTrace::ForDuration,
		HitResult,
		true,
		FLinearColor::Red,
		FLinearColor::Green,
		1.0f
	);

	if (!bHit)
	{
		return false;
	}

	OutHitActor = HitResult.GetActor();
	return true;
}

bool UMVFinisherComponent::JudgeDistanceAndDirection(const AActor* HitActor) const
{
	if (!HitActor)
	{
		return false;
	}
	
	float TargetDistance = 100.0f;
	float TargetDistanceSquared = TargetDistance * TargetDistance;
	float TargetMaxAngle = 35.0f;

	float Distance = FVector::DistSquared2D(GetOwner()->GetActorLocation(), HitActor->GetActorLocation());
	if(Distance> TargetDistanceSquared)
	{
		return false;
	}

	FVector	DirToOwner = (GetOwner()->GetActorLocation() - HitActor->GetActorLocation()).GetSafeNormal2D(0.001);
	FVector TargetForward = HitActor->GetActorForwardVector();
	
	float DotProduct = FMath::Clamp(FVector::DotProduct(DirToOwner, TargetForward), -1.0, 1.0);
	float Angle = FMath::Acos(DotProduct) * (180.0f / PI);

	if(FMath::Abs(Angle) > TargetMaxAngle)
	{
		return false;
	}

	return true;
}

bool UMVFinisherComponent::CheckThisActorGroggy(const AActor* HitActor) const
{
	if(!HitActor)
	{
		return false;
	}
	
	
	UMVStatComponent* StatComponent = HitActor->FindComponentByClass<UMVStatComponent>();
	if(!StatComponent)
	{
		return false;
	}

	if(!StatComponent->IsGroggy())
	{
		return false;
	}

	return true;
}

