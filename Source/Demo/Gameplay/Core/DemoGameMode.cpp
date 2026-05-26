// Copyright Epic Games, Inc. All Rights Reserved.

#include "DemoGameMode.h"
#include "DemoGameInstance.h"
#include "DemoGameState.h"
#include "Gameplay/Spawner/DemoAISpawner.h"
#include "Gameplay/Subsystem/DemoGameInstanceSubsystem.h"

DEFINE_LOG_CATEGORY(LogDemoGameMode);

ADemoGameMode::ADemoGameMode()
{
}

ADemoGameModeStarting::ADemoGameModeStarting()
{
}

ADemoGameModeRunning::ADemoGameModeRunning()
{
}

void ADemoGameModeRunning::StartPlay()
{
	Super::StartPlay();
	
	if (UDemoGameInstance* GI = Cast<UDemoGameInstance>(GetWorld()->GetGameInstance()))
	{
		if (UDemoGameInstanceSubsystem* GIS = GI->GetSubsystem<UDemoGameInstanceSubsystem>())
		{
			GIS->InitializeAIPool();
		}
	}	
	
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	if (UClass* SpawnerClass = AISpawnerClass.IsNull() ? nullptr : AISpawnerClass.LoadSynchronous())
	{
		AISpawner = GetWorld()->SpawnActor<ADemoAISpawner>(SpawnerClass, SpawnTransform, SpawnParameters);
	}
	
	if (AISpawner)
	{
		UE_LOG(LogDemoGameMode, Warning, TEXT("Successfully spawned AISpawner at location: %s"), *SpawnTransform.GetLocation().ToString());
	}
	else
	{
		UE_LOG(LogDemoGameMode, Error, TEXT("Failed to spawn AISpawner!"));
	}
}

void ADemoGameModeRunning::StartToLeaveMap()
{
	if (UDemoGameInstance* GI = Cast<UDemoGameInstance>(GetWorld()->GetGameInstance()))
	{
		if (UDemoGameInstanceSubsystem* GIS = GI->GetSubsystem<UDemoGameInstanceSubsystem>())
		{
			GIS->DeInitializeAIPool();
		}
	}
	
	UE_LOG(LogDemoGameMode, Warning, TEXT("StartToLeaveMap..."));
	Super::StartToLeaveMap();
}

void ADemoGameModeRunning::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
	if (!bHasSomeOneLoggedIn)
	{
		bHasSomeOneLoggedIn = true;
		GetWorld()->GetTimerManager().SetTimer(DelayStartCountDownTimerHandle, this, &ADemoGameModeRunning::OnDelayStartCountDownTimerReached, 
			StartCountDownDelayTime, false);
	}
}

void ADemoGameModeRunning::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	
	if (GetCurrentPlayerCount() <= 1)
	{
		ReturnRoomLevel();
	}
}

int32 ADemoGameModeRunning::GetCurrentPlayerCount() const
{
	ADemoGameState* DemoGameState = Cast<ADemoGameState>(GetWorld()->GetGameState());
	if (!DemoGameState)
	{
		return 0;
	}
	
	// 这里将PlayerArray的元素数减一，因为PlayerArray的更新比GameMode的Logout时机要晚
	UE_LOG(LogDemoGameMode, Warning, TEXT("Current Player Count: %d"), DemoGameState->PlayerArray.Num() - 1);
	return DemoGameState->PlayerArray.Num() - 1;
}

void ADemoGameModeRunning::ReturnRoomLevel()
{
	UDemoGameInstance* DemoGameInstance = Cast<UDemoGameInstance>(GetWorld()->GetGameInstance());
	if (!DemoGameInstance)
	{
		return;
	}
	
	UDemoGameInstanceSubsystem* DemoGameInstanceSubsystem = DemoGameInstance->GetSubsystem<UDemoGameInstanceSubsystem>();
	if (!DemoGameInstanceSubsystem)
	{
		return;
	}
	
	FString Ipv4Address = DemoGameInstanceSubsystem->GetCurrentIPv4Address();
	FString FinalAddress = Ipv4Address + RoomLevelAbsoluteURL;
	GetWorld()->ServerTravel(FinalAddress, true);
	
	UE_LOG(LogDemoGameMode, Warning, TEXT("=== Returning to room level ==="));
	UE_LOG(LogDemoGameMode, Warning, TEXT("    FinalAddress: %s"), *FinalAddress);
}

void ADemoGameModeRunning::OnDelayStartCountDownTimerReached()
{
	if (ADemoGameState* DemoGameState = Cast<ADemoGameState>(GetWorld()->GetGameState()))
	{
		DemoGameState->MultiNotifyStartCountDownText();
	}
	
	GetWorld()->GetTimerManager().SetTimer(DelayStartFirstRoundHandle, this, &ADemoGameModeRunning::OnDelayStartFirstRoundTimerReached, 
		CountDownTotalTime, false);
}

void ADemoGameModeRunning::OnDelayStartFirstRoundTimerReached()
{
	StartNewRound();
}

void ADemoGameModeRunning::StartNewRound()
{
	int32* AICount = RoundIndexToAICountMap.Find(CurrentRound);
	if (!AICount)
	{
		return;
	}
	
	if (ADemoGameState* DemoGameState = Cast<ADemoGameState>(GetWorld()->GetGameState()))
	{
		DemoGameState->MultiNotifyStartNewRound(CurrentRound, *AICount);
	}
	
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindUObject(this, &ADemoGameModeRunning::OnDelaySpanwAITimerReached, *AICount);
	GetWorld()->GetTimerManager().SetTimer(DelaySpawnAITimerHandle, TimerDelegate, FadeoutAnimationTotalTime + 1.f, false);
}

void ADemoGameModeRunning::OnDelaySpanwAITimerReached(int32 AICount)
{
	if (AISpawner)
	{
		AISpawner->SpawnSeveralAI(AICount);
	}
}