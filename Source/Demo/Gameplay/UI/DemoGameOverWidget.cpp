// Fill out your copyright notice in the Description page of Project Settings.

#include "Gameplay/UI/DemoGameOverWidget.h"
#include "Components/ScrollBox.h"

DEFINE_LOG_CATEGORY(LogDemoGameOverWidget);

void UDemoGameOverWidget::NativeConstruct()
{
	Super::NativeConstruct();
	ViewPortResizeDelegateHandle = GEngine->GameViewport->Viewport->ViewportResizedEvent.AddUObject(this, &UDemoGameOverWidget::HandleViewportResize);
}

void UDemoGameOverWidget::NativeDestruct()
{
	GEngine->GameViewport->Viewport->ViewportResizedEvent.Remove(ViewPortResizeDelegateHandle);
	Super::NativeDestruct();
}

void UDemoGameOverWidget::HandleViewportResize(FViewport* Viewport, uint32 Unknown) const
{
	if (Viewport && MemberScrollBox)
	{
		FVector2D ViewportSize = Viewport->GetSizeXY();
		FVector2D NewRenderSize = FVector2D(ViewportSize.X / BaseResolution.X, ViewportSize.Y / BaseResolution.Y);
		MemberScrollBox->SetRenderScale(NewRenderSize);
		UE_LOG(LogDemoGameOverWidget, Warning, TEXT("Viewport resized: %s, New Render Scale: %s"), *ViewportSize.ToString(), *NewRenderSize.ToString());
	}
}
