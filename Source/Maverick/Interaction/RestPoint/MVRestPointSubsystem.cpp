#include "Interaction/RestPoint/MVRestPointSubsystem.h"

#include "Character/PC/MVPlayerCharacter.h"
#include "Cinematics/Boss/MVBossIntroTrigger.h"
#include "Components/MVActionComponent.h"
#include "Components/MVInputManagerComponent.h"
#include "Components/MVStatComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Interaction/RestPoint/MVRestPoint.h"
#include "System/MVFieldTransitionSubsystem.h"
#include "System/MVWorldStateSubsystem.h"
#include "UI/Base/MVHUDWidgetBase.h"
#include "UI/System/MVUILayerBase.h"
#include "UI/System/MVUISubsystem.h"
#include "UI/Window/MVRestPointWindow.h"
#include "UI/Window/MVLevelUpWindow.h"
#include "UI/Window/MVWeaponPassiveSelectionWindow.h"

bool UMVRestPointSubsystem::DoesSupportWorldType(EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

TStatId UMVRestPointSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UMVRestPointSubsystem, STATGROUP_Tickables);
}

void UMVRestPointSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UWorld* World = GetWorld();
	APlayerController* Controller = World ? World->GetFirstPlayerController() : nullptr;
	AMVPlayerCharacter* Player = Controller && Controller->IsLocalController()
		? Cast<AMVPlayerCharacter>(Controller->GetPawn()) : nullptr;
	ObservePlayer(Player);

	if (ActiveLayer.IsExplicitlyNull())
	{
		return;
	}

	const UMVUILayerBase* Layer = ActiveLayer.Get();
	if (bMenuClosed || !ActiveWindow.IsValid() || !Layer || !Layer->IsInViewport()
		|| Layer->FindWindowByClass(UMVRestPointWindow::StaticClass()) != ActiveWindow.Get()
		|| !HasValidSessionPlayer(Player))
	{
		Close();
		return;
	}

	const double Now = World->GetTimeSeconds();
	if (!IsRestPending() && Now >= NextSafetyCheckAt)
	{
		NextSafetyCheckAt = Now + 0.2;
		if (ActiveRestPoint->HasLivingEnemyNearby())
		{
			Close();
		}
	}
}

void UMVRestPointSubsystem::Deinitialize()
{
	Close();
	ObservePlayer(nullptr);
	Super::Deinitialize();
}

FText UMVRestPointSubsystem::GetEntryBlockReason(
	const AMVRestPoint& RestPoint, const AMVPlayerCharacter& Player) const
{
	const UWorld* World = GetWorld();
	const APlayerController* Controller = World ? World->GetFirstPlayerController() : nullptr;
	const UMVFieldTransitionSubsystem* Transition = UMVFieldTransitionSubsystem::Get(this);
	if (!Controller || !Controller->IsLocalController() || Controller->GetPawn() != &Player
		|| !RestPoint.IsInRange(Player) || !Player.StatComponent || !Player.StatComponent->IsBaseStatsReady()
		|| Player.StatComponent->IsDead() || (Transition && Transition->IsTransitionRunning()))
	{
		return NSLOCTEXT("MVRestPoint", "Unavailable", "지금은 거점을 이용할 수 없습니다");
	}

	if (Controller->IsMoveInputIgnored() || (Player.ActionComponent && Player.ActionComponent->IsActionRunning())
		|| (Player.InputManagerComponent && Player.InputManagerComponent->IsGameplayInputBlocked()))
	{
		return NSLOCTEXT("MVRestPoint", "PlayerBusy", "현재 행동이 끝난 뒤 이용할 수 있습니다");
	}

	if (ObservedStats.Get() == Player.StatComponent && LastDamageAt.IsSet()
		&& World->GetTimeSeconds() - LastDamageAt.GetValue() < 5.0)
	{
		return NSLOCTEXT("MVRestPoint", "RecentDamage", "피격 직후에는 이용할 수 없습니다");
	}

	if (RestPoint.HasLivingEnemyNearby())
	{
		return NSLOCTEXT("MVRestPoint", "EnemiesNearby", "주변에 적이 있습니다");
	}

	return FText::GetEmpty();
}

