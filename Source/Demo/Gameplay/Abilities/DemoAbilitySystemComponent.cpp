// Copyright Epic Games, Inc. All Rights Reserved.

#include "DemoAbilitySystemComponent.h"
#include "DemoCharacterHoldingAbility.h"

UDemoAbilitySystemComponent::UDemoAbilitySystemComponent()
{
}

void UDemoAbilitySystemComponent::ServerConfirmHoldingAbility_Implementation(const FGameplayAbilitySpecHandle Handle, FPredictionKey PredictionKey)
{
	FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(Handle);
	if (!AbilitySpec)
	{
		return;
	}
	
	if (!AbilitySpec->IsActive())
	{
		return;
	}
	
	UDemoCharacterHoldingAbility* HoldingAbility = nullptr;
	for (UGameplayAbility* ActiveAbility : AbilitySpec->GetAbilityInstances())
	{
		if (ActiveAbility && ActiveAbility->GetCurrentActorInfo() != nullptr)
		{
			HoldingAbility = Cast<UDemoCharacterHoldingAbility>(ActiveAbility);
			if (HoldingAbility)
			{
				HoldingAbility->AuthConfirmHoldingAbility(PredictionKey);
				break;
			}
		}
	}
}

bool UDemoAbilitySystemComponent::ServerConfirmHoldingAbility_Validate(const FGameplayAbilitySpecHandle Handle, FPredictionKey PredictionKey)
{
	return true;
}
