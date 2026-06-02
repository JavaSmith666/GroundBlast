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
	
	int32 GetCurrentRound() const { return CurrentRound; }
	
	void StartNewRound();
	void OnPlayerDefeatCountChanged();
	void OnPlayerDead();
	
protected:
	UPROPERTY(EditAnywhere, Category = "RoomLevel")
	FString RoomLevelAbsoluteURL;
	
	UPROPERTY(EditAnywhere)
	float StartCountDownDelayTime = 3.f;
	
	UPROPERTY(EditAnywhere)
	int32 CountDownTotalTime = 10;
	
	UPROPERTY(EditAnywhere)
	float FadeoutAnimationTotalTime = 3.5f;
	
	UPROPERTY(EditAnywhere)
	float DelayShowRoundTextTime = 1.f;
	
	UPROPERTY(EditAnywhere, Category = "Spawner")
	FTransform SpawnTransform;
	
	UPROPERTY(EditAnywhere, Category = "Spawner")
	TSoftClassPtr<ADemoAISpawner> AISpawnerClass;
	
	UPROPERTY(EditAnywhere, Category = "Spawner")
	TMap<int32, int32> RoundIndexToAICountMap;
	
	bool bHasSomeOneLoggedIn = false;
	FTimerHandle DelayStartCountDownTimerHandle;
	int32 CurrentRound = 0;
	FTimerHandle DelayStartNewRoundHandle;
	FTimerHandle DelaySpawnAITimerHandle;
	FTimerHandle DelayShowRoundTextTimerHandle;
	
	UPROPERTY(Transient)
	ADemoAISpawner* AISpawner = nullptr;
	
protected:
	int32 GetCurrentPlayerCount() const;
	void ReturnRoomLevel();
	
	UFUNCTION()
	void OnDelayStartCountDownTimerReached();
	
	UFUNCTION()
	void OnCountDownTimerReached();
	
	UFUNCTION()
	void OnShowRoundTextTimerReached();
	
	UFUNCTION()
	void OnDelaySpawnAITimerReached(const int32 InAICount) const;
};

