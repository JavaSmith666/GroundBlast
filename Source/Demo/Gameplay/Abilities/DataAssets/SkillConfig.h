#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Engine/DataAsset.h"
#include "SkillConfig.generated.h"

class UGameplayEffect;
class ASummonItemBase;

/**
 * Base data asset for all skill configurations.
 */
UCLASS(BlueprintType)
class USkillConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "Info")
	int32 AbilityIndex = -1;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SummonItem")
	TSubclassOf<ASummonItemBase> SummonItemClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Material")
	TObjectPtr<UMaterialInstance> AbilityMaterialInstance;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> FireMontage;
};

UCLASS(BlueprintType)
class UMeleeConfig : public USkillConfig
{
	GENERATED_BODY()

public:
};

UCLASS(BlueprintType)
class UHealthRegenConfig : public USkillConfig
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameplayEffect")
	TSubclassOf<UGameplayEffect> HealthRegenEffect;
};

UCLASS(BlueprintType)
class UDashConfig : public USkillConfig
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ForwardImpulse = 1000.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameplayEffect")
	TSubclassOf<UGameplayEffect> DashDamageEffect;
};

UCLASS(BlueprintType)
class ULaserConfig : public USkillConfig
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> LaserDamageEffect;
	
	UPROPERTY(editAnywhere, BlueprintReadOnly)
	FGameplayTag LaserCostTag;
};

UCLASS(BlueprintType)
class UGroundBlastConfig : public USkillConfig
{
	GENERATED_BODY()
	
public:
};

UCLASS(BlueprintType)
class UFireBlastConfig : public USkillConfig
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag FireBlastPullEventTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag FireBlastPushEventTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> FireBlastDamageEffect;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GroundSelect")
	float SelectRadius = 1000.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Impulse")
	float ImpulseValue = 1000.f;
};