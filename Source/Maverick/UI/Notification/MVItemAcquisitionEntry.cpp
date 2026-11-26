#include "UI/Notification/MVItemAcquisitionEntry.h"

#include "Animation/WidgetAnimation.h"
#include "Blueprint/WidgetBlueprintGeneratedClass.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/AssetManager.h"
#include "Engine/Texture2D.h"
#include "MovieScene.h"
#include "UI/System/MVUIFont.h"

void UMVItemAcquisitionEntry::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (const auto* BlueprintClass = Cast<UWidgetBlueprintGeneratedClass>(GetClass()))
	{
		for (UWidgetAnimation* Animation : BlueprintClass->Animations)
		{
			if (Animation && Animation->GetMovieScene()->GetFName() == TEXT("Fade"))
			{
				FadeAnimation = Animation;
				break;
			}
		}
	}
	MVUIFont::AddDefaultFallback(*ItemNameText);
	FallbackIcon = Cast<UTexture2D>(ItemIcon->GetBrush().GetResourceObject());
	if (AcquisitionStatusText)
	{
		MVUIFont::AddDefaultFallback(*AcquisitionStatusText);
	}
}

void UMVItemAcquisitionEntry::SetDisplayData(const FMVItemAcquisitionDisplayData& Data)
{
	ItemNameText->SetText(Data.DisplayName);
	RequestedIcon = Data.Icon;
	UTexture2D* LoadedIcon = RequestedIcon.Get();
	ItemIcon->SetBrushFromTexture(LoadedIcon ? LoadedIcon : FallbackIcon.Get());
	if (!LoadedIcon && !RequestedIcon.IsNull())
	{
		const TSoftObjectPtr<UTexture2D> PendingIcon = RequestedIcon;
		UAssetManager::GetStreamableManager().RequestAsyncLoad(PendingIcon.ToSoftObjectPath(),
			FStreamableDelegate::CreateWeakLambda(this, [this, PendingIcon]()
			{
				if (UTexture2D* Texture = PendingIcon.Get(); Texture && RequestedIcon == PendingIcon)
				{
					ItemIcon->SetBrushFromTexture(Texture);
				}
			}));
	}
	if (AcquisitionStatusText)
	{
		AcquisitionStatusText->SetText(Data.bFirstAcquisition
			? NSLOCTEXT("MaverickItemAcquisition", "New", "NEW")
			: NSLOCTEXT("MaverickItemAcquisition", "Acquired", "획득"));
	}
}

float UMVItemAcquisitionEntry::PlayArrival()
{
	if (FadeAnimation)
	{
		PlayAnimation(FadeAnimation);
		return FadeAnimation->GetEndTime() - FadeAnimation->GetStartTime();
	}
	return 0.0f;
}

float UMVItemAcquisitionEntry::PlayDeparture()
{
	if (!FadeAnimation)
	{
		return 0.0f;
	}
	PlayAnimationReverse(FadeAnimation);
	return FadeAnimation->GetEndTime() - FadeAnimation->GetStartTime();
}

void UMVItemAcquisitionEntry::SetCardInFront(const bool bInFront)
{
	const ESlateVisibility ContentVisibility = bInFront ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden;
	ItemNameText->SetVisibility(ContentVisibility);
	ItemIcon->SetVisibility(ContentVisibility);
	if (AcquisitionStatusText)
	{
		AcquisitionStatusText->SetVisibility(ContentVisibility);
	}
}
