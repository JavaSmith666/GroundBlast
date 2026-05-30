// Fill out your copyright notice in the Description page of Project Settings.

#include "Gameplay/AbilityActors//LaserActor.h"
#include "Gameplay/Abilities/DemoAbilitySystemComponent.h"
#include "AbilitySystemComponent.h"
#include "Gameplay/Character/DemoCharacter.h"
#include "GameplayEffect.h"
#include "Components/ArrowComponent.h"

ALaserActor::ALaserActor()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ALaserActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (GetNetMode() < NM_Client)
	{
		if (!OwnerCharacter || !OwnerCharacter->LaserPoint)
		{
			return;
		}
		
		const FVector StartLocation = OwnerCharacter->LaserPoint->GetComponentLocation();
		const FVector ForwardDir = OwnerCharacter->GetActorForwardVector();
		const FVector EndLocation = StartLocation + ForwardDir * LaserTraceMaxDistance;
		
		FHitResult HitResult;		
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);
		QueryParams.AddIgnoredActor(OwnerCharacter);

		if (bool bHit = GetWorld()->LineTraceSingleByObjectType(HitResult, StartLocation, EndLocation, ECC_Pawn, QueryParams))
		{
			AActor* HitActor = HitResult.GetActor();
			if (ADemoCharacter* HitCharacter = Cast<ADemoCharacter>(HitActor))
			{
				if (!HitCharacter->IsDead() && HitCharacter != CurrentHitCharacter && HitCharacter->GetTeamID() != OwnerCharacter->GetTeamID())
				{
					ClearCurrentHitCharacterDamageEffect();
					CurrentHitCharacter = HitCharacter;
					if (UAbilitySystemComponent* HitCharacterASC = HitCharacter->GetAbilitySystemComponent())
					{
						if (UClass* GEClass = LaserDamageEffect.IsNull() ? nullptr : LaserDamageEffect.LoadSynchronous())
						{
							FGameplayEffectContextHandle ContextHandle = HitCharacterASC->MakeEffectContext();
							ContextHandle.AddInstigator(OwnerCharacter, OwnerCharacter);
							FGameplayEffectSpecHandle SpecHandle = HitCharacterASC->MakeOutgoingSpec(GEClass, 1.f, ContextHandle);
							CurrentHitCharacterDamageEffectHandle = HitCharacterASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());	
						}
					}
				}

				return;
			}
		}
		
		ClearCurrentHitCharacterDamageEffect();
	}
}

void ALaserActor::ClearCurrentHitCharacterDamageEffect()
{
	if (CurrentHitCharacter)
	{
		if (UAbilitySystemComponent* CurrentASC = CurrentHitCharacter->GetAbilitySystemComponent())
		{
			CurrentASC->RemoveActiveGameplayEffect(CurrentHitCharacterDamageEffectHandle);
		}
	}
	
	CurrentHitCharacter = nullptr;
}