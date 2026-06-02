// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "AnimInst_Shinbi.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAnimInst_Shinbi, Log, All);

class ADemoCharacter;
class UGameplayEffect;

/**
 * 
 */
UCLASS()
class UAnimInst_Shinbi : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	virtual void NativeInitializeAnimation() override;

	virtual void NativeUpdateAnimation(float DeltaTime) override;
	
	void OnStartMontageNotify();
	void OnEndMontageNotify();
	
protected:
	UPROPERTY(BlueprintReadWrite, Category = "Animation")
	bool bNeedMontage = false;
	
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Animation")
	ADemoCharacter* OwnerCharacter = nullptr;
	
	UPROPERTY(BlueprintReadWrite, Category = "Animation")
	float Speed = 0.f;
	
	UPROPERTY(BlueprintReadWrite, Category = "Animation")
	float Angle = 0.f;
	
	UPROPERTY(BlueprintReadWrite, Category = "Animation")
	bool bInAir = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSoftClassPtr<UGameplayEffect> MeleeDamageEffect;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	float AttackCapsuleRadius  = 80.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	float AttackCapsuleHalfHeight  = 30.0f;
};
