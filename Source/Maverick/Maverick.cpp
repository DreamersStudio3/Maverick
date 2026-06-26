// Copyright Epic Games, Inc. All Rights Reserved.

#include "Maverick.h"
#include "Modules/ModuleManager.h"

#if WITH_EDITOR
#include "Misc/MessageDialog.h"
#include "Tables/MVTableAssetGenerator.h"
#include "ToolMenus.h"
#endif

class FMaverickModule : public FDefaultGameModuleImpl
{
public:
	virtual void StartupModule() override
	{
		FDefaultGameModuleImpl::StartupModule();

#if WITH_EDITOR
		if (UToolMenus::IsToolMenuUIEnabled())
		{
			UToolMenus::RegisterStartupCallback(
				FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FMaverickModule::RegisterEditorMenus));
		}
#endif
	}

	virtual void ShutdownModule() override
	{
#if WITH_EDITOR
		if (UToolMenus::IsToolMenuUIEnabled())
		{
			UToolMenus::UnRegisterStartupCallback(this);
			UToolMenus::UnregisterOwner(this);
		}
#endif

		FDefaultGameModuleImpl::ShutdownModule();
	}

private:
#if WITH_EDITOR
	void RegisterEditorMenus()
	{
		FToolMenuOwnerScoped OwnerScoped(this);

		UToolMenu* ToolsMenu = UToolMenus::Get()->ExtendMenu(TEXT("LevelEditor.MainMenu.Tools"));
		FToolMenuSection& Section = ToolsMenu->FindOrAddSection(TEXT("Maverick"));
		Section.AddMenuEntry(
			TEXT("GenerateMaverickDataTables"),
			FText::FromString(TEXT("Generate Data Tables")),
			FText::FromString(TEXT("Run CSV conversion and regenerate Maverick DataTables.")),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateStatic(&FMaverickModule::GenerateDataTablesFromMenu)));
		Section.AddMenuEntry(
			TEXT("RefreshMaverickTableManifest"),
			FText::FromString(TEXT("Refresh Table Manifest")),
			FText::FromString(TEXT("Refresh Maverick table manifest from generated and direct-managed DataTables.")),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateStatic(&FMaverickModule::RefreshTableManifestFromMenu)));
	}

	static void GenerateDataTablesFromMenu()
	{
		FString Report;
		const bool bSucceeded = UMVTableAssetGenerator::GenerateDataTables(Report);
		UE_LOG(LogTemp, Display, TEXT("[MVTableAssetGenerator] %s%s"), bSucceeded ? TEXT("") : TEXT("Failed: "), *Report);

		FMessageDialog::Open(
			EAppMsgType::Ok,
			FText::FromString(bSucceeded
				? FString::Printf(TEXT("DataTable generation completed.\n\n%s"), *Report)
				: FString::Printf(TEXT("DataTable generation failed.\n\n%s"), *Report)));
	}

	static void RefreshTableManifestFromMenu()
	{
		FString Report;
		const bool bSucceeded = UMVTableAssetGenerator::RefreshTableManifest(Report);
		UE_LOG(LogTemp, Display, TEXT("[MVTableAssetGenerator] %s%s"), bSucceeded ? TEXT("") : TEXT("Failed: "), *Report);

		FMessageDialog::Open(
			EAppMsgType::Ok,
			FText::FromString(bSucceeded
				? FString::Printf(TEXT("Table manifest refresh completed.\n\n%s"), *Report)
				: FString::Printf(TEXT("Table manifest refresh failed.\n\n%s"), *Report)));
	}
#endif
};

IMPLEMENT_PRIMARY_GAME_MODULE(FMaverickModule, Maverick, "Maverick");
