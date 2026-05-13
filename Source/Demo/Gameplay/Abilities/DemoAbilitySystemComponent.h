// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "DemoAbilitySystemComponent.generated.h"

UCLASS()
class UDemoAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UDemoAbilitySystemComponent();
	
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerConfirmHoldingAbility(const FGameplayAbilitySpecHandle Handle, FPredictionKey PredictionKey);
};
