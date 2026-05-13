// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpec.h"
#include "GameFramework/Actor.h"
#include "GameplayTags.h"
#include "SummonItemBase.generated.h"

class ADemoCharacter;
class UGameplayEffect;

UCLASS()
class ASummonItemBase : public AActor
{
	GENERATED_BODY()
	
public:	
	ASummonItemBase();
	
	void SetActorActive(bool bActivate) { bActorActive = bActivate; }
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual void ConfirmHoldingAbility(TArray<AActor*>& FilterActors) {}
	
	virtual void ApplyEffectsToFilterActors(const TArray<AActor*>& FilterActors);
	
	virtual void SpawnEmitter();
	
	void SetDead(float InDelay = 0.f);
	
	void HiddenSelf();

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	TSoftClassPtr<UGameplayEffect> DamageEffectClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameplayCue")
	FGameplayTag DamageCueTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tag")
	FGameplayTag SourceAbilityTag;
	
	UPROPERTY(ReplicatedUsing=OnRep_ActorActive)
	bool bActorActive = true;
	
	UPROPERTY(Transient)
	ADemoCharacter* OwnerCharacter = nullptr;
	
	UPROPERTY(Transient)
	APlayerController* OwnerPlayerCharacter = nullptr;
	
	FGameplayAbilitySpec SourceAbilitySpec;
	FVector AbilityConfirmedLocation;
	
	UFUNCTION()
	void OnRep_ActorActive();
	
	void CheckAndReplaceLocalPredictedActor();
};
