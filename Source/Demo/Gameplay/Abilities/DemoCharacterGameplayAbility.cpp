// Copyright Epic Games, Inc. All Rights Reserved.

#include "DemoCharacterGameplayAbility.h"

#include "DemoAbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Animation/AnimMontage.h"
#include "Gameplay/AbilityActors/SummonItemBase.h"
#include "Gameplay/Character/DemoCharacter.h"

UDemoCharacterGameplayAbility::UDemoCharacterGameplayAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UDemoCharacterGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	// Try to get SkillConfig from SourceObject if not already set
	if (USkillConfig* Config = Cast<USkillConfig>(Spec.SourceObject))
	{
		RoleSkillConfig = Config;
	    AbilityIndex = Config->AbilityIndex;
	    if (ADemoCharacter* Character = Cast<ADemoCharacter>(ActorInfo->AvatarActor.Get()))
	    {
	        OwnerCharacter = Character;
	        OwnerPlayerController = Cast<APlayerController>(OwnerCharacter->GetController());
	        if (Character->bHasMainUICreated)
	        {
	            OnMainUICreated();
	        }
	        else
	        {
	            Character->OnMainUICreated.AddDynamic(this, &UDemoCharacterGameplayAbility::OnMainUICreated);
	        }
	    }
	}
}

void UDemoCharacterGameplayAbility::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
    
    if (!OnDemoCharacterGameplayAbilityActivated(Handle, ActorInfo, ActivationInfo, TriggerEventData) && HasAuthority(&ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
    }
}

bool UDemoCharacterGameplayAbility::OnDemoCharacterGameplayAbilityActivated(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    if (!ActorInfo || !RoleSkillConfig || !OwnerCharacter)
    {
        return false;
    }
    
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        return false;
    }   

    if (!PlayFireMontage())
    {
        return false;
    }
    
    if (OwnerCharacter->GetNetMode() != NM_DedicatedServer)
    {
        OwnerCharacter->StartCD(AbilityIndex);
    }
    
    if (RoleSkillConfig->SummonItemClass)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = OwnerCharacter;
        SpawnParams.Instigator = OwnerCharacter;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        SummonItem = GetWorld()->SpawnActor<ASummonItemBase>(RoleSkillConfig->SummonItemClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
    }
    
    return true;
}

void UDemoCharacterGameplayAbility::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility,
    bool bWasCancelled)
{
    PreDemoCharacterGameplayAbilityEnded(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UDemoCharacterGameplayAbility::PreDemoCharacterGameplayAbilityEnded(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility, bool bWasCancelled)
{
    ActiveMontageTask = nullptr;
    
    if (SummonItem)
    {
        SummonItem->SetDead();
        SummonItem = nullptr;
    }
}

bool UDemoCharacterGameplayAbility::PlayFireMontage()
{
    if (!RoleSkillConfig)
    {
        return false;
    }
    
    ActiveMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, RoleSkillConfig->FireMontage);
    if (!ActiveMontageTask)
    {
        return false;
    }

    ActiveMontageTask->OnCompleted.AddDynamic(this, &UDemoCharacterGameplayAbility::OnFireMontageCompleted);
    ActiveMontageTask->OnBlendOut.AddDynamic(this, &UDemoCharacterGameplayAbility::OnFireMontageBlendOut);
    ActiveMontageTask->OnInterrupted.AddDynamic(this, &UDemoCharacterGameplayAbility::OnFireMontageInterrupted);
    ActiveMontageTask->OnCancelled.AddDynamic(this, &UDemoCharacterGameplayAbility::OnFireMontageCancelled);
    ActiveMontageTask->ReadyForActivation();

    return true;
}

void UDemoCharacterGameplayAbility::OnFireMontageCompleted()
{
    HandleFireMontageEnded(false);
}

void UDemoCharacterGameplayAbility::OnFireMontageBlendOut()
{
    HandleFireMontageEnded(false);
}

void UDemoCharacterGameplayAbility::OnFireMontageInterrupted()
{
    HandleFireMontageEnded(true);
}

void UDemoCharacterGameplayAbility::OnFireMontageCancelled()
{
    HandleFireMontageEnded(true);
}

void UDemoCharacterGameplayAbility::OnMainUICreated()
{
    if (OwnerCharacter && RoleSkillConfig)
    {
        UGameplayEffect* CooldownGE = GetCooldownGameplayEffect();
        if (!CooldownGE)
        {
            return;
        }
    
        float CD = 0.f;
        CooldownGE->DurationMagnitude.GetStaticMagnitudeIfPossible(1, CD);
        OwnerCharacter->InitSkillIcon(AbilityIndex, CD, RoleSkillConfig->AbilityMaterialInstance);
    }
}

void UDemoCharacterGameplayAbility::HandleFireMontageEnded(bool bWasCancelled)
{
    if (ActiveMontageTask)
    {
        ActiveMontageTask->OnCompleted.RemoveDynamic(this, &UDemoCharacterGameplayAbility::OnFireMontageCompleted);
        ActiveMontageTask->OnBlendOut.RemoveDynamic(this, &UDemoCharacterGameplayAbility::OnFireMontageBlendOut);
        ActiveMontageTask->OnInterrupted.RemoveDynamic(this, &UDemoCharacterGameplayAbility::OnFireMontageInterrupted);
        ActiveMontageTask->OnCancelled.RemoveDynamic(this, &UDemoCharacterGameplayAbility::OnFireMontageCancelled);
    }
    
    if (!IsActive())
    {
        return;
    }

    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, bWasCancelled);
}