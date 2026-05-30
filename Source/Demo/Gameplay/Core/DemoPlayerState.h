// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "DemoPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class ADemoPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;
	
	void OnEnemyDefeated();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText NickName;
	
protected:
	UPROPERTY(ReplicatedUsing=OnRep_DefeatCount)
	int32 DefeatCount = 0;
	
	UFUNCTION()
	void OnRep_DefeatCount();
};
