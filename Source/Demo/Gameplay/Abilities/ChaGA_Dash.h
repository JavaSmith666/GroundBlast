// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Abilities/DemoCharacterGameplayAbility.h"
#include "ChaGA_Dash.generated.h"

/**
 * 
 */
UCLASS()
class UChaGA_Dash : public UDemoCharacterGameplayAbility
{
	GENERATED_BODY()
	
protected:
	virtual bool OnDemoCharacterGameplayAbilityActivated(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
	
	virtual void PreDemoCharacterGameplayAbilityEnded(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;
};
