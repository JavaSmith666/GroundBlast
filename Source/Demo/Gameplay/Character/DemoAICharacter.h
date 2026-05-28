// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Character/DemoCharacter.h"
#include "DemoAICharacter.generated.h"

class UBehaviorTree;
class UWidgetComponent;
class UAnimMontage;
class UDemoAICharacterGlobalConfig;

/**
 * 
 */
UCLASS()
class ADemoAICharacter : public ADemoCharacter
{
	GENERATED_BODY()
	
public:
	ADemoAICharacter();
	
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > &OutLifetimeProps) const override;
	virtual void CheckDeath(float InCurrentHP) override;
	void Activate(const FVector& Location, const FRotator& Rotation);
	void Deactivate();
	bool IsActive() const { return bIsActive; }
	UDemoAICharacterGlobalConfig* GetAICharacterGlobalConfig() const { return DemoAICharacterGlobalConfig; }
	
protected:
	UPROPERTY(EditAnywhere, Category = "AI")
	TSoftObjectPtr<UBehaviorTree> BehaviorTreeClass = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Montage")
	TSoftObjectPtr<UAnimMontage> SoftDeathMontage = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	UWidgetComponent* HPBar = nullptr;
	
	UPROPERTY(Transient)
	UDemoAICharacterGlobalConfig* DemoAICharacterGlobalConfig = nullptr;
	
	UFUNCTION()
	void OnRep_bIsActive();
	
	void OnAICharacterGlobalConfigLoaded();
	void InitializeAICharacterGlobalConfig();
	void TryGrantSkills();

private:
	UPROPERTY(ReplicatedUsing=OnRep_bIsActive)
	bool bIsActive = false;
};
