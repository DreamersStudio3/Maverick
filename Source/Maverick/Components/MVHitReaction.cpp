// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/MVHitReaction.h"

#include "Chooser.h"
#include "ChooserFunctionLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/MVActionComponent.h"
#include "Components/MVStatComponent.h"
#include "Components/MVInputManagerComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Character/MVCharacterBase.h"
#include "Tags/MVGameplayTags.h"

// Sets default values for this component's properties
UMVHitReaction::UMVHitReaction()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...

	
}


// Called when the game starts
void UMVHitReaction::BeginPlay()
{
	Super::BeginPlay();

	// ...
	OwnerCharacter = Cast<AMVCharacterBase>(GetOwner());
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("MVHitRreaction(Construction): This Component's Owner is not AMVCharacterBase"));
	}
	StatComponent = GetOwner()->FindComponentByClass<UMVStatComponent>();
	if (!StatComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("MVHitRreaction(Construction): Owner's Statcompoennt is null"));
	}
	ActionComponent = GetOwner()->FindComponentByClass<UMVActionComponent>();
	if (!ActionComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("MVHitRreaction(Construction): Owner's ActionComponent is null"));
	}
	InputManagerComponent = GetOwner()->FindComponentByClass<UMVInputManagerComponent>();
	if(!InputManagerComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("MVHitRreaction(Construction): Owner's InputManagerComponent is null"));
	}
	else
	{
		InputManagerComponent->RegisterActionInputHandler(this, MVActionInputHandlerPriorities::HitReaction);
	}

}

void UMVHitReaction::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (OwnerCharacter)
	{
		OwnerCharacter->MovementModeChangedDelegate.RemoveDynamic(this, &UMVHitReaction::OwnerCharacterLandEvent);
	}

	if(HitSequenceTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(HitSequenceTimerHandle);
	}

	if(GetUpTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(GetUpTimerHandle);
	}
	if(InputManagerComponent)
	{
		InputManagerComponent->UnregisterActionInputHandler(this);
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
	
	
	

	// Chooser Table에 사용할 Input 생성
	FMHitReactionChooserInput ChooserInput;
	ChooserInput.HitReactionDir = CalculateHitDirection(HitData.ImpactNormal);
	ChooserInput.HitReactionType = HitData.HitReactionType;
	ChooserInput.bPoiseBreak = HitData.PoiseBreak;
	if (OwnerCharacter)
	{
		ChooserInput.bIsAir = OwnerCharacter->GetCharacterMovement()->IsFalling();
	}
	ChooserInput.bIsGroggy = StatComponent->IsGroggy();
	
	// Chooser Table의 Context 생성 -> Chooser Table에서의 Input과 Output을 꽂아주는 것
	FChooserEvaluationContext ChooserContext;
	
	FDataTableRowHandle HitReactionRowHandle;
	FDataTableRowHandle GetupRowHandle;
	ChooserContext.AddStructParam(ChooserInput);
	ChooserContext.AddStructParam(HitReactionRowHandle);
	ChooserContext.AddStructParam(GetupRowHandle);

	// Chooser Table의 EvaluateChooser를 통해 나오는 결과 -> 현재는 ChooserTable의 결과는 FallBack느낌의 DataTable이다
	// 실제로 사용할 Output은 RowHandle이 될 것이고, RowHandle이 없는 경우에는 Fallback DataTable에 있는 Fallback Montage를 Additive Play해야함
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

	// RowHandle이 없는 경우 -> 이 부분에서는 Fallback Montage로 Additive Play를 해야함
	if (HitReactionRowHandle.IsNull())
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
		ActionComponent->TryAdditiveFromRowHandle(FallbackRowHandle);
		return;
	}

	// HitReaction에 맞는 Getup RowHandle을 저장 -> 추후에 Getup Action을 Play할 때 사용
	// AdditiveCondition이 true인 경우에는 Getup RowHandle을 저장하지 않음 -> 가장 최근 HitReaction의 Getup RowHandle을 저장해야함
	if (GetupRowHandle.IsNull() == false)
	{
		SelectedGetupRowHandle = GetupRowHandle;
	}


	// AdditiveCondition은 Additive Play를 해야하는 조건을 의미함
	bool AdditiveCondition = ChooserInput.bIsAir || ChooserInput.bIsGroggy || !ChooserInput.bPoiseBreak;

	// RowHandle이 있는 경우 -> 이 부분에서는 RowHandle에 있는 Montage를 Play해야함
	// PoiseBreak이 true인 경우에는 후처리 추가	-> Action Rotate, Launch, HitSequence(KnockDown, Airborne)
	if (!AdditiveCondition)
	{
		if (OwnerCharacter)
		{
			/*if (OwnerCharacter->IsInvincible())
			{
				return;
			}*/
			OwnerCharacter->SetCharacterIsLying(false);
		}

		// 후처리
		// Action Rotate
		AdjustActionRotation(ChooserInput, Cast<AActor>(HitData.Attacker), AdditiveCondition);

		// Launch Data가 있는 경우
		ApplyHitReactionLaunch(HitData.HitDirection, HitData.HitLaunchData);

		if (HitData.HitReactionType == EMVActionHitReactionType::KnockDown || HitData.HitReactionType == EMVActionHitReactionType::Airborne)
		{
			// KnockDown과 Airborne은 현재 실행한 몽타주의 End Delegate를 가져와야하고, Character의 Movement가 바뀔때의 Event를 가져옴
			BeginHitSequence(HitData);
			OwnerCharacter->SetLyingFaceDirection(ChooserInput.HitReactionDir);
		}
	}
	
	// Hit Reaction Play (여기는 Additive일 수 도 있고 일반 Montage Play일 수도 있음)
	PlayHitReaction(HitReactionRowHandle, AdditiveCondition);
	

}

