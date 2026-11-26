#include "UI/System/MVUIFont.h"

#include "Components/TextBlock.h"
#include "Styling/CoreStyle.h"

void MVUIFont::AddDefaultFallback(UTextBlock& TextBlock)
{
	FSlateFontInfo Font = TextBlock.GetFont();
	const FCompositeFont* Original = Font.GetCompositeFont();
	if (!Original || !Original->FallbackTypeface.Typeface.Fonts.IsEmpty())
	{
		return;
	}

	const TSharedRef<FStandaloneCompositeFont> Composite = MakeShared<FStandaloneCompositeFont>();
	static_cast<FCompositeFont&>(Composite.Get()) = *Original;
	Composite->FallbackTypeface = FCoreStyle::GetDefaultFont()->FallbackTypeface;
	Font.FontObject = nullptr;
	Font.CompositeFont = Composite;
	TextBlock.SetFont(Font);
}
