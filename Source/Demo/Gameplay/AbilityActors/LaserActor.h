// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "SummonItemBase.h"
#include "GameFramework/Actor.h"
#include "LaserActor.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogLaserActor, Log, All);

class ADemoCharacter;
class UGameplayEffect;

UCLASS()
class ALaserActor : public ASummonItemBase
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALaserActor();
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	void ClearCurrentHitCharacterDamageEffect();
	
public:	
	UPROPERTY(EditAnywhere)
	float LaserTraceMaxDistance = 10000.f;
	
	UPROPERTY(EditAnywhere)
	TSoftClassPtr<UGameplayEffect> LaserDamageEffect;
	
	UPROPERTY(Transient)
	ADemoCharacter* CurrentHitCharacter = nullptr;
	
protected:
	
	FActiveGameplayEffectHandle CurrentHitCharacterDamageEffectHandle;
};
