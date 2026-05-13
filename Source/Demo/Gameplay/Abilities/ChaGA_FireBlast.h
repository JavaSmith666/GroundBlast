// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Abilities/DemoCharacterGameplayAbility.h"
#include "ChaGA_FireBlast.generated.h"

class AFireBlastActor;

/**
 * 
 */
UCLASS()
class UChaGA_FireBlast : public UDemoCharacterGameplayAbility
{
	GENERATED_BODY()
	
public:
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	
protected:
	virtual bool OnDemoCharacterGameplayAbilityActivated(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData);
	
	UFUNCTION()
	void OnWaitPullEventTaskReady(FGameplayEventData Payload);
	
	UFUNCTION()
	void OnWaitPushEventTaskReady(FGameplayEventData Payload);
	
	void ScanEnemies(TArray<ADemoCharacter*>& Enemies);
	
	UPROPERTY(Transient)
	UFireBlastConfig* FireBlastConfig = nullptr;
	
	UPROPERTY(Transient)
	TArray<ADemoCharacter*> CacheOverlapedEnemies;
};
