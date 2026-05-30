// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Core/DemoPlayerState.h"

#include "DemoGameState.h"
#include "Net/UnrealNetwork.h"


void ADemoPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION(ADemoPlayerState, DefeatCount, COND_OwnerOnly);
}

void ADemoPlayerState::OnEnemyDefeated()
{
	++DefeatCount;
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
	if (ADemoGameState* DemoGameState = Cast<ADemoGameState>(GetWorld()->GetGameState()))
	{
		DemoGameState->OnCurrentDefeatCountChanged.Broadcast(DefeatCount);
	}
}
