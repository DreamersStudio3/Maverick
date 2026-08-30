// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/MVHitReaction.h"

#include "Chooser.h"
#include "ChooserFunctionLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/MVActionComponent.h"
#include "Components/MVStatComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Character/MVCharacterBase.h"

// Sets default values for this component's properties
UMVHitReaction::UMVHitReaction()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...

	
}


// Called when the game starts
void UMVHitReaction::BeginPlay()
{
	Super::BeginPlay();

	// ...
	OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("MVHitRreaction(Construction): This Component's Owner is not ACharacter"));
	}
	StatComponent = GetOwner()->FindComponentByClass<UMVStatComponent>();
	if (!StatComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("MVHitRreaction(Construction): Owner's Statcompoennt is null"));
	}
	ActionComponent = GetOwner()->FindComponentByClass<UMVActionComponent>();
	{
		UE_LOG(LogTemp, Warning, TEXT("MVHitRreaction(Construction): Owner's ActionComponent is null"));
	}
	

}


// Called every frame
void UMVHitReaction::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UMVHitReaction::HandleHitEvent(const FMVResolvedHitData& HitData)
{
	if (!IsValid(HitReactionChooser))
	{
		return;
	}
	
	if (HitData.HitReactionType == EMVActionHitReactionType::None)
	{
		return;
	}

	// 죽었을 때 그냥 끝
	if (!StatComponent || StatComponent->IsDead())
	{
		return;
	}
	
	FMHitReactionChooserInput ChooserInput;
	ChooserInput.HitReactionDir = CalculateHitDirection(HitData.ImpactNormal);
	ChooserInput.HitReactionType = HitData.HitReactionType;
	ChooserInput.bPoiseBreak = HitData.PoiseBreak;
	if (OwnerCharacter)
	{
		ChooserInput.bIsAir = OwnerCharacter->GetCharacterMovement()->IsFalling();
	}
	ChooserInput.bIsGroggy = StatComponent->IsGroggy();
	
	FChooserEvaluationContext ChooserContext;
	
	FDataTableRowHandle RowHandle;
	ChooserContext.AddStructParam(ChooserInput);
	ChooserContext.AddStructParam(RowHandle);

	TSoftObjectPtr<UObject> SelectedObject;
	UChooserTable::EvaluateChooser(
		ChooserContext,
		HitReactionChooser,
		FObjectChooserBase::FObjectChooserSoftObjectIteratorCallback::CreateLambda
		(
			[&SelectedObject](const TSoftObjectPtr<UObject>& InResult)
			{
				SelectedObject = InResult;
				return FObjectChooserBase::EIteratorStatus::Stop;
			}
		)
	);

	// RowHandle이 없거나, HitReaction 실행을 실패한 경우
	if (RowHandle.IsNull() || !PlayHitReaction(RowHandle, ChooserInput))
	{
		// Fallback Montage play
		UObject* ResolvedObject = SelectedObject.IsValid()
			? SelectedObject.Get()
			: SelectedObject.LoadSynchronous();
		UDataTable* SelectedDataTable = Cast<UDataTable>(ResolvedObject);
		if (!SelectedDataTable)
		{
			return;
		}
		FDataTableRowHandle FallbackRowHandle;
		FallbackRowHandle.DataTable = SelectedDataTable;
		FallbackRowHandle.RowName = FName(TEXT("Fallback"));
		ActionComponent->TryStartActionFromRowHandle(FallbackRowHandle);
		return;
	}

	// 후처리
	// Action Rotate
	AdjustActionRotation(ChooserInput, Cast<AActor>(HitData.Attacker));

	// KnockBack || Airborne

}

EMVHitReactionDir UMVHitReaction::CalculateHitDirection(const FVector& ImpactNormal)
{
	FVector IncomingDir = FVector(-ImpactNormal.X, -ImpactNormal.Y, 0);
	AActor* OwnerActor = GetOwner();

	float ForwardDot = FVector::DotProduct(OwnerActor->GetActorForwardVector(), IncomingDir);
	float RightDot = FVector::DotProduct(OwnerActor->GetActorRightVector(), IncomingDir);

	if (ForwardDot > 0.7f)
	{
		return EMVHitReactionDir::Front;
	}
	else if (ForwardDot < -0.7f)
	{
		return EMVHitReactionDir::Back;
	}
	else if (RightDot > 0.f)
	{
		return EMVHitReactionDir::Right;
	}
	else
	{
		return EMVHitReactionDir::Left;
	}
	
}

bool UMVHitReaction::PlayHitReaction(const FDataTableRowHandle& HitReactionRowHandle, const FMHitReactionChooserInput& ChooserInput)
{
	if (!ActionComponent)
	{
		return false;
	}
	
	if (ChooserInput.bIsAir || ChooserInput.bIsGroggy || !ChooserInput.bPoiseBreak)
	{
		ActionComponent->TryAdditiveFromRowHandle(HitReactionRowHandle);
	}
	else
	{
		ActionComponent->CancelActiveAction();
		ActionComponent->TryStartActionFromRowHandle(HitReactionRowHandle);
	}

	return true;
}

void UMVHitReaction::AdjustActionRotation(const FMHitReactionChooserInput& ChooserInput, const AActor* Attacker)
{
	if (ChooserInput.bIsAir || ChooserInput.bIsGroggy || !ChooserInput.bPoiseBreak)
	{
		return;
	}

	// Todo: 테스트 후 공격자로 변경
	FVector Direction = /*Attacker->GetActorLocation()*/ FVector(450, 0, 92) -GetOwner()->GetActorLocation();
	if (Direction.IsNearlyZero())
	{
		return;
	}
	
	FRotator NewRotation = UKismetMathLibrary::MakeRotFromX(Direction);
	float YawOffset = 0.0f;
	if (ChooserInput.HitReactionDir == EMVHitReactionDir::Right)
	{
		YawOffset = 90.0f;
	}
	else if (ChooserInput.HitReactionDir == EMVHitReactionDir::Front)
	{
		YawOffset = 180.0f;
	}
	else if (ChooserInput.HitReactionDir == EMVHitReactionDir::Left)
	{
		YawOffset = 270.0f;
	}
	else
	{
		YawOffset = 0.0f;
	}
	NewRotation.Yaw = NewRotation.Yaw + YawOffset;

	GetOwner()->SetActorRotation(NewRotation);

}

