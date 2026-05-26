// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DemoGameMode.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDemoGameMode, Log, All);

class ADemoAISpawner;

/**
 *  Simple GameMode for a third person game
 */
UCLASS(abstract)
class ADemoGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ADemoGameMode();
};

UCLASS()
class ADemoGameModeStarting : public ADemoGameMode
{
	GENERATED_BODY()

public:
	ADemoGameModeStarting();
};

UCLASS()
class ADemoGameModeRunning : public ADemoGameMode
{
	GENERATED_BODY()

public:
	ADemoGameModeRunning();
	
	virtual void StartPlay() override;
	virtual void StartToLeaveMap() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	
protected:
	UPROPERTY(EditAnywhere, Category = "RoomLevel")
	FString RoomLevelAbsoluteURL;
	
	UPROPERTY(EditAnywhere)
	float StartCountDownDelayTime = 3.f;
	
	UPROPERTY(EditAnywhere)
	float CountDownTotalTime = 10.f;
	
	UPROPERTY(EditAnywhere)
	float FadeoutAnimationTotalTime = 3.5f;
	
	UPROPERTY(EditAnywhere, Category = "Spawner")
	FTransform SpawnTransform;
	
	UPROPERTY(EditAnywhere, Category = "Spawner")
	TSoftClassPtr<ADemoAISpawner> AISpawnerClass;
	
	UPROPERTY(EditAnywhere, Category = "Spawner")
	TMap<int32, int32> RoundIndexToAICountMap;
	
	bool bHasSomeOneLoggedIn = false;
	FTimerHandle DelayStartCountDownTimerHandle;
	int32 CurrentRound = 1;
	FTimerHandle DelayStartFirstRoundHandle;
	FTimerHandle DelaySpawnAITimerHandle;
	
	UPROPERTY(Transient)
	ADemoAISpawner* AISpawner = nullptr;
	
protected:
	int32 GetCurrentPlayerCount() const;
	void ReturnRoomLevel();
	
	UFUNCTION()
	void OnDelayStartCountDownTimerReached();
	
	UFUNCTION()
	void OnDelayStartFirstRoundTimerReached();
	
	UFUNCTION()
	void OnDelaySpanwAITimerReached(int32 AICount);
	
	void StartNewRound();
};

