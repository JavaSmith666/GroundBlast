// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "DemoGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerListChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameStartedInRoom);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameOver, bool, bVictory);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNotifyCountDownLeftTime, int32, CountDownLeftTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCurrentAICountChanged, int32, CurrentAICount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCurrentDefeatCountChanged, int32, CurrentDefeatCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCurrentRoundIndexChanged, int32, CurrentDefeatCount);

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
	
	UFUNCTION(NetMulticast, Reliable)
	void MultiNotifyGameStartedInRoom();
	
	UFUNCTION(NetMulticast, Reliable)
	void MultiNotifyGameOver(bool bVictory);
	
	UPROPERTY(BlueprintAssignable)
	FOnCurrentDefeatCountChanged OnCurrentDefeatCountChanged;
	
	UPROPERTY(BlueprintAssignable)
	FOnCurrentRoundIndexChanged OnCurrentRoundIndexChanged;
	
	void SetHasGameStartedInRoom(bool bInHasClientGameStartedInRoom) { bHasClientGameStartedInRoom = bInHasClientGameStartedInRoom; }
	void SetCountDownEndTimeStamp(int64 InCountDownEndTimeStamp);
	void SetCurrentAICount(int32 InCurrentAICount);
	void SetCurrentRoundIndex(int32 InCurrentRoundIndex);
	
	int32 GetCurrentAICount() const { return CurrentAICount; }
	int32 GetCurrentRoundIndex() const { return CurrentRoundIndex; }
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	UFUNCTION()
	void OnRep_CountDownEndTimeStamp() const;
	
	UFUNCTION()
	void OnRep_CurrentAICount() const;
	
	UFUNCTION()
	void OnRep_CurrentRoundIndex() const;
	
protected:
	UPROPERTY(BlueprintAssignable)
	FOnPlayerListChanged OnPlayerListChanged;
	
	UPROPERTY(BlueprintAssignable)
	FOnGameStartedInRoom OnGameStartedInRoom;
	
	UPROPERTY(BlueprintAssignable)
	FOnGameOver OnGameOver;
	
	UPROPERTY(BlueprintAssignable)
	FOnNotifyCountDownLeftTime OnNotifyCountDownLeftTime;
	
	UPROPERTY(BlueprintAssignable)
	FOnCurrentAICountChanged OnCurrentAICountChanged;
	
	bool bHasClientGameStartedInRoom = false;
	
	UPROPERTY(ReplicatedUsing=OnRep_CountDownEndTimeStamp)
	int64 CountDownEndTimeStamp = 0;
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_CurrentAICount)
	int32 CurrentAICount = 0;
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_CurrentRoundIndex)
	int32 CurrentRoundIndex = 0;
};
