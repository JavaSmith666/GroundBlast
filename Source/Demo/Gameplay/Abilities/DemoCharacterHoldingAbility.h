// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Abilities/DemoCharacterGameplayAbility.h"
#include "DemoCharacterHoldingAbility.generated.h"

class ASummonItemBase;

/**
 * 
 */
UCLASS()
class UDemoCharacterHoldingAbility : public UDemoCharacterGameplayAbility
{
	GENERATED_BODY()
	
public:
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	
	void AuthConfirmHoldingAbility(FPredictionKey PredictionKey = FPredictionKey());
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Animation")
	TSoftObjectPtr<UAnimMontage> HoldingMontage;
	
	virtual bool PlayFireMontage() override;
	
protected:
	virtual bool OnDemoCharacterGameplayAbilityActivated(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
	
	virtual void PreDemoCharacterGameplayAbilityEnded(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled);
	
	virtual bool PlayHoldingMontage();
	
	void AddSkillIMC();
	
	void RemoveSkillIMC();

	UFUNCTION()
	void OnHoldingMontageCancelled();
	
	UFUNCTION()
	virtual void OnSkillConfirmed();
	
	UPROPERTY(Transient)
	UAbilityTask_PlayMontageAndWait* HoldingMontageTask = nullptr;
	
private:
	void HandleHoldingMontageEnded(bool bWasCancelled);
	
	void ApplyCostAndCooldown(FPredictionKey PredictionKey);
};
