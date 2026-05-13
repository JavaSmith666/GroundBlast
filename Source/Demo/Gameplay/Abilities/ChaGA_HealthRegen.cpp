// Fill out your copyright notice in the Description page of Project Settings.

#include "Gameplay/Abilities/ChaGA_HealthRegen.h"
#include "DemoAbilitySystemComponent.h"
#include "Gameplay/Character/DemoCharacter.h"

bool UChaGA_HealthRegen::OnDemoCharacterGameplayAbilityActivated(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	bool res = Super::OnDemoCharacterGameplayAbilityActivated(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	if (OwnerCharacter)
	{
		UHealthRegenConfig* HealthRegenConfig = Cast<UHealthRegenConfig>(RoleSkillConfig);
		if (HealthRegenConfig && HealthRegenConfig->HealthRegenEffect)
		{
			if (UDemoAbilitySystemComponent* ASC = OwnerCharacter->GetAbilitySystemComponent())
			{
				if (OwnerCharacter->GetNetMode() < NM_Client)
				{
					FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(HealthRegenConfig->HealthRegenEffect, 1, MakeEffectContext(Handle, ActorInfo));
					ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
				}
			}
		}
	}
	
	return res;
}

bool UChaGA_HealthRegen::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                            const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
                                            const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	bool res = Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
	if (!res)
	{
		return false;
	}
	
	ADemoCharacter* Character = Cast<ADemoCharacter>(ActorInfo->AvatarActor.Get());
	if (!Character)
	{
		return false;
	}
	
	UDemoAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent();
	if (!ASC)
	{
		return false;
	}
	
	bool found = false;
	float CurrentHealth = ASC->GetGameplayAttributeValue(UBaseAttributeSet::GetHPAttribute(), found);
	float MaxHealth = ASC->GetGameplayAttributeValue(UBaseAttributeSet::GetMaxHPAttribute(), found);
	if (CurrentHealth >= MaxHealth)
	{
		return false;
	}
	
	return true;
}
