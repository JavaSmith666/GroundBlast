// Fill out your copyright notice in the Description page of Project Settings.

#include "Gameplay/Abilities/ChaGA_Dash.h"

#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Gameplay/Character/DemoCharacter.h"

bool UChaGA_Dash::OnDemoCharacterGameplayAbilityActivated(const FGameplayAbilitySpecHandle Handle,
                                                          const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                          const FGameplayEventData* TriggerEventData)
{
	if (OwnerCharacter)
	{
		OwnerCharacter->ResetDashOverlapActorsArray();
	}
	
	if (const bool bRes = Super::OnDemoCharacterGameplayAbilityActivated(Handle, ActorInfo, ActivationInfo, TriggerEventData); !bRes)
	{
		return false;
	}
	
	if (OwnerCharacter)
	{
		if (USphereComponent* DashComponent = OwnerCharacter->GetDashDamageSphere())
		{
			DashComponent->SetCollisionResponseToChannel(ECC_Pawn, ECollisionResponse::ECR_Overlap);
		}
		
		if (UCapsuleComponent* CapsuleComponent = OwnerCharacter->GetCapsuleComponent())
		{
			CapsuleComponent->SetCollisionResponseToChannel(ECC_Pawn, ECollisionResponse::ECR_Overlap);
		}
		
		OwnerCharacter->SetFrictionZero();
		if (UCharacterMovementComponent* CharacterMovement = OwnerCharacter->GetCharacterMovement())
		{
			if (UDashConfig* DashConfig = Cast<UDashConfig>(RoleSkillConfig))
			{
				CharacterMovement->StopMovementImmediately();
				FVector ForwardVector = CharacterMovement->GetForwardVector();
				CharacterMovement->AddImpulse(ForwardVector * DashConfig->ForwardImpulse, true);
			}
		}
	}
	
	return true;
}

void UChaGA_Dash::PreDemoCharacterGameplayAbilityEnded(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if (OwnerCharacter)
	{
		OwnerCharacter->ResetFriction();
		OwnerCharacter->ResetDashOverlapActorsArray();
		
		if (USphereComponent* DashComponent = OwnerCharacter->GetDashDamageSphere())
		{
			DashComponent->SetCollisionResponseToChannel(ECC_Pawn, ECollisionResponse::ECR_Ignore);
		}
		
		if (UCapsuleComponent* CapsuleComponent = OwnerCharacter->GetCapsuleComponent())
		{
			CapsuleComponent->SetCollisionResponseToChannel(ECC_Pawn, ECollisionResponse::ECR_Block);
		}
	}
	
	Super::PreDemoCharacterGameplayAbilityEnded(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
