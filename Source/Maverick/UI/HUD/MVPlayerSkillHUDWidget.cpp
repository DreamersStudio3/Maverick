#include "UI/HUD/MVPlayerSkillHUDWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/MVCombatComponent.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/Texture2D.h"
#include "GameFramework/Pawn.h"
#include "UI/HUD/MVSkillSlotWidget.h"

UMVPlayerSkillHUDWidget::UMVPlayerSkillHUDWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	QSkillIconAssets = {
		TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/UI/Textures/Skills/Yone/T_Icon_Yone_SkillQ_1.T_Icon_Yone_SkillQ_1"))),
		TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/UI/Textures/Skills/Yone/T_Icon_Yone_SkillQ_2.T_Icon_Yone_SkillQ_2"))),
		TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/UI/Textures/Skills/Yone/T_Icon_Yone_SkillQ_3.T_Icon_Yone_SkillQ_3")))
	};
	RSkillIconAssets = {
		TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/UI/Textures/Skills/Yone/T_Icon_Yone_SkillR.T_Icon_Yone_SkillR")))
	};
}

void UMVPlayerSkillHUDWidget::BindToCombatComponent(UMVCombatComponent* InCombatComponent)
{
	BoundCombatComponent = InCombatComponent;
	RefreshSkillSlots();
}

void UMVPlayerSkillHUDWidget::UnbindCombatComponent()
{
	BoundCombatComponent = nullptr;
	RefreshSkillSlots();
}

void UMVPlayerSkillHUDWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	BuildNativeWidgetTree();
	ResolveIconAssets();
	if (QSkillSlot)
	{
		QSkillSlot->SetHotKeyText(FText::FromString(TEXT("Q")));
		QSkillSlot->SetSlotSize(SkillSlotSize);
	}
	if (RSkillSlot)
	{
		RSkillSlot->SetHotKeyText(FText::FromString(TEXT("R")));
		RSkillSlot->SetSlotSize(SkillSlotSize);
	}
	if (QChainTimer)
	{
		QChainTimer->SetHotKeyText(FText::GetEmpty());
		QChainTimer->SetSlotSize(SkillSlotSize * ChainTimerScale);
	}
	RefreshSkillSlots();
}

void UMVPlayerSkillHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	APawn* OwningPawn = GetOwningPlayerPawn();
	BindToCombatComponent(OwningPawn ? OwningPawn->FindComponentByClass<UMVCombatComponent>() : nullptr);
}

void UMVPlayerSkillHUDWidget::NativeDestruct()
{
	UnbindCombatComponent();
	Super::NativeDestruct();
}

void UMVPlayerSkillHUDWidget::NativeTick(const FGeometry& MyGeometry, const float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	RefreshSkillSlots();
}

bool UMVPlayerSkillHUDWidget::BuildNativeWidgetTree()
{
	if (!WidgetTree || WidgetTree->RootWidget)
	{
		return false;
	}

	UHorizontalBox* SkillRow = WidgetTree->ConstructWidget<UHorizontalBox>(
		UHorizontalBox::StaticClass(),
		TEXT("SkillRow"));
	UVerticalBox* QSkillColumn = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(),
		TEXT("QSkillColumn"));
	QChainTimer = WidgetTree->ConstructWidget<UMVSkillSlotWidget>(
		UMVSkillSlotWidget::StaticClass(),
		TEXT("QChainTimer"));
	QSkillSlot = WidgetTree->ConstructWidget<UMVSkillSlotWidget>(
		UMVSkillSlotWidget::StaticClass(),
		TEXT("QSkillSlot"));
	RSkillSlot = WidgetTree->ConstructWidget<UMVSkillSlotWidget>(
		UMVSkillSlotWidget::StaticClass(),
		TEXT("RSkillSlot"));

	WidgetTree->RootWidget = SkillRow;
	if (UHorizontalBoxSlot* QColumnSlot = SkillRow->AddChildToHorizontalBox(QSkillColumn))
	{
		QColumnSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
		QColumnSlot->SetHorizontalAlignment(HAlign_Center);
		QColumnSlot->SetVerticalAlignment(VAlign_Bottom);
		QColumnSlot->SetPadding(FMargin(0.0f, 0.0f, SkillSlotGap * 0.5f, 0.0f));
	}
	if (UVerticalBoxSlot* TimerSlot = QSkillColumn->AddChildToVerticalBox(QChainTimer))
	{
		TimerSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
		TimerSlot->SetHorizontalAlignment(HAlign_Left);
		TimerSlot->SetVerticalAlignment(VAlign_Center);
		TimerSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, ChainTimerGap));
	}
	if (UVerticalBoxSlot* QSlot = QSkillColumn->AddChildToVerticalBox(QSkillSlot))
	{
		QSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
		QSlot->SetHorizontalAlignment(HAlign_Center);
		QSlot->SetVerticalAlignment(VAlign_Center);
	}
	if (UHorizontalBoxSlot* RSlot = SkillRow->AddChildToHorizontalBox(RSkillSlot))
	{
		RSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
		RSlot->SetHorizontalAlignment(HAlign_Center);
		RSlot->SetVerticalAlignment(VAlign_Bottom);
		RSlot->SetPadding(FMargin(SkillSlotGap * 0.5f, 0.0f, 0.0f, 0.0f));
	}

	return true;
}

