#include "UI/BossHUD/MVBossAttackTestWidget.h"

#include "AI/Controller/MVAIController.h"
#include "Character/NPC/Enemy/MVEnemy.h"
#include "Combat/Duelist/MVDuelistResonanceComponent.h"
#include "Components/Button.h"
#include "Components/MVActionComponent.h"
#include "Components/MVCombatComponent.h"
#include "Components/MVStatComponent.h"
#include "Components/StateTreeAIComponent.h"
#include "Engine/GameViewportClient.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "UObject/ConstructorHelpers.h"

UMVBossAttackTestWidget::UMVBossAttackTestWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	static ConstructorHelpers::FObjectFinder<UDataTable> Table(TEXT("/Game/DuelistComplete/DT_DuelistBossCombatAttack.DT_DuelistBossCombatAttack"));
	AttackTable = Table.Object;
}

void UMVBossAttackTestWidget::NativeConstruct()
{
	Super::NativeConstruct();
	BaseAttackButton->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleBaseAttackClicked);
	QAttackButton->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleQAttackClicked);
	WAttackButton->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleWAttackClicked);
	PazeChangeAttackButton->OnClicked.AddUniqueDynamic(this, &ThisClass::HandlePazeChangeAttackClicked);
	for (UButton* Button : {BaseAttackButton.Get(), QAttackButton.Get(), WAttackButton.Get(), PazeChangeAttackButton.Get()})
	{
		Button->IsFocusable = false;
	}
	if (APlayerController* Player = GetOwningPlayer())
	{
		bWasMouseCursorVisible = Player->bShowMouseCursor;
	}
	ResolveBoss();
	EnsureMouseInput();
	RefreshButtons();
}

void UMVBossAttackTestWidget::NativeDestruct()
{
	BaseAttackButton->OnClicked.RemoveDynamic(this, &ThisClass::HandleBaseAttackClicked);
	QAttackButton->OnClicked.RemoveDynamic(this, &ThisClass::HandleQAttackClicked);
	WAttackButton->OnClicked.RemoveDynamic(this, &ThisClass::HandleWAttackClicked);
	PazeChangeAttackButton->OnClicked.RemoveDynamic(this, &ThisClass::HandlePazeChangeAttackClicked);
	ReleaseManualControl();
	if (APlayerController* Player = GetOwningPlayer())
	{
		Player->SetShowMouseCursor(bWasMouseCursorVisible);
		if (!bWasMouseCursorVisible) Player->SetInputMode(FInputModeGameOnly());
	}
	Super::NativeDestruct();
}

void UMVBossAttackTestWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	ResolveBoss();
	EnsureMouseInput();
	RefreshButtons();
}

void UMVBossAttackTestWidget::SetTestBoss(AMVEnemy* InBoss)
{
	if (TestBoss == InBoss) return;
	ReleaseManualControl();
	TestBoss = InBoss;
	EnsureMouseInput();
	RefreshButtons();
}

void UMVBossAttackTestWidget::EnsureMouseInput()
{
	APlayerController* Player = GetOwningPlayer();
	if (!Player || !IsVisible()) return;
	UGameViewportClient* Viewport = GetWorld() ? GetWorld()->GetGameViewport() : nullptr;
	const AMVAIController* AI = IsValid(TestBoss) ? Cast<AMVAIController>(TestBoss->GetController()) : nullptr;
	if (AI && AI->BossPlayStart && !bHasManualControl)
	{
		if (!Player->bShowMouseCursor && Viewport
			&& Viewport->GetMouseCaptureMode() == EMouseCaptureMode::CapturePermanently_IncludingInitialMouseDown
			&& Viewport->GetMouseLockMode() == EMouseLockMode::LockAlways) return;
		Player->SetShowMouseCursor(false);
		FInputModeGameOnly InputMode;
		InputMode.SetConsumeCaptureMouseDown(false);
		Player->SetInputMode(InputMode);
		if (Viewport)
		{
			Viewport->SetMouseCaptureMode(EMouseCaptureMode::CapturePermanently_IncludingInitialMouseDown);
			Viewport->SetMouseLockMode(EMouseLockMode::LockAlways);
		}
		return;
	}
	if (Player->bShowMouseCursor && Viewport && Viewport->GetMouseLockMode() == EMouseLockMode::DoNotLock) return;
	// CommonUI의 기본 게임 입력 복구 후에도 시험 패널의 후속 클릭 허용
	Player->SetShowMouseCursor(true);
	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	Player->SetInputMode(InputMode);
}

