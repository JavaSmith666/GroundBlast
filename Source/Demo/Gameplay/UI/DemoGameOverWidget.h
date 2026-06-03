// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DemoGameOverWidget.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDemoGameOverWidget, Log, All);

class UScrollBox;

/**
 * 
 */
UCLASS()
class UDemoGameOverWidget : public UUserWidget
{
	GENERATED_BODY()
	
	
public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UScrollBox* MemberScrollBox;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Scale")
	FVector2D BaseResolution = FVector2D(1920.f, 1080.f);
	
protected:
	void HandleViewportResize(FViewport* Viewport, uint32 Unknown) const;
	
	FDelegateHandle ViewPortResizeDelegateHandle;
};
