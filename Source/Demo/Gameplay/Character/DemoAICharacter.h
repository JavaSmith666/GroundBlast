// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Character/DemoCharacter.h"
#include "DemoAICharacter.generated.h"

class UBehaviorTree;
class UWidgetComponent;
class UAnimMontage;
class UDemoAICharacterGlobalConfig;
class AAIController;

DECLARE_LOG_CATEGORY_EXTERN(LogDemoAICharacter, Log, All);

struct FAutoActivateParameters
{
	bool bAutoActivateOnAssetLoaded = false;
	FVector Location = FVector::ZeroVector;
	FRotator Rotation = FRotator::ZeroRotator;
};

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
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > &OutLifetimeProps) const override;
	virtual void CheckDeath(float InCurrentHP) override;
	void Activate(const FVector& Location, const FRotator& Rotation);
	void Deactivate();
	bool IsActive() const { return bIsActive; }
	void SetAutoActivateParameters(const FAutoActivateParameters& InAutoActivateParameters) { AutoActivateParameters = InAutoActivateParameters; }
	UDemoAICharacterGlobalConfig* GetAICharacterGlobalConfig() const { return DemoAICharacterGlobalConfig; }
	
protected:
	UPROPERTY(EditAnywhere, Category = "Mesh")
	TMap<int32, TSoftObjectPtr<USkeletalMesh>> RoundIndexToAISkeletalMeshMap;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	UWidgetComponent* HPBar = nullptr;
	
	UPROPERTY(Transient)
	UDemoAICharacterGlobalConfig* DemoAICharacterGlobalConfig = nullptr;
	
	UPROPERTY(Transient)
	AAIController* CachedAIController = nullptr;
	
	UFUNCTION()
	void OnRep_bIsActive();
	
	void OnAICharacterGlobalConfigLoaded();
	void InitializeAICharacterGlobalConfig();
	void TryGrantSkills();
	void ChangeSkeletalMesh();
	
	FAutoActivateParameters AutoActivateParameters;

private:	
	UPROPERTY(ReplicatedUsing=OnRep_bIsActive)
	bool bIsActive = false;
};