AMVEnemy* UMVBossAttackTestWidget::ResolveBoss()
{
	if (IsValid(TestBoss)) return TestBoss;
	ReleaseManualControl();
	if (!GetWorld() || !GetWorld()->IsGameWorld()) return nullptr;
	const APawn* Player = GetOwningPlayerPawn();
	float NearestDistance = TNumericLimits<float>::Max();
	for (TActorIterator<AMVEnemy> It(GetWorld()); It; ++It)
	{
		if (!It->FindComponentByClass<UMVDuelistResonanceComponent>()) continue;
		const float Distance = Player ? FVector::DistSquared(Player->GetActorLocation(), It->GetActorLocation()) : 0.0f;
		if (Distance < NearestDistance)
		{
			NearestDistance = Distance;
			TestBoss = *It;
		}
	}
	return TestBoss;
}

bool UMVBossAttackTestWidget::CanStartTest() const
{
	if (!IsValid(TestBoss) || !TestBoss->StatComponent || TestBoss->StatComponent->IsDead()
		|| TestBoss->StatComponent->IsGroggy() || !TestBoss->CombatComponent || !TestBoss->ActionComponent
		|| TestBoss->ActionComponent->IsActionRunning()) return false;
	const UMVDuelistResonanceComponent* Resonance = TestBoss->FindComponentByClass<UMVDuelistResonanceComponent>();
	return !Resonance || !Resonance->IsResonanceActive();
}

void UMVBossAttackTestWidget::RefreshButtons()
{
	const bool bReady = CanStartTest();
	for (UButton* Button : {BaseAttackButton.Get(), QAttackButton.Get(), WAttackButton.Get(), PazeChangeAttackButton.Get()})
	{
		if (Button) Button->SetIsEnabled(bReady);
	}
}

void UMVBossAttackTestWidget::AcquireManualControl()
{
	if (!bHasManualControl)
	{
		bHasManualControl = true;
		SavedAttackTarget = TestBoss->AttackTarget;
		ControlledAI = Cast<AMVAIController>(TestBoss->GetController());
		if (AMVAIController* AI = ControlledAI.Get())
		{
			SavedAITarget = AI->TargetActor;
			bWasAITickEnabled = AI->IsActorTickEnabled();
			AI->SetActorTickEnabled(false);
			ControlledTree = AI->FindComponentByClass<UStateTreeAIComponent>();
			if (UStateTreeAIComponent* Tree = ControlledTree.Get())
			{
				bWasTreeRunning = Tree->IsRunning();
				Tree->StopLogic(TEXT("Boss attack test buttons"));
			}
		}
	}
	if (APawn* Player = GetOwningPlayerPawn())
	{
		TestBoss->AttackTarget = Player;
		if (AMVAIController* AI = ControlledAI.Get())
		{
			AI->TargetActor = Player;
			AI->StopMovement();
		}
		TestBoss->SetActorRotation(TestBoss->GetTargetRotation());
	}
}

void UMVBossAttackTestWidget::ReleaseManualControl()
{
	if (!bHasManualControl) return;
	if (IsValid(TestBoss))
	{
		if (UMVDuelistResonanceComponent* Resonance = TestBoss->FindComponentByClass<UMVDuelistResonanceComponent>()) Resonance->CancelResonance();
		if (TestBoss->ActionComponent) TestBoss->ActionComponent->CancelActiveAction();
		TestBoss->AttackTarget = SavedAttackTarget.Get();
	}
	if (AMVAIController* AI = ControlledAI.Get())
	{
		AI->TargetActor = SavedAITarget.Get();
		AI->SetActorTickEnabled(bWasAITickEnabled);
	}
	if (UStateTreeAIComponent* Tree = ControlledTree.Get(); Tree && bWasTreeRunning && IsValid(TestBoss)
		&& TestBoss->StatComponent && !TestBoss->StatComponent->IsDead()) Tree->StartLogic();
	ControlledAI.Reset();
	ControlledTree.Reset();
	SavedAttackTarget.Reset();
	SavedAITarget.Reset();
	bHasManualControl = false;
	bWasTreeRunning = false;
}

void UMVBossAttackTestWidget::ExecuteAttack(FName RowName)
{
	if (!ResolveBoss() || !CanStartTest() || !AttackTable) return;
	AcquireManualControl();
	FDataTableRowHandle Row;
	Row.DataTable = AttackTable;
	Row.RowName = RowName;
	TestBoss->CombatComponent->TryStartCombatActionFromRowHandle(Row);
	RefreshButtons();
}

void UMVBossAttackTestWidget::HandleBaseAttackClicked() { ExecuteAttack(TEXT("BasicAttack1")); }
void UMVBossAttackTestWidget::HandleQAttackClicked() { ExecuteAttack(TEXT("SkillQ")); }
void UMVBossAttackTestWidget::HandleWAttackClicked() { ExecuteAttack(TEXT("SkillW")); }

void UMVBossAttackTestWidget::HandlePazeChangeAttackClicked()
{
	if (!ResolveBoss() || !CanStartTest()) return;
	UMVDuelistResonanceComponent* Resonance = TestBoss->FindComponentByClass<UMVDuelistResonanceComponent>();
	if (!Resonance) return;
	AcquireManualControl();
	Resonance->StartResonanceForTest();
	RefreshButtons();
}
