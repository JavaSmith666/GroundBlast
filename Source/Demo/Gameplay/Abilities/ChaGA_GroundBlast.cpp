// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Abilities/ChaGA_GroundBlast.h"

bool UChaGA_GroundBlast::OnDemoCharacterGameplayAbilityActivated(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	bool res = Super::OnDemoCharacterGameplayAbilityActivated(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	AddSkillIMC();
	return res;
}

void UChaGA_GroundBlast::PreDemoCharacterGameplayAbilityEnded(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	RemoveSkillIMC();
	Super::PreDemoCharacterGameplayAbilityEnded(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}