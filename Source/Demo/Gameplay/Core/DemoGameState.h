// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "DemoGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerListChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameStartedInRoom);

/**
 * 
 */
UCLASS()
class ADemoGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	virtual void AddPlayerState(APlayerState* PlayerState) override;
	
	virtual void RemovePlayerState(APlayerState* PlayerState) override;
	
	UPROPERTY(BlueprintAssignable)
	FOnPlayerListChanged OnPlayerListChanged;
	
	UPROPERTY(BlueprintAssignable)
	FOnGameStartedInRoom OnGameStartedInRoom;
	
	UFUNCTION(NetMulticast, Reliable)
	void MultiNotifyGameStartedInRoom();
	
	void SetHasGameStartedInRoom(bool bInHasGameStartedInRoom) { bHasGameStartedInRoom = bInHasGameStartedInRoom; }
	
protected:
	bool bHasGameStartedInRoom = false;
};
