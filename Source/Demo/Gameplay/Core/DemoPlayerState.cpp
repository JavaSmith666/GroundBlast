// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Core/DemoPlayerState.h"

#include "DemoGameMode.h"
#include "DemoGameState.h"
#include "DemoPlayerController.h"
#include "Gameplay/Character/DemoCharacter.h"
#include "Net/UnrealNetwork.h"

void ADemoPlayerState::BeginPlay()
{
	Super::BeginPlay();
	
	if (GetNetMode() < NM_Client)
	{
		if (ADemoGameState* DemoGameState = Cast<ADemoGameState>(GetWorld()->GetGameState()))
		{
			Rank = DemoGameState->PlayerArray.Num();
		}
	}
}

ADemoCharacter* ADemoPlayerState::GetOwningCharacter() const
{	
	ADemoPlayerController* DemoPlayerController = Cast<ADemoPlayerController>(GetOwner());
	if (!DemoPlayerController)
	{
		return nullptr;
	}
	
	ADemoCharacter* DemoCharacter = Cast<ADemoCharacter>(DemoPlayerController->GetCharacter());
	return DemoCharacter;
}

void ADemoPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ADemoPlayerState, DefeatCount);
}

void ADemoPlayerState::OnEnemyDefeated()
{
	if (GetNetMode() == NM_Client)
	{
		return;
	}
	
	++DefeatCount;
	
	if (ADemoGameModeRunning* DemoGameModeRunning = GetWorld()->GetAuthGameMode<ADemoGameModeRunning>())
	{
		DemoGameModeRunning->OnPlayerDefeatCountChanged();
	}
	
	if (GetNetMode() == NM_Standalone)
	{
		if (ADemoGameState* DemoGameState = Cast<ADemoGameState>(GetWorld()->GetGameState()))
		{
			DemoGameState->OnCurrentDefeatCountChanged.Broadcast(DefeatCount);
		}
	}
}

void ADemoPlayerState::OnRep_DefeatCount()
{
	ADemoPlayerController* DemoPlayerController = Cast<ADemoPlayerController>(GetOwner());
	if (!DemoPlayerController || !DemoPlayerController->IsLocalController())
	{
		return;
	}
	
	if (ADemoGameState* DemoGameState = Cast<ADemoGameState>(GetWorld()->GetGameState()))
	{
		DemoGameState->OnCurrentDefeatCountChanged.Broadcast(DefeatCount);
	}
}
