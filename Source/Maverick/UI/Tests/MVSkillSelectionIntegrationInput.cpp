#if WITH_DEV_AUTOMATION_TESTS

#include "HAL/IConsoleManager.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/PlayerController.h"
#include "Character/PC/MVPlayerCharacter.h"
#include "Components/MVActionComponent.h"
#include "Components/MVInputManagerComponent.h"
#include "Components/MVStatComponent.h"
#include "Components/MVWeaponComponent.h"
#include "DataAsset/MVWeaponSkillMap.h"
#include "Weapon/Skill/MVSkillSelectionData.h"
#include "Engine/World.h"
#include "Interaction/RestPoint/MVRestPointSubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogMVSkillSelectionIntegrationInput, Log, All);

namespace
{
// 한 프레임 안의 PressKey와 달리 프레임 사이에 키를 유지하는 개발 검사 도구
TSet<FKey> MVSkillSelectionIntegrationHeldKeys;

void MVSkillSelectionIntegrationSendKey(const FKey Key, const bool bPressed)
{
	if (!FSlateApplication::IsInitialized()) return;
	if (bPressed) MVSkillSelectionIntegrationHeldKeys.Add(Key);
	else MVSkillSelectionIntegrationHeldKeys.Remove(Key);
	const auto Held = [](const FKey InKey) { return MVSkillSelectionIntegrationHeldKeys.Contains(InKey); };
	const FModifierKeysState Modifiers(Held(EKeys::LeftShift), Held(EKeys::RightShift),
		Held(EKeys::LeftControl), Held(EKeys::RightControl), Held(EKeys::LeftAlt), Held(EKeys::RightAlt),
		Held(EKeys::LeftCommand), Held(EKeys::RightCommand), false);
	const FKeyEvent Event(Key, Modifiers, 0, false, 0, 0);
	if (bPressed) FSlateApplication::Get().ProcessKeyDownEvent(Event);
	else FSlateApplication::Get().ProcessKeyUpEvent(Event);
	UE_LOG(LogMVSkillSelectionIntegrationInput, Display, TEXT("SkillIntegrationTrace Frame=%llu Key=%s Pressed=%d"),
		GFrameCounter, *Key.ToString(), bPressed);
}

void MVSkillSelectionIntegrationTrace(const TArray<FString>& Args, UWorld* World)
{
	auto* Controller = World ? World->GetFirstPlayerController() : nullptr;
	auto* Player = Controller ? Cast<AMVPlayerCharacter>(Controller->GetPawn()) : nullptr;
	if (!Player) return;
	const auto* Rest = World->GetSubsystem<UMVRestPointSubsystem>();
	UE_LOG(LogMVSkillSelectionIntegrationInput, Display,
		TEXT("SkillIntegrationTrace Frame=%llu Label=%s Player=%s Rest=%d MoveIgnored=%d LookIgnored=%d Blocked=%d Cursor=%d Action=%d Speed=%.2f W=%d Q=%d"),
		GFrameCounter, Args.IsEmpty() ? TEXT("Snapshot") : *Args[0], *Player->GetName(),
		Rest && Rest->IsUsingRestPoint(*Player), Controller->IsMoveInputIgnored(), Controller->IsLookInputIgnored(),
		Player->InputManagerComponent && Player->InputManagerComponent->IsGameplayInputBlocked(), Controller->bShowMouseCursor,
		Player->ActionComponent && Player->ActionComponent->IsActionRunning(), Player->GetVelocity().Size(),
		Controller->IsInputKeyDown(EKeys::W), Controller->IsInputKeyDown(EKeys::Q));
	if (!Args.IsEmpty() && Args[0] == TEXT("Authored") && Player->WeaponComponent)
	{
		for (const UMVWeaponSkillMap* Map : Player->WeaponComponent->WeaponSkillMaps)
		{
			if (!Map) continue;
			TArray<FString> Errors;
			Map->ValidateSkillCandidates(Errors);
			for (const auto& Error : Errors)
				UE_LOG(LogMVSkillSelectionIntegrationInput, Display, TEXT("SkillIntegrationTrace Frame=%llu Player=%s Map=%s Invalid=%s"),
					GFrameCounter, *Player->GetName(), *Map->GetName(), *Error);
			for (const auto& Definition : MVSkillSelectionData::GetSlots())
			{
				TArray<FString> Rows;
				for (const auto& Entry : Map->GetSortedSkillCandidates(Definition.Slot)) Rows.Add(Entry.SkillRowHandle.RowHandle.RowName.ToString());
				UE_LOG(LogMVSkillSelectionIntegrationInput, Display, TEXT("SkillIntegrationTrace Frame=%llu Player=%s Map=%s Slot=%s Candidates=%s"),
					GFrameCounter, *Player->GetName(), *Map->GetName(), *Definition.DisplayName.ToString(), *FString::Join(Rows, TEXT(",")));
			}
		}
	}
}

FAutoConsoleCommandWithWorldAndArgs MVSkillSelectionIntegrationKeyDownCommand(
	TEXT("Maverick.Test.SkillSelection.KeyDown"), TEXT("Development integration fixture: hold a Slate keyboard key."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		if (!World || Args.Num() != 1) return;
		const FKey Key(*Args[0]);
		if (Key.IsValid() && !Key.IsMouseButton()) MVSkillSelectionIntegrationSendKey(Key, true);
	}));

FAutoConsoleCommandWithWorldAndArgs MVSkillSelectionIntegrationKeyUpCommand(
	TEXT("Maverick.Test.SkillSelection.KeyUp"), TEXT("Development integration fixture: release a held Slate keyboard key."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		if (!World || Args.Num() != 1) return;
		const FKey Key(*Args[0]);
		if (Key.IsValid() && !Key.IsMouseButton()) MVSkillSelectionIntegrationSendKey(Key, false);
	}));

FAutoConsoleCommandWithWorldAndArgs MVSkillSelectionIntegrationReleaseCommand(
	TEXT("Maverick.Test.SkillSelection.ReleaseAll"), TEXT("Development integration fixture: release all synthetic keys."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		const auto Keys = MVSkillSelectionIntegrationHeldKeys.Array();
		for (const auto Key : Keys) MVSkillSelectionIntegrationSendKey(Key, false);
	}));

FAutoConsoleCommandWithWorldAndArgs MVSkillSelectionIntegrationTraceCommand(
	TEXT("Maverick.Test.SkillSelection.Trace"), TEXT("Development integration fixture: trace UI/game input ownership."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&MVSkillSelectionIntegrationTrace));

FAutoConsoleCommandWithWorldAndArgs MVSkillSelectionIntegrationDamageCommand(
	TEXT("Maverick.Test.SkillSelection.Damage"), TEXT("Development integration fixture: deliver resolved player damage; amount or lethal."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		auto* Controller = World ? World->GetFirstPlayerController() : nullptr;
		auto* Player = Controller ? Cast<AMVPlayerCharacter>(Controller->GetPawn()) : nullptr;
		if (!Player || !Player->StatComponent || Args.Num() != 1) return;
		FMVResolvedHitData Hit;
		Hit.Victim = Player;
		Hit.FinalDamage = Args[0] == TEXT("lethal") ? Player->StatComponent->CurrentHP + 1.0f : FCString::Atof(*Args[0]);
		UE_LOG(LogMVSkillSelectionIntegrationInput, Display, TEXT("SkillIntegrationTrace Frame=%llu Damage=%.2f Player=%s"),
			GFrameCounter, Hit.FinalDamage, *Player->GetName());
		Player->StatComponent->HandleDamaged(Hit);
	}));
}
#endif
