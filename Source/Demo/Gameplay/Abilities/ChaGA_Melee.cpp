// Copyright Epic Games, Inc. All Rights Reserved.

#include "ChaGA_Melee.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Gameplay/Character/DemoCharacter.h"

bool UChaGA_Melee::PlayFireMontage()
{
    if (!RoleSkillConfig || !RoleSkillConfig->FireMontage || !OwnerCharacter)
    {
        return false;
    }
    
    UCharacterMovementComponent* CharacterMovementComponent = OwnerCharacter->GetCharacterMovement();
    if (!CharacterMovementComponent)
    {
        return false;
    }

    const bool bIsInFalling = CharacterMovementComponent->IsFalling();
    const FName StartSection(*FString::FromInt(bIsInFalling ? 2 : 1));
    ActiveMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, RoleSkillConfig->FireMontage, 1.0f, StartSection);
    if (!ActiveMontageTask)
    {
        return false;
    }

    ActiveMontageTask->OnCompleted.AddDynamic(this, &UChaGA_Melee::OnFireMontageCompleted);
    ActiveMontageTask->OnBlendOut.AddDynamic(this, &UChaGA_Melee::OnFireMontageBlendOut);
    ActiveMontageTask->OnInterrupted.AddDynamic(this, &UChaGA_Melee::OnFireMontageInterrupted);
    ActiveMontageTask->OnCancelled.AddDynamic(this, &UChaGA_Melee::OnFireMontageCancelled);
    ActiveMontageTask->ReadyForActivation();

    return true;
}

