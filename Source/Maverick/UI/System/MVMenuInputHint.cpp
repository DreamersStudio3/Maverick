#include "UI/System/MVMenuInputHint.h"

#include "CommonInputSubsystem.h"
#include "CommonUITypes.h"

FText MVMenuInputHint::MakeActionLabel(
	const ULocalPlayer* LocalPlayer, const FDataTableRowHandle& Action, const FText& Label)
{
	const FCommonInputActionDataBase* Data = Action.GetRow<FCommonInputActionDataBase>(TEXT("MVMenuInputHint"));
	if (!Data)
	{
		return Label;
	}
	const UCommonInputSubsystem* Input = UCommonInputSubsystem::Get(LocalPlayer);
	const FKey Key = Input ? Data->GetCurrentInputTypeInfo(Input).GetKey()
		: Data->GetInputTypeInfo(ECommonInputType::MouseAndKeyboard, NAME_None).GetKey();
	if (!Key.IsValid())
	{
		return Label;
	}
	FText KeyLabel = Key.GetDisplayName(false);
	if (Key == EKeys::Gamepad_FaceButton_Bottom)
	{
		KeyLabel = NSLOCTEXT("MVMenuInputHint", "GamepadFaceBottom", "아래 버튼");
	}
	else if (Key == EKeys::Gamepad_FaceButton_Right)
	{
		KeyLabel = NSLOCTEXT("MVMenuInputHint", "GamepadFaceRight", "오른쪽 버튼");
	}
	return FText::Format(NSLOCTEXT("MVMenuInputHint", "ActionKey", "[{0}] {1}"), KeyLabel, Label);
}
