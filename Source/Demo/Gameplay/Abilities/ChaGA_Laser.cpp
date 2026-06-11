// Fill out your copyright notice in the Description page of Project Settings.

#include "Gameplay/Abilities/ChaGA_Laser.h"
#include "DemoAbilitySystemComponent.h"
#include "Components/ArrowComponent.h"
#include "Gameplay/Character/DemoCharacter.h"
#include "Gameplay/AbilityActors///LaserActor.h"
#include "Gameplay/AbilityActors/SummonItemBase.h"

bool UChaGA_Laser::OnDemoCharacterGameplayAbilityActivated(const FGameplayAbilitySpecHandle Handle,
                                                           const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                           const FGameplayEventData* TriggerEventData)
{	
	bool res = Super::OnDemoCharacterGameplayAbilityActivated(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	ULaserConfig* LaserConfig = Cast<ULaserConfig>(RoleSkillConfig);
	if (!LaserConfig || !OwnerCharacter)
	{
		return res;
	}
	
	if (SummonItem)
	{
		SummonItem->AttachToComponent(OwnerCharacter->GetLaserPoint(), FAttachmentTransformRules::SnapToTargetIncludingScale);
		SpawnedLaserActor = Cast<ALaserActor>(SummonItem);
	}
	
	if (UDemoAbilitySystemComponent* ASC = OwnerCharacter->GetAbilitySystemComponent())
	{
		ASC->AddLooseGameplayTag(LaserConfig->LaserCostTag);
		ASC->GetGameplayAttributeValueChangeDelegate(UBaseAttributeSet::GetMPAttribute()).AddUObject(this, &UChaGA_Laser::OnMPAttributeChanged);
	}
	
	OwnerCharacter->UpdateCameraLockState(true);
	OwnerCharacter->PlaySound(LaserConfig->LaserSoundName, EAudioType::Skill);
	
	return res;
}

void UChaGA_Laser::PreDemoCharacterGameplayAbilityEnded(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	OwnerCharacter->UpdateCameraLockState(false);
	OwnerCharacter->StopSound(EAudioType::Skill);
	
	ULaserConfig* LaserConfig = Cast<ULaserConfig>(RoleSkillConfig);
	if (!LaserConfig)
	{
		return;
	}
	
	if (UDemoAbilitySystemComponent* ASC = OwnerCharacter ? OwnerCharacter->GetAbilitySystemComponent() : nullptr)
	{
		ASC->RemoveLooseGameplayTag(LaserConfig->LaserCostTag);
	}
	
	if (SpawnedLaserActor)
	{
		SpawnedLaserActor->ClearCurrentHitCharacterDamageEffect();
	}
	
	Super::PreDemoCharacterGameplayAbilityEnded(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UChaGA_Laser::OnMPAttributeChanged(const FOnAttributeChangeData& Data)
{
	if (Data.NewValue <= 0.f)
	{
		if (OwnerCharacter && OwnerCharacter->GetNetMode() < NM_Client)
		{
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		}
	}
}
