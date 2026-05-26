// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "DemoGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerListChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameStartedInRoom);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNotifyShowCountDownText);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNotifyStartNewRound, int32, RoundIndex, int32, AICount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNotifyCurrentAICountChanged, int32, CurrentAICount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNotifyDefeatCountChanged, int32, DefeatCount);

/**
 * 
 */
UCLASS()
class ADemoGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	virtual void AddPlayerState(APlayerState* PlayerState) override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > &OutLifetimeProps) const override;
	
	UPROPERTY(BlueprintAssignable)
	FOnPlayerListChanged OnPlayerListChanged;
	
	UPROPERTY(BlueprintAssignable)
	FOnGameStartedInRoom OnGameStartedInRoom;
	
	UPROPERTY(BlueprintAssignable)
	FOnNotifyShowCountDownText OnNotifyShowCountDownText;
	
	UPROPERTY(BlueprintAssignable)
	FOnNotifyStartNewRound OnNotifyStartNewRound;
	
	UPROPERTY(BlueprintAssignable)
	FOnNotifyCurrentAICountChanged OnNotifyCurrentAICountChanged;
	
	UPROPERTY(BlueprintAssignable)
	FOnNotifyDefeatCountChanged OnNotifyDefeatCountChanged;
	
	UFUNCTION(NetMulticast, Reliable)
	void MultiNotifyGameStartedInRoom();
	
	UFUNCTION(NetMulticast, Reliable)
	void MultiNotifyStartCountDownText();
	
	UFUNCTION(NetMulticast, Reliable)
	void MultiNotifyStartNewRound(int32 RoundIndex, int32 AICount);
	
	void SetHasGameStartedInRoom(bool bInHasGameStartedInRoom) { bHasGameStartedInRoom = bInHasGameStartedInRoom; }
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	UFUNCTION()
	void OnRep_CurrentAICount();
	
protected:
	bool bHasGameStartedInRoom = false;
	
	UPROPERTY(ReplicatedUsing=OnRep_CurrentAICount)
	int32 CurrentAICount = 0;
};
