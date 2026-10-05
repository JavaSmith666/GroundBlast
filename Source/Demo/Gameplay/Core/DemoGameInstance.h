// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "DemoGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class UDemoGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	virtual void Init() override;
	
	UFUNCTION(BlueprintPure, Category = "MVVM | Game State")
	UGameStateViewModel* GetGlobalGameStateViewModel() const { return CachedGameStateViewModel; }
	
protected:
	UPROPERTY(Transient)
	UGameStateViewModel* CachedGameStateViewModel = nullptr;
};
