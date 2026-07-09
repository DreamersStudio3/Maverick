// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/MVFinisherComponent.h"

#include "Kismet/kismetSystemLibrary.h"
#include "Components/MVStatComponent.h"
#include "Public/Tables/MVSkillDataTableColumn.h"
#include "Chooser.h"
#include "ChooserFunctionLibrary.h"
#include "Character/MVCharacterBase.h"
#include "Components/MVWeaponComponent.h"
#include "MotionWarpingComponent.h"
#include "Components/MVActionComponent.h"
#include "Combat/MVAbilityBase.h"
#include "Tags/MVGameplayTags.h"
#include "Components/MVInputManagerComponent.h"

// Sets default values for this component's properties
UMVFinisherComponent::UMVFinisherComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UMVFinisherComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
	AMVCharacterBase* OwnerCharacter = Cast<AMVCharacterBase>(GetOwner());
	if (!OwnerCharacter)
	{
		return;
	}

	if (UMVInputManagerComponent* InputManager = OwnerCharacter->InputManagerComponent)
	{
		InputManager->RegisterActionInputHandler(this, MVActionInputHandlerPriorities::Finisher);
	}

}

void UMVFinisherComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (const AMVCharacterBase* OwnerCharacter = Cast<AMVCharacterBase>(GetOwner()))
	{
		if (UMVInputManagerComponent* InputManager = OwnerCharacter->InputManagerComponent)
		{
			InputManager->UnregisterActionInputHandler(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}


// Called every frame
void UMVFinisherComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

bool UMVFinisherComponent::TryHandleActionInput(FGameplayTag ActionInputTag, FVector2D ControllerSpaceInput, bool bHasMovementInput)
{
	if (ActionInputTag.MatchesTagExact(MVGameplayTags::Action_Input_LightAttack))
	{
		return TryFinisherMove();
	}

	return false;
}

bool UMVFinisherComponent::TryFinisherMove()
{
	AActor* HitActor = nullptr;

	// Detect HitActor and Judge that Attacker's location is valid for Finisher action
	if(!CanFinisherMove(HitActor))
	{
		return false;
	}

	ClearAttackData();

	// Find Animation
	FDataTableRowHandle AttackerRowHandle;
	FDataTableRowHandle VictimRowHandle;
	if (!FindFinisherAnimation(HitActor, FName("FallBack"), AttackerRowHandle, VictimRowHandle))
	{
		return false;
	}

	// Set Warp Target
	if(!SetWarpTarget(HitActor))
	{
		return false;
	}

	// Play Animation
	if (!SendAnimation(HitActor,AttackerRowHandle,VictimRowHandle))
	{
		return false;
	}

	SetAttackData(AttackerRowHandle);

	// Store Target Actor
	TargetActor = HitActor;

	ResetTargetGroggy();

	return true;
}

AActor* UMVFinisherComponent::GetTargetActor_Implementation()
{
	if (TargetActor.IsValid())
	{
		return TargetActor.Get();
	}
	return nullptr;
}

bool UMVFinisherComponent::CanFinisherMove(AActor*& OutHitActor) const
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

bool UMVFinisherComponent::MakeSphereTrace(AActor*& OutHitActor) const
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
		UEngineTypes::ConvertToTraceType(ECC_Camera),		// Todo: Trace channel -> Currenytly using Visibility channel, should be changed to a custom channel
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
	
	float TargetDistance = 200.0f;
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

bool UMVFinisherComponent::FindFinisherAnimation(
	AActor* HitActor, 
	FName FallBackRowName, 
	FDataTableRowHandle& OutAttacker, 
	FDataTableRowHandle& OutVictim)
{
	OutAttacker = FDataTableRowHandle();
	OutVictim = FDataTableRowHandle();

	// Check if the Attacker and Victim ChooserTable is valid
	if (!AttackerChooserTable.IsValid() || !VictimChooserTable.IsValid())
	{
		return false;
	}
	UChooserTable* AttackerChooser = Cast<UChooserTable>(AttackerChooserTable.TryLoad());
	if (!AttackerChooser)
	{
		return false;
	}
	UChooserTable* VictimChooser = Cast<UChooserTable>(VictimChooserTable.TryLoad());
	if (!VictimChooser)
	{
		return false;
	}

	// If Owner and Target are not CharacterBase, return false
	AMVCharacterBase* OwnerCharacter = Cast<AMVCharacterBase>(GetOwner());
	AMVCharacterBase* TargetCharacter = Cast<AMVCharacterBase>(HitActor);
	if (!OwnerCharacter || !TargetCharacter)
	{
		return false;
	}

	//====================== Evaluate Attacker Chooser ======================//

	// Input for the AttackerChooser 
	FMVFinisherChooserInput Input;
	Input.WeaponTag.AddTag(OwnerCharacter->WeaponComponent->GetEquippedWeaponState().ItemTag);
	Input.AttackerTag.AddTag(OwnerCharacter->CharacterIndexCode);
	Input.VictimTag.AddTag(TargetCharacter->CharacterIndexCode);

	// Output for the chooser
	FMVFinisherChooserOutput Output;
	Output.Reset();

	// Adding Context for the Chooser
	FChooserEvaluationContext ChooserContext;
	ChooserContext.AddStructParam(Input);
	ChooserContext.AddStructParam(Output);

	TSoftObjectPtr<UObject> SelectedObject;
	UChooserTable::EvaluateChooser(
		ChooserContext,
		AttackerChooser,
		FObjectChooserBase::FObjectChooserSoftObjectIteratorCallback::CreateLambda(
			[&SelectedObject](const TSoftObjectPtr<UObject>& InResult)
			{
				SelectedObject = InResult;
				return FObjectChooserBase::EIteratorStatus::Stop;
			}));
	// If Succeeded to find Attacker Row
	if (Output.ActionRow.DataTable)
	{
		OutAttacker = Output.ActionRow;
		if (OutAttacker.RowName.IsNone())
		{
			OutAttacker.RowName = FallBackRowName;
		}
	}
	// If Failed to find Attacker Row
	else
	{
		UObject* ResolvedObject = SelectedObject.IsValid()
			? SelectedObject.Get()
			: SelectedObject.LoadSynchronous();
		UDataTable* SelectedDataTable = Cast<UDataTable>(ResolvedObject);
		if (!SelectedDataTable)
		{
			return false;
		}

		OutAttacker.DataTable = SelectedDataTable;
		OutAttacker.RowName = FallBackRowName;
	}

	//====================== Evaluate Victim Chooser ======================//

	// Input for the VictimChooser --> Same as AttackerChooser

	// Output for the Chooser
	Output.Reset();

	// Adding Context for the Victim Chooser
	ChooserContext = FChooserEvaluationContext();
	ChooserContext.AddStructParam(Input);
	ChooserContext.AddStructParam(Output);

	SelectedObject.Reset();
	UChooserTable::EvaluateChooser(
		ChooserContext,
		VictimChooser,
		FObjectChooserBase::FObjectChooserSoftObjectIteratorCallback::CreateLambda(
			[&SelectedObject](const TSoftObjectPtr<UObject>& InResult)
			{
				SelectedObject = InResult;
				return FObjectChooserBase::EIteratorStatus::Stop;
			}));
	// If Succeeded to find Attacker Row
	if (Output.ActionRow.DataTable)
	{
		OutVictim = Output.ActionRow;
		if (OutVictim.RowName.IsNone())
		{
			OutVictim.RowName = FallBackRowName;
		}
	}
	// If Failed to find Attacker Row
	else
	{
		UObject* ResolvedObject = SelectedObject.IsValid()
			? SelectedObject.Get()
			: SelectedObject.LoadSynchronous();
		UDataTable* SelectedDataTable = Cast<UDataTable>(ResolvedObject);
		if (!SelectedDataTable)
		{
			return false;
		}

		OutVictim.DataTable = SelectedDataTable;
		OutVictim.RowName = FallBackRowName;
	}

	return true;

}

bool UMVFinisherComponent::SetWarpTarget(const AActor* HitActor)
{
	if (!HitActor)
	{
		return false;
	}
	
	AMVCharacterBase* OwnerCharacter = Cast<AMVCharacterBase>(GetOwner());
	if (!OwnerCharacter)
	{
		return false;
	}

	FVector TargetLocation;
	FRotator TargetRotation;
	float Distance = 130.0f;

	TargetLocation = HitActor->GetActorLocation() + (HitActor->GetActorForwardVector() * Distance);
	TargetRotation = HitActor->GetActorRotation();
	TargetRotation.Yaw = FRotator::NormalizeAxis(TargetRotation.Yaw + 180.0f);

	OwnerCharacter->MotionWarpingComponent->AddOrUpdateWarpTargetFromLocationAndRotation(
		FName(TEXT("Attacker")), 
		TargetLocation, 
		TargetRotation
	);

	return true;
}

bool UMVFinisherComponent::SendAnimation(AActor* HitActor, const FDataTableRowHandle& AttackerRowHandle, const FDataTableRowHandle& VictimRowHandle)
{
	// Attacker Action Play
	UMVActionComponent* ActionComp = GetOwner()->FindComponentByClass<UMVActionComponent>();
	if (!ActionComp)
	{
		return false;
	}
	// Attacker's Action Can Play even if HitActor doesn't have ActionComponent(But returns false)
	ActionComp->TryTransitionActionFromRowHandle(AttackerRowHandle);

	// Victim Action Play
	ActionComp = HitActor->FindComponentByClass<UMVActionComponent>();
	if (!ActionComp)
	{
		return false;
	}
	ActionComp->TryTransitionActionFromRowHandle(VictimRowHandle);

	return true;
}

void UMVFinisherComponent::SetAttackData(const FDataTableRowHandle& AttackDataRowHandle)
{
	AttackData = AttackDataRowHandle;

	const UDataTable* DataTable = AttackData.DataTable;
	const FName RowName = AttackData.RowName;

	FMVSkillDataTableColumn* RowData =
		AttackDataRowHandle.DataTable->FindRow<FMVSkillDataTableColumn>(RowName, TEXT("BuildFinisherAttackData"));
	if (!RowData || !RowData->AbilityReference)
	{
		return;
	}

	// Create ability instance
	AbilityInstance = NewObject<UMVAbilityBase>(this, RowData->AbilityReference);
	if (AbilityInstance)
	{
		AbilityInstance->SetOwner(this);
		AbilityInstance->InitAbility(*RowData);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MVFinisherComponent::SetAttackData - Failed to create ability '%s'"), *RowName.ToString());
		return;
	}


	return;
}

void UMVFinisherComponent::ClearAttackData()
{
	AttackData = FDataTableRowHandle();
	if (AbilityInstance)
	{
		AbilityInstance->MarkAsGarbage();
	}
	AbilityInstance = nullptr;
	TargetActor = nullptr;
}

void UMVFinisherComponent::ResetTargetGroggy()
{
	if (!TargetActor.Get())
	{
		return;
	}
	
	AMVCharacterBase* TargetCharacter = Cast<AMVCharacterBase>(TargetActor);
	if (!TargetCharacter)
	{
		return;
	}

	UMVStatComponent* TargetStatComp = TargetCharacter->FindComponentByClass<UMVStatComponent>();
	if (!TargetStatComp)
	{
		return;
	}

	TargetStatComp->SetCurrentGroggy(0);

}

