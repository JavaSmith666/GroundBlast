// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Abilities/DemoCharacterHoldingAbility.h"
#include "ChaGA_GroundBlast.generated.h"

/**
 * 
 */
UCLASS()
class UChaGA_GroundBlast : public UDemoCharacterHoldingAbility
{
	GENERATED_BODY()
	
public:
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
		bool bWasCancelled);
};
