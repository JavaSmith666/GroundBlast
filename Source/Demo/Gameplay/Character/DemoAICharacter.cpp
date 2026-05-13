// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Character/DemoAICharacter.h"

#include "Gameplay/Abilities/DemoAbilitySystemComponent.h"

ADemoAICharacter::ADemoAICharacter()
{
	
}

void ADemoAICharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	}
}
