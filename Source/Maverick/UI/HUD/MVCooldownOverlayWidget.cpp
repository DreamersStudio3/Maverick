#include "UI/HUD/MVCooldownOverlayWidget.h"

#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"
#include "Widgets/SLeafWidget.h"

class SMVCooldownOverlay final : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SMVCooldownOverlay)
		: _CooldownPercent(0.0f)
		, _OverlayColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.68f))
		, _SegmentCount(64)
	{
	}
		SLATE_ARGUMENT(float, CooldownPercent)
		SLATE_ARGUMENT(FLinearColor, OverlayColor)
		SLATE_ARGUMENT(int32, SegmentCount)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		CooldownPercent = FMath::Clamp(InArgs._CooldownPercent, 0.0f, 1.0f);
		OverlayColor = InArgs._OverlayColor;
		SegmentCount = FMath::Clamp(InArgs._SegmentCount, 8, 128);
	}

	void SetCooldownPercent(const float InCooldownPercent)
	{
		const float NormalizedPercent = FMath::Clamp(InCooldownPercent, 0.0f, 1.0f);
		if (!FMath::IsNearlyEqual(CooldownPercent, NormalizedPercent))
		{
			CooldownPercent = NormalizedPercent;
			Invalidate(EInvalidateWidgetReason::Paint);
		}
	}

	void SetOverlayColor(const FLinearColor& InOverlayColor)
	{
		if (OverlayColor != InOverlayColor)
		{
			OverlayColor = InOverlayColor;
			Invalidate(EInvalidateWidgetReason::Paint);
		}
	}

	void SetSegmentCount(const int32 InSegmentCount)
	{
		const int32 NormalizedSegmentCount = FMath::Clamp(InSegmentCount, 8, 128);
		if (SegmentCount != NormalizedSegmentCount)
		{
			SegmentCount = NormalizedSegmentCount;
			Invalidate(EInvalidateWidgetReason::Paint);
		}
	}

private:
	virtual FVector2D ComputeDesiredSize(float) const override
	{
		return FVector2D(64.0f, 64.0f);
	}

	virtual int32 OnPaint(
		const FPaintArgs& Args,
		const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements,
		const int32 LayerId,
		const FWidgetStyle& InWidgetStyle,
		const bool bParentEnabled) const override
	{
		if (CooldownPercent <= KINDA_SMALL_NUMBER || OverlayColor.A <= KINDA_SMALL_NUMBER)
		{
			return LayerId;
		}

		const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush(TEXT("GenericWhiteBox"));
		if (!WhiteBrush)
		{
			return LayerId;
		}

		const FSlateResourceHandle ResourceHandle = WhiteBrush->GetRenderingResource();
		if (!ResourceHandle.IsValid())
		{
			return LayerId;
		}

		const FVector2D LocalSize = AllottedGeometry.GetLocalSize();
		const FVector2D LocalCenter = LocalSize * 0.5f;
		const float Radius = 0.5f * FMath::Sqrt(FMath::Square(LocalSize.X) + FMath::Square(LocalSize.Y)) + 1.0f;
		const int32 DrawSegmentCount = FMath::Max(1, FMath::CeilToInt(SegmentCount * CooldownPercent));
		const float SweepAngle = UE_TWO_PI * CooldownPercent;
		const float StartAngle = -UE_HALF_PI + UE_TWO_PI * (1.0f - CooldownPercent);
		const FColor VertexColor = OverlayColor.ToFColor(true);

		TArray<FSlateVertex> Vertices;
		Vertices.Reserve(DrawSegmentCount + 2);

		auto AddVertex = [&Vertices, &AllottedGeometry, VertexColor](const FVector2D& LocalPosition)
		{
			const FVector2D AbsolutePosition = AllottedGeometry.LocalToAbsolute(LocalPosition);
			Vertices.AddZeroed();
			FSlateVertex& Vertex = Vertices.Last();
			Vertex.Position[0] = static_cast<float>(AbsolutePosition.X);
			Vertex.Position[1] = static_cast<float>(AbsolutePosition.Y);
			Vertex.TexCoords[0] = 0.5f;
			Vertex.TexCoords[1] = 0.5f;
			Vertex.TexCoords[2] = 1.0f;
			Vertex.TexCoords[3] = 1.0f;
			Vertex.Color = VertexColor;
		};

		AddVertex(LocalCenter);
		for (int32 SegmentIndex = 0; SegmentIndex <= DrawSegmentCount; ++SegmentIndex)
		{
			const float SegmentAlpha = static_cast<float>(SegmentIndex) / static_cast<float>(DrawSegmentCount);
			const float Angle = StartAngle + SweepAngle * SegmentAlpha;
			const FVector2D Direction(FMath::Cos(Angle), FMath::Sin(Angle));
			AddVertex(LocalCenter + Direction * Radius);
		}

		TArray<SlateIndex> Indices;
		Indices.Reserve(DrawSegmentCount * 3);
		for (int32 SegmentIndex = 0; SegmentIndex < DrawSegmentCount; ++SegmentIndex)
		{
			Indices.Add(0);
			Indices.Add(static_cast<SlateIndex>(SegmentIndex + 1));
			Indices.Add(static_cast<SlateIndex>(SegmentIndex + 2));
		}

		const ESlateDrawEffect DrawEffect = bParentEnabled
			? ESlateDrawEffect::None
			: ESlateDrawEffect::DisabledEffect;
		FSlateDrawElement::MakeCustomVerts(
			OutDrawElements,
			LayerId,
			ResourceHandle,
			Vertices,
			Indices,
			nullptr,
			0,
			0,
			DrawEffect);

		return LayerId + 1;
	}

	float CooldownPercent = 0.0f;
	FLinearColor OverlayColor = FLinearColor::Transparent;
	int32 SegmentCount = 64;
};

void UMVCooldownOverlayWidget::SetCooldownPercent(const float InCooldownPercent)
{
	CooldownPercent = FMath::Clamp(InCooldownPercent, 0.0f, 1.0f);
	if (MyCooldownOverlay.IsValid())
	{
		MyCooldownOverlay->SetCooldownPercent(CooldownPercent);
	}
}

void UMVCooldownOverlayWidget::SetOverlayColor(const FLinearColor InOverlayColor)
{
	OverlayColor = InOverlayColor;
	if (MyCooldownOverlay.IsValid())
	{
		MyCooldownOverlay->SetOverlayColor(OverlayColor);
	}
}

TSharedRef<SWidget> UMVCooldownOverlayWidget::RebuildWidget()
{
	SAssignNew(MyCooldownOverlay, SMVCooldownOverlay)
		.CooldownPercent(CooldownPercent)
		.OverlayColor(OverlayColor)
		.SegmentCount(SegmentCount);
	return MyCooldownOverlay.ToSharedRef();
}

void UMVCooldownOverlayWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	if (MyCooldownOverlay.IsValid())
	{
		MyCooldownOverlay->SetCooldownPercent(CooldownPercent);
		MyCooldownOverlay->SetOverlayColor(OverlayColor);
		MyCooldownOverlay->SetSegmentCount(SegmentCount);
	}
}

void UMVCooldownOverlayWidget::ReleaseSlateResources(const bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	MyCooldownOverlay.Reset();
}
