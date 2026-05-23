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
	UFUNCTION(BlueprintCallable)
	void CreateNewRoom(FString InURL);
	
	UFUNCTION(BlueprintPure)
	static FString GetCurrentIPv4Address();
	
	UFUNCTION(BlueprintCallable)
	void StartGame(FString InURL, bool bAbsolute, float LoadLevelDelay);
	
	void AuthStartGameInRoom(FString InURL, bool bAbsolute, float LoadLevelDelay);
	
	UFUNCTION(BlueprintCallable)
	void JoinRoom(FString InIPv4Address, FString InURL);
	
protected:
	UFUNCTION()
	void OnLoadLevelDelayTimeReached(FString InURL, bool bAbsolute, bool bIsStandalone);
};
