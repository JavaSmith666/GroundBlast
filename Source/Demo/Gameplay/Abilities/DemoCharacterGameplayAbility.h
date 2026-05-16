// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "DataAssets/SkillConfig.h"
#include "Gameplay/AttributeSet/BaseAttributeSet.h"
#include "DemoCharacterGameplayAbility.generated.h"

class UAnimMontage;
class UAbilityTask_PlayMontageAndWait;
class ADemoCharacter;

DECLARE_LOG_CATEGORY_EXTERN(LogDemoCharacterGameplayAbility, Log, All);

/**
 * Minimal gameplay ability for Demo characters.
 * This class is intentionally small — extend it with ability logic as needed.
 */
UCLASS()
class UDemoCharacterGameplayAbility : public UGameplayAbility
{
    GENERATED_BODY()

public:
    UDemoCharacterGameplayAbility();
    
    /** Called when the ability is given to an AbilitySystemComponent */
    virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
    
    /** Public setter for skill configuration. */
    void SetRoleSkillConfig(USkillConfig* InConfig) { RoleSkillConfig = InConfig; }
    
    ADemoCharacter* GetOwnerCharacter() const { return OwnerCharacter; }
    
    void SetSummonItem(ASummonItemBase* InSummonItem) { SummonItem = InSummonItem; }
    ASummonItemBase* GetSummonItem() const { return SummonItem; }
    
    UFUNCTION(BlueprintCallable)
    int32 GetAbilityIndex() const { return AbilityIndex; }
    
    virtual bool PlayFireMontage();
    
    bool HasAbilityBeenConfirmed() { return bHasConfirmed; }

protected:
    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;
    
    virtual bool OnDemoCharacterGameplayAbilityActivated(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData);

    virtual void EndAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        bool bReplicateEndAbility,
        bool bWasCancelled) override;
    
    virtual void PreDemoCharacterGameplayAbilityEnded(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        bool bReplicateEndAbility,
        bool bWasCancelled);

    UFUNCTION()
    void OnFireMontageCompleted();

    UFUNCTION()
    void OnFireMontageBlendOut();

    UFUNCTION()
    void OnFireMontageInterrupted();

    UFUNCTION()
    void OnFireMontageCancelled();
    
    UFUNCTION()
    void OnMainUICreated();

protected:
    UPROPERTY(Transient)
    UAbilityTask_PlayMontageAndWait* ActiveMontageTask = nullptr;
    
    UPROPERTY(Transient)
    USkillConfig* RoleSkillConfig = nullptr;
    
    int32 AbilityIndex = -1;
    
    UPROPERTY(Transient)
    ADemoCharacter* OwnerCharacter = nullptr;
    
    UPROPERTY(Transient)
    APlayerController* OwnerPlayerController = nullptr;
    
    UPROPERTY(Transient)
    ASummonItemBase* SummonItem = nullptr;
    
    bool bHasConfirmed = false;
    
private:
    void HandleFireMontageEnded(bool bWasCancelled);
};