void UMVPlayerSkillHUDWidget::ResolveIconAssets()
{
	ResolvedQSkillIcons.Reset(QSkillIconAssets.Num());
	for (const TSoftObjectPtr<UTexture2D>& IconAsset : QSkillIconAssets)
	{
		if (UTexture2D* Icon = IconAsset.LoadSynchronous())
		{
			ResolvedQSkillIcons.Add(Icon);
		}
	}

	ResolvedRSkillIcons.Reset(RSkillIconAssets.Num());
	for (const TSoftObjectPtr<UTexture2D>& IconAsset : RSkillIconAssets)
	{
		if (UTexture2D* Icon = IconAsset.LoadSynchronous())
		{
			ResolvedRSkillIcons.Add(Icon);
		}
	}

	if (QSkillSlot)
	{
		TArray<UTexture2D*> QIcons;
		QIcons.Reserve(ResolvedQSkillIcons.Num());
		for (UTexture2D* Icon : ResolvedQSkillIcons)
		{
			QIcons.Add(Icon);
		}
		QSkillSlot->SetIconStack(QIcons);

		if (QChainTimer && !QIcons.IsEmpty())
		{
			TArray<UTexture2D*> ChainTimerIcons;
			ChainTimerIcons.Add(QIcons[0]);
			QChainTimer->SetIconStack(ChainTimerIcons);
		}
	}

	if (RSkillSlot)
	{
		TArray<UTexture2D*> RIcons;
		RIcons.Reserve(ResolvedRSkillIcons.Num());
		for (UTexture2D* Icon : ResolvedRSkillIcons)
		{
			RIcons.Add(Icon);
		}
		RSkillSlot->SetIconStack(RIcons);
	}
}

void UMVPlayerSkillHUDWidget::RefreshSkillSlots()
{
	FMVSkillSlotRuntimeState QState;
	const bool bQAvailable = BoundCombatComponent
		&& BoundCombatComponent->GetSkillSlotRuntimeState(0, QState);
	if (QSkillSlot)
	{
		QSkillSlot->SetRuntimeState(
			QState.ActiveStackIndex,
			QState.CooldownRemaining,
			QState.CooldownDuration,
			bQAvailable);
	}
	if (QChainTimer)
	{
		const bool bShowChainTimer = bQAvailable
			&& QState.bChainActive
			&& QState.ChainWindowRemaining > KINDA_SMALL_NUMBER;
		QChainTimer->SetVisibility(
			bShowChainTimer ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
		QChainTimer->SetRuntimeState(
			0,
			QState.ChainWindowRemaining,
			QState.ChainWindowDuration,
			bQAvailable);
	}

	FMVSkillSlotRuntimeState RState;
	const bool bRAvailable = BoundCombatComponent
		&& BoundCombatComponent->GetSkillSlotRuntimeState(1, RState);
	if (RSkillSlot)
	{
		RSkillSlot->SetRuntimeState(
			RState.ActiveStackIndex,
			RState.CooldownRemaining,
			RState.CooldownDuration,
			bRAvailable);
	}
}
