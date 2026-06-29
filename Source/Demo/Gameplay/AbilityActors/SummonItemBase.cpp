// Fill out your copyright notice in the Description page of Project Settings.

#include "Gameplay/AbilityActors/SummonItemBase.h"
#include "Gameplay/Abilities/DemoAbilitySystemComponent.h"
#include "Gameplay/Character/DemoCharacter.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffect.h"
#include "Gameplay/Abilities/DemoCharacterGameplayAbility.h"

DEFINE_LOG_CATEGORY(LogSummonItemBase);

ASummonItemBase::ASummonItemBase()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ASummonItemBase::BeginPlay()
{
	Super::BeginPlay();
	
	OwnerCharacter = Cast<ADemoCharacter>(GetOwner());
	OwnerPlayerCharacter = OwnerCharacter ? OwnerCharacter->GetPlayerController() : nullptr;
	CheckAndReplaceLocalPredictedActor();
}

void ASummonItemBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ASummonItemBase, bActorActive)
}

void ASummonItemBase::OnRep_ActorActive()
{
	SetActorTickEnabled(bActorActive);
}

void ASummonItemBase::CheckAndReplaceLocalPredictedActor()
{
	if (!OwnerCharacter || OwnerCharacter->GetNetMode() < NM_Client)
	{
		return;
	}
	
	bool bFound = false;
	if (UDemoAbilitySystemComponent* ASC = OwnerCharacter->GetAbilitySystemComponent())
	{
		TArray<FGameplayAbilitySpec>& AbilitySpecs = ASC->GetActivatableAbilities();
		for (auto& AbilitySpec : AbilitySpecs)
		{
			if (AbilitySpec.Ability->AbilityTags.HasTag(SourceAbilityTag))
			{
				TArray<UGameplayAbility*> AbilityInstances = AbilitySpec.GetAbilityInstances();
				for (UGameplayAbility* Ability : AbilityInstances)
				{
					UDemoCharacterGameplayAbility* DemoAbility = Cast<UDemoCharacterGameplayAbility>(Ability);
					if (DemoAbility && DemoAbility->GetOwnerCharacter() == OwnerCharacter)
					{
						if (DemoAbility->IsActive() && !DemoAbility->HasAbilityBeenConfirmed())
						{
							bFound = true;
						}
						
						ASummonItemBase* SummonItem = DemoAbility->GetSummonItem();
						if (SummonItem && SummonItem != this)
						{
							SummonItem->Destroy();
							DemoAbility->SetSummonItem(this);
						}
					}
				}	
			}
		}
	}
	
	if (!bFound && OwnerCharacter->IsLocallyControlled())
	{
		// 技能已结束且不是模拟端
		HiddenSelf();
	}
}

void ASummonItemBase::HiddenSelf()
{
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);
}

void ASummonItemBase::ApplyEffectsToFilterActors(const TArray<AActor*>& FilterActors)
{
	for (AActor* Actor : FilterActors)
	{
		ADemoCharacter* FilterCharacter = Cast<ADemoCharacter>(Actor);
		if (!FilterCharacter)
		{
			continue;
		}
		
		if (UDemoAbilitySystemComponent* ASC = FilterCharacter->GetAbilitySystemComponent())
		{
			UClass* DamageClass = DamageEffectClass.IsNull() ? nullptr : DamageEffectClass.LoadSynchronous();
			FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
			ContextHandle.AddInstigator(OwnerCharacter, OwnerCharacter);
			FGameplayEffectSpecHandle DamageEffectSpec = ASC->MakeOutgoingSpec(DamageClass, 1.f, ContextHandle);
			ASC->ApplyGameplayEffectSpecToSelf(*DamageEffectSpec.Data.Get());
			UE_LOG(LogSummonItemBase, Log, TEXT("[ASummonItemBase::ApplyEffectsToFilterActors] Instigator: %s"), *OwnerCharacter->GetName());
		}
	}
}

void ASummonItemBase::ExecuteDamageGameplayCue()
{
	if (!OwnerCharacter)
	{
		return;
	}
	
	FGameplayCueParameters Parameters;
	Parameters.Location = AbilityConfirmedLocation;
	if (UDemoAbilitySystemComponent* ASC = OwnerCharacter->GetAbilitySystemComponent())
	{
		ASC->ExecuteGameplayCue(DamageCueTag, Parameters);
	}
}

void ASummonItemBase::SetDead(float InDelay)
{
	if (InDelay <= 0.f)
	{
		Destroy();
		return;
	}
	
	SetLifeSpan(InDelay);
}
