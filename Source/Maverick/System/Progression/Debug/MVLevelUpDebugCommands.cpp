#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "UI/System/MVUISubsystem.h"
#include "UI/Window/MVLevelUpWindow.h"

#if !UE_BUILD_SHIPPING

DEFINE_LOG_CATEGORY_STATIC(LogMVLevelUpDebug, Log, All);

namespace
{
	UMVUISubsystem* MVLevelUpDebugResolveUISubsystem(UWorld* World)
	{
		if (!World || !World->IsGameWorld())
		{
			return nullptr;
		}

		UGameInstance* GameInstance = World->GetGameInstance();

		return GameInstance
			? GameInstance->GetSubsystem<UMVUISubsystem>()
			: nullptr;
	}

	void MVLevelUpDebugShowCommand(
		const TArray<FString>& Args,
		UWorld* World)
	{
		if (!Args.IsEmpty())
		{
			UE_LOG(
				LogMVLevelUpDebug,
				Warning,
				TEXT("Usage: MV.UI.LevelUp.Show"));

			return;
		}

		UMVUISubsystem* UI = MVLevelUpDebugResolveUISubsystem(World);

		if (!UI || !UI->ShowLevelUpWindow())
		{
			UE_LOG(
				LogMVLevelUpDebug,
				Warning,
				TEXT("LevelUpDebug show failed. Check the game world, player state, progression data, and LevelUpWindowClass."));

			return;
		}

		UE_LOG(
			LogMVLevelUpDebug,
			Display,
			TEXT("LevelUpDebug show requested."));
	}

	void MVLevelUpDebugHideCommand(
		const TArray<FString>& Args,
		UWorld* World)
	{
		if (!Args.IsEmpty())
		{
			UE_LOG(
				LogMVLevelUpDebug,
				Warning,
				TEXT("Usage: MV.UI.LevelUp.Hide"));

			return;
		}

		UMVUISubsystem* UI = MVLevelUpDebugResolveUISubsystem(World);

		if (!UI || !UI->HideLevelUpWindow())
		{
			UE_LOG(
				LogMVLevelUpDebug,
				Display,
				TEXT("LevelUpDebug no level-up window found."));

			return;
		}

		UE_LOG(
			LogMVLevelUpDebug,
			Display,
			TEXT("LevelUpDebug hide requested."));
	}

	FAutoConsoleCommandWithWorldAndArgs MVLevelUpDebugShowConsoleCommand(
		TEXT("MV.UI.LevelUp.Show"),
		TEXT("Open the configured level-up window."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(
			&MVLevelUpDebugShowCommand));

	FAutoConsoleCommandWithWorldAndArgs MVLevelUpDebugHideConsoleCommand(
		TEXT("MV.UI.LevelUp.Hide"),
		TEXT("Close the level-up window and discard pending allocation."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(
			&MVLevelUpDebugHideCommand));
}

#endif