bool UMVRestPointSubsystem::Open(AMVRestPoint& RestPoint, AMVPlayerCharacter& Player)
{
	if (ActivePlayer.IsValid() || !GetEntryBlockReason(RestPoint, Player).IsEmpty())
	{
		return false;
	}

	UGameInstance* Instance = GetWorld()->GetGameInstance();
	UMVUISubsystem* UI = Instance ? Instance->GetSubsystem<UMVUISubsystem>() : nullptr;
	UMVUILayerBase* Layer = UI ? UI->GetOrCreateRootLayer(GetWorld()) : nullptr;
	if (!Layer || Layer->FindWindowByClass(UCommonActivatableWidget::StaticClass()))
	{
		return false;
	}

	ObservePlayer(&Player);
	ActiveRestPoint = &RestPoint;
	ActivePlayer = &Player;
	ActiveLayer = Layer;
	NextSafetyCheckAt = 0.0;
	BlockedInput = Player.InputManagerComponent.Get();
	if (UMVInputManagerComponent* Input = BlockedInput.Get())
	{
		Input->BeginGameplayInputBlock();
	}

	if (UMVHUDWidgetBase* HUD = UI->GetMainHUD())
	{
		SuppressedHUD = HUD;
		PreviousHUDVisibility = HUD->GetVisibility();
		HUD->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (!Layer->PushWindow(RestPoint.GetMenuClass(), [this](UCommonActivatableWidget& Window)
	{
		ActiveWindow = CastChecked<UMVRestPointWindow>(&Window);
	}))
	{
		ClearSession();
		return false;
	}

	return true;
}

void UMVRestPointSubsystem::CloseFor(const AMVRestPoint& RestPoint)
{
	if (ActiveRestPoint == &RestPoint)
	{
		Close();
	}
}

bool UMVRestPointSubsystem::NotifyMenuActivated(const UMVRestPointWindow& Window, bool& bReturningFromDetail)
{
	if (bClosing || bMenuClosed || ActiveWindow != &Window)
	{
		return false;
	}
	bReturningFromDetail = bMenuOpened;
	bMenuOpened = true;
	ActiveDetail.Reset();
	if (!bReturningFromDetail && HasValidSessionPlayer(ActivePlayer.Get()))
	{
		const AMVRestPoint& Point = *ActiveRestPoint.Get();
		if (UMVWorldStateSubsystem* State = UMVWorldStateSubsystem::Get(this);
			State && State->RecordRestPoint(Point.GetRestPointId(), Point.GetFieldId(), Point.GetRespawnTransform()))
		{
			State->SaveActiveSlot();
		}
	}
	return true;
}

void UMVRestPointSubsystem::NotifyMenuDeactivated(const UMVRestPointWindow& Window)
{
	if (ActiveWindow == &Window && !bOpeningDetail && !IsRestPending())
	{
		bMenuClosed = true;
	}
}

bool UMVRestPointSubsystem::OpenDetail(const UMVRestPointWindow& Window, EMVRestPointDetail Detail)
{
	if (bOpeningDetail || ActiveDetail.IsValid() || ActiveWindow != &Window || !Window.IsActivated()
		|| Window.IsFadingOut() || !CanUseDetail())
	{
		return false;
	}
	UMVUILayerBase* Layer = ActiveLayer.Get();
	if (!Layer || Layer->FindWindowByClass(UCommonActivatableWidget::StaticClass()) != &Window)
	{
		return false;
	}

	UGameInstance* Instance = GetWorld()->GetGameInstance();
	UMVUISubsystem* UI = Instance ? Instance->GetSubsystem<UMVUISubsystem>() : nullptr;
	if (!UI)
	{
		return false;
	}

	TGuardValue<bool> OpeningGuard(bOpeningDetail, true);
	if (Detail == EMVRestPointDetail::LevelUp)
	{
		ActiveDetail = UI->ShowLevelUpWindowForRestPoint(*this);
	}
	else
	{
		UMVWeaponPassiveSelectionWindow* Passive = UI->ShowWeaponPassiveSelectionWindow();
		if (Passive && Layer->FindWindowByClass(UMVWeaponPassiveSelectionWindow::StaticClass()) == Passive)
		{
			ActiveDetail = Passive;
		}
	}
	return ActiveDetail.IsValid();
}

bool UMVRestPointSubsystem::CanUseDetail() const
{
	const AMVPlayerCharacter* Player = ActivePlayer.Get();
	return !IsRestPending() && !bClosing && !bMenuClosed && ActiveWindow.IsValid() && HasValidSessionPlayer(Player)
		&& (!Player->ActionComponent || !Player->ActionComponent->IsActionRunning())
		&& !ActiveRestPoint->HasLivingEnemyNearby();
}

bool UMVRestPointSubsystem::RequestRest(const UMVRestPointWindow& Window)
{
	AMVRestPoint* Point = ActiveRestPoint.Get();
	UMVFieldTransitionSubsystem* Transition = UMVFieldTransitionSubsystem::Get(this);
	if (!Point || !Transition || ActiveWindow != &Window || !Window.IsActivated() || Window.IsFadingOut()
		|| ActiveDetail.IsValid() || bOpeningDetail || !CanUseDetail())
	{
		return false;
	}

	PendingRestRequestId = ++NextRestRequestId;
	RestStatus = NSLOCTEXT("MVRestPoint", "RestInProgress", "휴식 중");
	if (!IsRestRequestValid(PendingRestRequestId) || !Transition->StartRestTransition(*Point, PendingRestRequestId))
	{
		PendingRestRequestId = 0;
		RestStatus = FText::GetEmpty();
		return false;
	}
	return true;
}

bool UMVRestPointSubsystem::IsRestRequestValid(const uint64 RequestId) const
{
	const AMVPlayerCharacter* Player = ActivePlayer.Get();
	if (RequestId == 0 || RequestId != PendingRestRequestId || bClosing || bMenuClosed
		|| !ActiveWindow.IsValid() || !HasValidSessionPlayer(Player)
		|| (Player->ActionComponent && Player->ActionComponent->IsActionRunning())
		|| ActiveRestPoint->HasLivingEnemyNearby())
	{
		return false;
	}
	for (TActorIterator<AMVBossIntroTrigger> It(GetWorld()); It; ++It)
	{
		if (It->IsIntroPlaying())
		{
			return false;
		}
	}

	const FTransform Anchor = ActiveRestPoint->GetRespawnTransform();
	FVector Location = Anchor.GetLocation();
	return Anchor.IsValid() && GetWorld()->FindTeleportSpot(Player, Location, Anchor.Rotator());
}

void UMVRestPointSubsystem::FinishRestRequest(const uint64 RequestId, const bool bApplied)
{
	if (RequestId != PendingRestRequestId || !ActiveWindow.IsValid())
	{
		return;
	}
	PendingRestRequestId = 0;
	if (!HasValidSessionPlayer(ActivePlayer.Get()))
	{
		Close();
		return;
	}

	RestStatus = NSLOCTEXT("MVRestPoint", "RestIncomplete", "휴식을 완료하지 못했습니다");
	if (bApplied)
	{
		UMVWorldStateSubsystem* State = UMVWorldStateSubsystem::Get(this);
		const AMVRestPoint& Point = *ActiveRestPoint.Get();
		if (State)
		{
			State->RecordRestPoint(Point.GetRestPointId(), Point.GetFieldId(), Point.GetRespawnTransform());
		}
		RestStatus = State && State->SaveActiveSlot()
			? NSLOCTEXT("MVRestPoint", "RestComplete", "휴식과 저장을 마쳤습니다")
			: NSLOCTEXT("MVRestPoint", "RestSaveFailed", "회복했지만 저장하지 못했습니다");
	}
	if (UMVRestPointWindow* Window = ActiveWindow.Get())
	{
		Window->ShowRestStatus(RestStatus);
	}
}

bool UMVRestPointSubsystem::TryResolveRespawnAnchor(
	const FMVCheckpointSaveData& Record, FTransform& OutTransform) const
{
	if (!Record.bHasCheckpoint || Record.CheckpointId.IsNone() || Record.FieldId.IsNone()
		|| Record.MapName != UMVWorldStateSubsystem::GetRestPointMapId(*GetWorld()))
	{
		return false;
	}

	const AMVRestPoint* Match = nullptr;
	for (TActorIterator<AMVRestPoint> It(GetWorld()); It; ++It)
	{
		if (It->GetRestPointId() == Record.CheckpointId && It->GetFieldId() == Record.FieldId)
		{
			if (Match)
			{
				return false;
			}
			Match = *It;
		}
	}

	if (!Match)
	{
		return false;
	}
	OutTransform = Match->GetRespawnTransform();
	return OutTransform.IsValid();
}

bool UMVRestPointSubsystem::HasValidSessionPlayer(const AMVPlayerCharacter* Player) const
{
	const APlayerController* Controller = GetWorld()->GetFirstPlayerController();
	const AMVRestPoint* RestPoint = ActiveRestPoint.Get();
	const UMVFieldTransitionSubsystem* Transition = UMVFieldTransitionSubsystem::Get(this);
	return Player && ActivePlayer.Get() == Player && Controller && Controller->IsLocalController()
		&& Controller->GetPawn() == Player && Player->StatComponent && Player->StatComponent->IsBaseStatsReady()
		&& !Player->StatComponent->IsDead() && RestPoint && RestPoint->IsInRange(*Player)
		&& (!Transition || !Transition->IsTransitionRunning() || Transition->IsRestTransition(PendingRestRequestId));
}

bool UMVRestPointSubsystem::IsUsingRestPoint(const AMVPlayerCharacter& Player) const
{
	return ActivePlayer == &Player;
}

void UMVRestPointSubsystem::ObservePlayer(AMVPlayerCharacter* Player)
{
	UMVStatComponent* Stats = Player ? Player->StatComponent.Get() : nullptr;
	if (ObservedStats.Get() == Stats)
	{
		return;
	}

	if (UMVStatComponent* PreviousStats = ObservedStats.Get())
	{
		PreviousStats->OnDamageApplied.RemoveDynamic(this, &ThisClass::HandleDamage);
	}

	ObservedStats = Stats;
	LastDamageAt.Reset();
	if (Stats)
	{
		Stats->OnDamageApplied.AddUniqueDynamic(this, &ThisClass::HandleDamage);
	}
}

void UMVRestPointSubsystem::HandleDamage(
	float AppliedDamage, float PreviousHP, float CurrentHP, const FMVResolvedHitData& HitData)
{
	if (AppliedDamage > 0.0f)
	{
		LastDamageAt = GetWorld()->GetTimeSeconds();
		Close();
	}
}

void UMVRestPointSubsystem::Close()
{
	if (bClosing)
	{
		return;
	}

	TGuardValue<bool> ClosingGuard(bClosing, true);
	if (UMVUILayerBase* Layer = ActiveLayer.Get())
	{
		// 부모부터 제거하면 상세 창이 닫혀도 거점 목록이 다시 활성화되지 않는다.
		Layer->RemoveWindow(ActiveWindow.Get());
		Layer->RemoveWindow(ActiveDetail.Get());
	}
	ClearSession();
}

void UMVRestPointSubsystem::ClearSession()
{
	if (UMVInputManagerComponent* Input = BlockedInput.Get())
	{
		Input->EndGameplayInputBlock();
	}
	if (UMVHUDWidgetBase* HUD = SuppressedHUD.Get())
	{
		UGameInstance* Instance = GetWorld()->GetGameInstance();
		UMVUISubsystem* UI = Instance ? Instance->GetSubsystem<UMVUISubsystem>() : nullptr;
		if (UI && UI->GetMainHUD() == HUD)
		{
			HUD->SetVisibility(PreviousHUDVisibility);
		}
	}
	SuppressedHUD.Reset();
	BlockedInput.Reset();
	ActiveWindow.Reset();
	ActiveDetail.Reset();
	ActiveLayer.Reset();
	ActivePlayer.Reset();
	ActiveRestPoint.Reset();
	PendingRestRequestId = 0;
	RestStatus = FText::GetEmpty();
	bMenuOpened = false;
	bMenuClosed = false;
}
