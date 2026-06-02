#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "DemoAICharacterGlobalConfig.generated.h"

class UGameplayEffect;
class UAnimMontage;
class USkillConfig;
class UGameplayAbility;

UCLASS(BlueprintType)
class UDemoAICharacterGlobalConfig : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> DeathMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayAbility> AbilityClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagContainer AbilityTags;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<USkillConfig> SkillConfig;
	
	UPROPERTY(EditAnywhere, Category = "BehaviorTree")
    TObjectPtr<UBehaviorTree> BehaviorTreeClass = nullptr;
};