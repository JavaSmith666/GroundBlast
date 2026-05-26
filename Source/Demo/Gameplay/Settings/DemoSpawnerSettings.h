// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "DemoSpawnerSettings.generated.h"

class ADemoAICharacter;

/**
 * 
 */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Demo Spawner Settings"))
class UDemoSpawnerSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UDemoSpawnerSettings();
	
	UPROPERTY(Config, EditAnywhere, Category = "ObjectPool")
	TSubclassOf<ADemoAICharacter> AICharacterClass;
	
	UPROPERTY(Config, EditAnywhere, Category = "ObjectPool")
	int32 PoolSize = 50;
};
