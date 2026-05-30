// Copyright Epic Games, Inc. All Rights Reserved.

#include "DemoGameMode.h"
#include "DemoGameInstance.h"
#include "DemoGameState.h"
#include "DemoPlayerState.h"
#include "Engine/AssetManager.h"
#include "Gameplay/Character/DemoCharacter.h"
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
	
	UE_LOG(LogDemoGameMode, Warning, TEXT("Player logged in. Current player count"), GetCurrentPlayerCount());
	if (!bHasSomeOneLoggedIn)
	{
		bHasSomeOneLoggedIn = true;
		GetWorld()->GetTimerManager().SetTimer(DelayStartCountDownTimerHandle, this, &ADemoGameModeRunning::OnDelayStartCountDownTimerReached, 
			StartCountDownDelayTime, false);
		
		if (ADemoGameState* DemoGameState = GetWorld()->GetGameState<ADemoGameState>())
		{
			int32* AICount = RoundIndexToAICountMap.Find(1);
			if (!AICount)
			{
				return;
			}
			
			DemoGameState->SetCurrentAICount(*AICount);
		}
	}
}

void ADemoGameModeRunning::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	
	// 这里将PlayerArray的元素数减一，因为PlayerArray的更新比GameMode的Logout时机要晚
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
	
	UE_LOG(LogDemoGameMode, Warning, TEXT("Current Player Count: %d"), DemoGameState->PlayerArray.Num());
	return DemoGameState->PlayerArray.Num();
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
		DemoGameState->SetCountDownEndTimeStamp(FDateTime::UtcNow().ToUnixTimestamp() + CountDownTotalTime);
	}
	
	GetWorld()->GetTimerManager().SetTimer(DelayStartNewRoundHandle, this, &ADemoGameModeRunning::OnCountDownTimerReached, CountDownTotalTime, false);
}

void ADemoGameModeRunning::OnCountDownTimerReached()
{
	StartNewRound();
}

void ADemoGameModeRunning::StartNewRound()
{
	GetWorld()->GetTimerManager().SetTimer(DelayShowRoundTextTimerHandle, this, &ADemoGameModeRunning::OnShowRoundTextTimerReached, DelayShowRoundTextTime, false);
}

void ADemoGameModeRunning::OnPlayerDefeatCountChanged()
{
	ADemoGameState* DemoGameState = GetWorld()->GetGameState<ADemoGameState>();
	if (!DemoGameState)
	{
		return;
	}
	
	// 按照击杀数进行排序
	TArray<TObjectPtr<APlayerState>> TempPlayerArray = DemoGameState->PlayerArray;
	TempPlayerArray.Sort([](const TObjectPtr<APlayerState>& A, const TObjectPtr<APlayerState>& B)
	{
		ADemoPlayerState* PlayerStateA = Cast<ADemoPlayerState>(A.Get());
		ADemoPlayerState* PlayerStateB = Cast<ADemoPlayerState>(B.Get());
		if (!PlayerStateA || !PlayerStateB)
		{
			return false;
		}
		
		return PlayerStateA->GetDefeatCount() > PlayerStateB->GetDefeatCount();
	});
	
	for (int32 i = 0; i < TempPlayerArray.Num(); ++i)
	{
		if (ADemoPlayerState* DemoPlayerState = Cast<ADemoPlayerState>(TempPlayerArray[i].Get()))
		{
			DemoPlayerState->SetRank(i + 1);
		}
	}
}

void ADemoGameModeRunning::OnPlayerDead()
{
	bool bHasSomeOneAlive = false;
	ADemoGameState* DemoGameState = Cast<ADemoGameState>(GetWorld()->GetGameState());
	if (!DemoGameState)
	{
		return;
	}
	
	for (auto PlayerState : DemoGameState->PlayerArray)
	{
		if (ADemoCharacter* DemoCharacter = PlayerState->GetPawn<ADemoCharacter>())
		{
			if (!DemoCharacter->IsDead())
			{
				bHasSomeOneAlive = true;
				break;
			}
		}
	}
	
	if (!bHasSomeOneAlive)
	{
		DemoGameState->MultiNotifyGameOver(false);
	}
}

void ADemoGameModeRunning::OnShowRoundTextTimerReached()
{
	++CurrentRound;
	int32* AICount = RoundIndexToAICountMap.Find(CurrentRound);
	if (!AICount)
	{
		if (ADemoGameState* DemoGameState = GetWorld()->GetGameState<ADemoGameState>())
		{
			DemoGameState->MultiNotifyGameOver(true);
		}
		
		return;
	}
	
	if (ADemoGameState* DemoGameState = Cast<ADemoGameState>(GetWorld()->GetGameState()))
	{
		DemoGameState->SetCurrentAICount(*AICount);
		DemoGameState->SetCurrentRoundIndex(CurrentRound);
	}
	
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindUObject(this, &ADemoGameModeRunning::OnDelaySpawnAITimerReached, *AICount);
	GetWorld()->GetTimerManager().SetTimer(DelaySpawnAITimerHandle, TimerDelegate, FadeoutAnimationTotalTime, false);
}

void ADemoGameModeRunning::OnDelaySpawnAITimerReached(const int32 InAICount) const
{
	if (AISpawner)
	{
		if (auto TempMesh = RoundIndexToAISkeletalMeshMap.Find(CurrentRound))
		{
			FSoftObjectPath TempMeshPath = (*TempMesh).ToSoftObjectPath();
			UAssetManager::GetStreamableManager().RequestAsyncLoad(TempMeshPath, 
				FStreamableDelegate::CreateWeakLambda(this, [this, InAICount, TempMeshPath]()
				{
					if (USkeletalMesh* TempSkeletalMesh = Cast<USkeletalMesh>(TempMeshPath.ResolveObject()))
					{
						AISpawner->SpawnSeveralAI(InAICount, TempSkeletalMesh);
					}
				}));
		}
	}
}