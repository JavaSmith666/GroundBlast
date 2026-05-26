// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Character/DemoCharacter.h"
#include "DemoAICharacter.generated.h"

class UBehaviorTree;
class UWidgetComponent;

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
	void Activate(const FVector& Location, const FRotator& Rotation);
	void Deactivate();
	bool IsActive() const { return bIsActive; }
	
protected:
	UPROPERTY(EditAnywhere, Category = "AI")
	TSubclassOf<UBehaviorTree> BehaviorTreeClass = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	UWidgetComponent* HPBar = nullptr;
	
	UFUNCTION()
	void OnRep_bIsActive();

private:
	UPROPERTY(ReplicatedUsing=OnRep_bIsActive)
	bool bIsActive = false;
};
