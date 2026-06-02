// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DemoAISpawner.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDemoAISpawner, Log, All);

UCLASS()
class ADemoAISpawner : public AActor
{
	GENERATED_BODY()
	
public:
	ADemoAISpawner();
	
	UFUNCTION(BlueprintCallable)
	void SpawnSeveralAI(int32 InCount);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
	float SpawnInterval = 3.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
	float SpawnRange = 400.0f;

private:
	int32 CurrentSpawnCount = 0;
	
	FTimerHandle SpawnTimerHandle;
	
	void SpawnSingleAI(int32 InMaxCount);
};
