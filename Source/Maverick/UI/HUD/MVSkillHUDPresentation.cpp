#include "UI/HUD/MVSkillHUDPresentation.h"

#include "Tables/MVSkillDataTableColumn.h"
#include "Weapon/Skill/MVSkillExecutionData.h"
#include "EnhancedActionKeyMapping.h"
#include "InputTriggers.h"
#include "Algo/Find.h"

namespace
{
FText MVSkillHUDKeyName(const FKey Key)
{
	if (Key == EKeys::LeftShift || Key == EKeys::RightShift) return FText::FromString(TEXT("Shift"));
	if (Key == EKeys::LeftControl || Key == EKeys::RightControl) return FText::FromString(TEXT("Ctrl"));
	if (Key == EKeys::LeftAlt || Key == EKeys::RightAlt) return FText::FromString(TEXT("Alt"));
	if (Key == EKeys::LeftMouseButton) return NSLOCTEXT("SkillHUD", "LeftMouse", "좌클릭");
	if (Key == EKeys::RightMouseButton) return NSLOCTEXT("SkillHUD", "RightMouse", "우클릭");
	if (Key == EKeys::Gamepad_LeftShoulder) return FText::FromString(TEXT("LB"));
	if (Key == EKeys::Gamepad_RightShoulder) return FText::FromString(TEXT("RB"));
	if (Key == EKeys::Gamepad_LeftTrigger) return FText::FromString(TEXT("LT"));
	if (Key == EKeys::Gamepad_RightTrigger) return FText::FromString(TEXT("RT"));
	if (Key == EKeys::Gamepad_FaceButton_Top) return NSLOCTEXT("SkillHUD", "PadTop", "위 버튼");
	if (Key == EKeys::Gamepad_FaceButton_Bottom) return NSLOCTEXT("SkillHUD", "PadBottom", "아래 버튼");
	if (Key == EKeys::Gamepad_FaceButton_Left) return NSLOCTEXT("SkillHUD", "PadLeft", "왼쪽 버튼");
	if (Key == EKeys::Gamepad_FaceButton_Right) return NSLOCTEXT("SkillHUD", "PadRight", "오른쪽 버튼");
	return Key.GetDisplayName(false);
}

FKey MVSkillHUDFindKey(const TArray<FKey>& Keys, const bool bGamepad)
{
	for (const auto& Key : Keys) if (Key.IsValid() && Key.IsGamepadKey() == bGamepad && !Key.IsTouch()) return Key;
	return FKey();
}

bool MVSkillHUDResolveMappingLabel(TConstArrayView<const FEnhancedActionKeyMapping> Mappings, const UInputAction* Action,
	const UInputAction* AdditionalModifier, const bool bGamepad, TSet<const UInputAction*>& Visiting, FText& Out)
{
	if (!Action || Visiting.Contains(Action) || Visiting.Num() >= 8) return false;
	Visiting.Add(Action);
	const auto* Mapping = Algo::FindByPredicate(Mappings, [Action, bGamepad](const auto& Value)
	{
		return Value.Action == Action && Value.Key.IsValid() && Value.Key.IsGamepadKey() == bGamepad && !Value.Key.IsTouch();
	});
	if (!Mapping) { Visiting.Remove(Action); return false; }
	TArray<const UInputAction*> Modifiers;
	if (AdditionalModifier) Modifiers.Add(AdditionalModifier);
	const auto GatherChords = [&Modifiers](const auto& Triggers)
	{
		for (const UInputTrigger* Trigger : Triggers)
		{
			const auto* Chord = Cast<UInputTriggerChordAction>(Trigger);
			if (Chord && !Chord->IsA<UInputTriggerChordBlocker>()) Modifiers.AddUnique(Chord->ChordAction);
		}
	};
	GatherChords(Mapping->Triggers);
	GatherChords(Action->Triggers);
	Out = FText::GetEmpty();
	for (const auto* Modifier : Modifiers)
	{
		FText ModifierLabel;
		if (!MVSkillHUDResolveMappingLabel(Mappings, Modifier, nullptr, bGamepad, Visiting, ModifierLabel))
		{
			Visiting.Remove(Action);
			return false;
		}
		Out = Out.IsEmpty() ? ModifierLabel : FText::Format(NSLOCTEXT("SkillHUD", "Chord", "{0}+{1}"), Out, ModifierLabel);
	}
	const auto KeyLabel = MVSkillHUDKeyName(Mapping->Key.GetVirtualKey());
	Out = Out.IsEmpty() ? KeyLabel : FText::Format(NSLOCTEXT("SkillHUD", "Chord", "{0}+{1}"), Out, KeyLabel);
	Visiting.Remove(Action);
	return true;
}
}

FMVSkillHUDPresentation MVSkillHUDPresentation::Build(const FDataTableRowHandle& Selected, TSoftObjectPtr<UTexture2D> Fallback)
{
	FMVSkillHUDPresentation Result;
	if (Selected.IsNull())
	{
		Result.Status = NSLOCTEXT("SkillHUD", "Empty", "미선택");
		return Result;
	}
	TArray<FName> Rows;
	if (!MVSkillExecutionData::ValidateDefinition(Selected, Rows))
	{
		Result.Icons.Add(Fallback);
		Result.Status = NSLOCTEXT("SkillHUD", "Invalid", "자료 오류");
		return Result;
	}
	for (const FName RowName : Rows)
	{
		const auto* Row = Selected.DataTable->FindRow<FMVSkillDataTableColumn>(RowName, TEXT("SkillHUD.Icon"), false);
		Result.Icons.Add(Row->Icon.IsNull() ? Fallback : Row->Icon);
		if (Result.Icons.Num() == 1)
		{
			Result.Name = Row->DisplayName;
			Result.bPrototype = Row->ImplementationState == EMVSkillImplementationState::Prototype;
		}
	}
	if (Result.bPrototype) Result.Status = NSLOCTEXT("SkillHUD", "Prototype", "준비 중");
	return Result;
}

FText MVSkillHUDPresentation::MakeKeyLabel(const TArray<FKey>& ActionKeys, const TArray<FKey>& ModifierKeys,
	const bool bRequiresModifier, const bool bGamepad, const bool bTouch)
{
	const FKey Action = MVSkillHUDFindKey(ActionKeys, bGamepad);
	const FKey Modifier = MVSkillHUDFindKey(ModifierKeys, bGamepad);
	if (bTouch || !Action.IsValid() || (bRequiresModifier && !Modifier.IsValid()))
		return NSLOCTEXT("SkillHUD", "Unmapped", "미지정");
	return bRequiresModifier
		? FText::Format(NSLOCTEXT("SkillHUD", "Chord", "{0}+{1}"), MVSkillHUDKeyName(Modifier), MVSkillHUDKeyName(Action))
		: MVSkillHUDKeyName(Action);
}

FText MVSkillHUDPresentation::MakeMappedKeyLabel(TConstArrayView<const FEnhancedActionKeyMapping> Mappings,
	const UInputAction* Action, const UInputAction* AdditionalModifier, const bool bGamepad, const bool bTouch)
{
	TSet<const UInputAction*> Visiting;
	FText Label;
	return !bTouch && MVSkillHUDResolveMappingLabel(Mappings, Action, AdditionalModifier, bGamepad, Visiting, Label)
		? Label : NSLOCTEXT("SkillHUD", "Unmapped", "미지정");
}
