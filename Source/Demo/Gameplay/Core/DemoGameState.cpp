// Fill out your copyright notice in the Description page of Project Settings.

#include "Gameplay/Core/DemoGameState.h"

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
	if (!bHasGameStartedInRoom)
	{
		OnGameStartedInRoom.Broadcast();	
	}
}
