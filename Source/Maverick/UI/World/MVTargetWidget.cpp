#include "UI/World/MVTargetWidget.h"

void UMVTargetWidget::SetTargeted(bool bInTargeted)
{
	SetVisibility(bInTargeted ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}
