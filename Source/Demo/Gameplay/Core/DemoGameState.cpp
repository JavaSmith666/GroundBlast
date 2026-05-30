// Fill out your copyright notice in the Description page of Project Settings.

#include "Gameplay/Core/DemoGameState.h"

#include "DemoGameMode.h"
#include "Net/UnrealNetwork.h"

void ADemoGameState::BeginPlay()
{
	Super::BeginPlay();
	
	bHasGameStartedInRoom = false;
}

void ADemoGameState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	bHasGameStartedInRoom = false;
	
	Super::EndPlay(EndPlayReason);
}

void ADemoGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ADemoGameState, bHasGameStartedInRoom);
	DOREPLIFETIME(ADemoGameState, CurrentAICount);
	DOREPLIFETIME(ADemoGameState, CurrentRoundIndex);
}

void ADemoGameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);
	
	if (GetNetMode() != NM_DedicatedServer)
	{
		OnPlayerListChanged.Broadcast();
	}
}

void ADemoGameState::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);
	
	if (GetNetMode() != NM_DedicatedServer)
	{
		OnPlayerListChanged.Broadcast();
	}
}

void ADemoGameState::SetHasGameStartedInRoom(bool bInHasGameStartedInRoom)
{
	bHasGameStartedInRoom = bInHasGameStartedInRoom;
	if (GetNetMode() == NM_Standalone && bHasGameStartedInRoom)
	{
		OnGameStartedInRoom.Broadcast();
	}
}

void ADemoGameState::SetCountDownEndTimeStamp(int64 InCountDownEndTimeStamp)
{
	CountDownEndTimeStamp = InCountDownEndTimeStamp;
	if (GetNetMode() == NM_Standalone)
	{
		const int32 LeftTime = InCountDownEndTimeStamp - FDateTime::UtcNow().ToUnixTimestamp();
		OnNotifyCountDownLeftTime.Broadcast(LeftTime > 0 ? LeftTime : 0);
	}
}

void ADemoGameState::SetCurrentAICount(int32 InCurrentAICount)
{
	CurrentAICount = InCurrentAICount;
	if (GetNetMode() == NM_Standalone)
	{
		OnCurrentAICountChanged.Broadcast(CurrentAICount);
	}
	
	if (CurrentAICount == 0)
	{
		if (ADemoGameModeRunning* DemoGameMode = GetWorld()->GetAuthGameMode<ADemoGameModeRunning>())
		{
			DemoGameMode->StartNewRound();
		}	
	}
}

void ADemoGameState::SetCurrentRoundIndex(int32 InCurrentRoundIndex)
{
	CurrentRoundIndex = InCurrentRoundIndex;
	if (GetNetMode() == NM_Standalone)
	{
		OnCurrentRoundIndexChanged.Broadcast(CurrentRoundIndex);
	}
}

void ADemoGameState::OnRep_BHasGameStartedInRoom() const
{
	OnGameStartedInRoom.Broadcast();
}

void ADemoGameState::OnRep_CountDownEndTimeStamp() const
{
	const int32 LeftTime = CountDownEndTimeStamp - FDateTime::UtcNow().ToUnixTimestamp();
	OnNotifyCountDownLeftTime.Broadcast(LeftTime > 0 ? LeftTime : 0);
}

void ADemoGameState::OnRep_CurrentAICount() const
{
	OnCurrentAICountChanged.Broadcast(CurrentAICount);
}

void ADemoGameState::OnRep_CurrentRoundIndex() const
{
	OnCurrentRoundIndexChanged.Broadcast(CurrentRoundIndex);
}
