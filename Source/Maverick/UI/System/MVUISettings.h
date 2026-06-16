#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "MVUISettings.generated.h"

class UMVHUDWidgetBase;
class UMVMessagePopup;
class UMVPopupBase;
class UMVUILayerBase;
class UMVWindowBase;

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Maverick UI Settings"))
class MAVERICK_API UMVUISettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UMVUISettings();

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Layer")
	TSubclassOf<UMVUILayerBase> RootLayerClass;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Layer")
	int32 RootLayerZOrder = 0;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Layer")
	int32 LayerZOrderStep = 100;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "HUD")
	TSubclassOf<UMVHUDWidgetBase> DefaultHUDClass;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Window")
	TSubclassOf<UMVWindowBase> TitleWindowClass;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Window")
	TSubclassOf<UMVWindowBase> EscapeWindowClass;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Window")
	TSubclassOf<UMVWindowBase> LoadingWindowClass;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Window")
	TSubclassOf<UMVWindowBase> DeathOverlayWindowClass;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Window")
	TSubclassOf<UMVWindowBase> SkillTreeWindowClass;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Window", meta = (ClampMin = "0.0"))
	float DeathOverlayDuration = 3.0f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Popup")
	TSubclassOf<UMVPopupBase> InteractionPromptPopupClass;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Popup")
	TSubclassOf<UMVPopupBase> BossClearPopupClass;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Popup")
	TSubclassOf<UMVPopupBase> DialoguePopupClass;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Popup")
	TSubclassOf<UMVMessagePopup> MessagePopupClass;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Data")
	FName UIMessageTableName = TEXT("UIMessage");
};