bool UMVHitReaction::TryHandleActionInput(FGameplayTag ActionInputTag, FVector2D ControllerSpaceInput, bool bHasMovementInput)
{
	// GetUp Action을 Play할 수 있는 Character의 조건
	if (!OwnerCharacter || OwnerCharacter->GetCharacterIsLying() == false || OwnerCharacter->bIsFalling == true)
	{
		return false;
	}

	// 땅에 닿은 후(EndHitSequence) 일정 시간이 지났는 지 확인 
	if(bCanGetUp == false)
	{
		return false;
	}

	if (ActionInputTag.IsValid() && !ActionInputTag.MatchesTagExact(MVGameplayTags::Action_Input_Dodge))
	{
		GetUpCharacter();
		return true;
	}

	return false;
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

bool UMVHitReaction::PlayHitReaction(const FDataTableRowHandle& HitReactionRowHandle, bool AdditiveCondition)
{
	if (!ActionComponent)
	{
		return false;
	}
	
	if (AdditiveCondition)
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

void UMVHitReaction::AdjustActionRotation(const FMHitReactionChooserInput& ChooserInput, const AActor* Attacker, bool AdditiveCondition)
{
	if (AdditiveCondition)
	{
		return;
	}

	FVector Direction = Attacker->GetActorLocation() - GetOwner()->GetActorLocation();
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

bool UMVHitReaction::ApplyHitReactionLaunch(const FVector& HitDirection, const FMVHitLaunchData& HitLaunchData)
{
	if (HitDirection.IsNearlyZero())
	{
		return false;
	}

	if (HitLaunchData.LaunchDuration < KINDA_SMALL_NUMBER &&
		HitLaunchData.LaunchDistance < KINDA_SMALL_NUMBER &&
		HitLaunchData.LaunchVerticalSpeed < KINDA_SMALL_NUMBER)
	{
		return false;
	}
	
	FVector NormalizedDirection = HitDirection.GetSafeNormal2D();
	
	const float LaunchDuration = FMath::Max(0.0f, HitLaunchData.LaunchDuration);
	const float HorizontalSpeed = LaunchDuration > KINDA_SMALL_NUMBER
		? FMath::Max(0.0f, HitLaunchData.LaunchDistance) / LaunchDuration
		: 0.0f;

	// HitDirection은 피격자 위치에서 공격자 위치를 뺀 월드 방향이다. Actor yaw가 바뀌어도 Launch 방향은 이 값 그대로 간다.
	FVector LaunchVelocity = NormalizedDirection * HorizontalSpeed;
	LaunchVelocity.Z = FMath::Max(0.0f, HitLaunchData.LaunchVerticalSpeed);

	if (!OwnerCharacter)
	{
		return false;
	}

	OwnerCharacter->LaunchCharacter(LaunchVelocity, true, false);

	return true;
}

bool UMVHitReaction::BeginHitSequence(const FMVResolvedHitData& HitData)
{
	if (!OwnerCharacter)
	{
		OwnerCharacter = Cast<AMVCharacterBase>(GetOwner());
		if (!OwnerCharacter)
		{
			UE_LOG(LogTemp, Warning, TEXT("MVHitRreaction(BeginHitSequence): This Component's Owner is Null"));
		}
	}
	if(!ActionComponent)
	{
		ActionComponent = GetOwner()->FindComponentByClass<UMVActionComponent>();
		if (!ActionComponent)
		{
			UE_LOG(LogTemp, Warning, TEXT("MVHitRreaction(BeginHitSequence): Owner's ActionComponent is Null"));
		}
	}
	if(!InputManagerComponent)
	{
		InputManagerComponent = GetOwner()->FindComponentByClass<UMVInputManagerComponent>();
		if(!InputManagerComponent)
		{
			UE_LOG(LogTemp, Warning, TEXT("MVHitRreaction(BeginHitSequence): Owner's InputManagerComponent is Null"));
		}
	}

	if(HitSequenceTimerHandle.IsValid() || GetUpTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(HitSequenceTimerHandle);
		GetWorld()->GetTimerManager().ClearTimer(GetUpTimerHandle);
		OwnerCharacter->MovementModeChangedDelegate.RemoveDynamic(this, &UMVHitReaction::OwnerCharacterLandEvent);
		OwnerCharacter->OnChangedCharacterIsLying.RemoveDynamic(this, &UMVHitReaction::UnbindCharacterMovementEvent);
	}
	// 일정 시간 이후 GetUpCharacter 할 수 있는 상태로 변경 -> 이 상태는 다시 맞을 시 비활성화 해야함
	bCanGetUp = false;

	GetWorld()->GetTimerManager().SetTimer(
		HitSequenceTimerHandle,
		FTimerDelegate::CreateLambda(
			[this, HitData]()
			{
				if(OwnerCharacter->GetCharacterMovement()->IsFalling())
				{
					// 바닥 탐지 시작 (이벤트 바인드)
					OwnerCharacter->MovementModeChangedDelegate.AddUniqueDynamic(this, &UMVHitReaction::OwnerCharacterLandEvent);
					OwnerCharacter->OnChangedCharacterIsLying.AddUniqueDynamic(this, &UMVHitReaction::UnbindCharacterMovementEvent);
					return;
				}

				// 공중에 있지 않은 경우는 바로 Land Section으로 이동
				EndHitSequence();
			}),
		0.5f,
		false);

	OwnerCharacter->SetCharacterMovementRotationActive(false, false);

	return true;
}

void UMVHitReaction::OwnerCharacterLandEvent(ACharacter* Character, EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{

	// 다시 떨어질 때는 떨어지는 Montage로 이동
	if (OwnerCharacter->GetCharacterMovement()->IsFalling())
	{
		bool JumpResult = ActionComponent->TryJumpActiveActionSection(HitSequenceFallSectionName);
		if (!JumpResult)
		{
			ActionComponent->CancelActiveAction();
		}
		if(GetUpTimerHandle.IsValid())
		{
			GetWorld()->GetTimerManager().ClearTimer(GetUpTimerHandle);
		}
		bCanGetUp = false;
		return;
	}

	EndHitSequence();
}

void UMVHitReaction::EndHitSequence()
{
	if (HitSequenceTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(HitSequenceTimerHandle);
	}

	// Jump to Land section
	if (!ActionComponent)
	{
		return;
	}
	
	// Secton Jump 실행
	bool JumpResult = ActionComponent->TryJumpActiveActionSection(HitSequenceLandSectionName);
	if (!JumpResult)
	{
		ActionComponent->CancelActiveAction();
	}
	OwnerCharacter->SetCharacterIsLying(true);

	// 일정 시간 이후 GetUpCharacter 할 수 있는 상태로 변경 -> 이 상태는 다시 맞을 시 비활성화 해야함
	GetWorld()->GetTimerManager().SetTimer(
		GetUpTimerHandle,
		FTimerDelegate::CreateLambda(
			[this]()
			{
				bCanGetUp = true;
			}),
		CanGetupDelay,
		false);
}

void UMVHitReaction::GetUpCharacter()
{
	// BlockMovementInput 해제
	OwnerCharacter->SetCharacterMovementRotationActive(true, true);

	// 다른 입력 시 Getup 실행
	ActionComponent->CancelActiveAction();
	ActionComponent->TryStartActionFromRowHandle(SelectedGetupRowHandle);

	OwnerCharacter->SetCharacterIsLying(false);
}

void UMVHitReaction::UnbindCharacterMovementEvent(bool bNewState)
{
	if(bNewState == false)
	{
		OwnerCharacter->MovementModeChangedDelegate.RemoveDynamic(this, &UMVHitReaction::OwnerCharacterLandEvent);
		OwnerCharacter->OnChangedCharacterIsLying.RemoveDynamic(this, &UMVHitReaction::UnbindCharacterMovementEvent);
	}
}

