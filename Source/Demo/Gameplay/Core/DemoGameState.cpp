// Fill out your copyright notice in the Description page of Project Settings.

#include "Gameplay/Core/DemoGameState.h"

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
	
	DOREPLIFETIME(ADemoGameState, CurrentAICount);
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

void ADemoGameState::MultiNotifyGameStartedInRoom_Implementation()
{
	if (!bHasGameStartedInRoom && GetNetMode() != NM_DedicatedServer)
	{
		OnGameStartedInRoom.Broadcast();	
	}
}

void ADemoGameState::MultiNotifyStartCountDownText_Implementation()
{
	if (GetNetMode() != NM_DedicatedServer)
	{
		OnNotifyShowCountDownText.Broadcast();	
	}
}

void ADemoGameState::MultiNotifyStartNewRound_Implementation(int32 RoundIndex, int32 AICount)
{
	if (GetNetMode() != NM_DedicatedServer)
	{
		OnNotifyStartNewRound.Broadcast(RoundIndex, AICount);
	}
	
	if (GetNetMode() < NM_Client)
	{
		CurrentAICount = AICount;
	}
}

void ADemoGameState::OnRep_CurrentAICount()
{
	OnNotifyCurrentAICountChanged.Broadcast(CurrentAICount);
}