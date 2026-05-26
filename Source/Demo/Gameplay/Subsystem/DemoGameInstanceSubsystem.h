// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DemoGameInstanceSubsystem.generated.h"

class ADemoAICharacter;

/**
 * 
 */
UCLASS()
class UDemoGameInstanceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	virtual void Deinitialize() override;
	
	UFUNCTION(BlueprintCallable)
	void CreateNewRoom(FString InURL);
	
	UFUNCTION(BlueprintPure)
	static FString GetCurrentIPv4Address();
	
	UFUNCTION(BlueprintCallable)
	void StartGame(FString InURL, bool bAbsolute, float LoadLevelDelay);
	
	void AuthStartGameInRoom(FString InURL, bool bAbsolute, float LoadLevelDelay);
	
	UFUNCTION(BlueprintCallable)
	void JoinRoom(FString InIPv4Address, FString InURL);
	
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	void InitializeAIPool();
	
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	void DeInitializeAIPool();

	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	ADemoAICharacter* GetAICharacterFromPool(const FVector& Location, const FRotator& Rotation);

	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	void ReturnAICharacterToPool(ADemoAICharacter* CharacterToReturn);
	
protected:
	UFUNCTION()
	void OnLoadLevelDelayTimeReached(FString InURL, bool bAbsolute, bool bIsStandalone);

	UPROPERTY(Transient)
	TArray<ADemoAICharacter*> AIPool;

	UPROPERTY(Transient)
	TSubclassOf<ADemoAICharacter> AICharacterClass = nullptr;

	int32 PoolSize = 50;
};
