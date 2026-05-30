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
	int32 GetDefeatCount() const { return DefeatCount; }
	void SetRank(int32 NewRank) { Rank = NewRank; }
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText NickName;
	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_DefeatCount)
	int32 DefeatCount = 0;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
	int32 Rank = 1;
	
	UFUNCTION()
	void OnRep_DefeatCount();
};
