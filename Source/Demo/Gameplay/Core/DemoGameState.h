// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "DemoGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerListChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameStartedInRoom);
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
	
	UPROPERTY(BlueprintAssignable)
	FOnPlayerListChanged OnPlayerListChanged;
	
	UPROPERTY(BlueprintAssignable)
	FOnGameStartedInRoom OnGameStartedInRoom;
	
	UPROPERTY(BlueprintAssignable)
	FOnNotifyCountDownLeftTime OnNotifyCountDownLeftTime;
	
	UPROPERTY(BlueprintAssignable)
	FOnCurrentAICountChanged OnCurrentAICountChanged;
	
	UPROPERTY(BlueprintAssignable)
	FOnCurrentDefeatCountChanged OnCurrentDefeatCountChanged;
	
	UPROPERTY(BlueprintAssignable)
	FOnCurrentRoundIndexChanged OnCurrentRoundIndexChanged;
	
	void SetHasGameStartedInRoom(bool bInHasGameStartedInRoom);
	void SetCountDownEndTimeStamp(int64 InCountDownEndTimeStamp);
	void SetCurrentAICount(int32 InCurrentAICount);
	void SetCurrentRoundIndex(int32 InCurrentRoundIndex);
	
	int32 GetCurrentAICount() const { return CurrentAICount; }
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	UFUNCTION()
	void OnRep_BHasGameStartedInRoom() const;
	
	UFUNCTION()
	void OnRep_CountDownEndTimeStamp() const;
	
	UFUNCTION()
	void OnRep_CurrentAICount() const;
	
	UFUNCTION()
	void OnRep_CurrentRoundIndex() const;
	
protected:
	UPROPERTY(ReplicatedUsing=OnRep_BHasGameStartedInRoom)
	bool bHasGameStartedInRoom = false;
	
	UPROPERTY(ReplicatedUsing=OnRep_CountDownEndTimeStamp)
	int64 CountDownEndTimeStamp = 0;
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_CurrentAICount)
	int32 CurrentAICount = 0;
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_CurrentRoundIndex)
	int32 CurrentRoundIndex = 0;
};
