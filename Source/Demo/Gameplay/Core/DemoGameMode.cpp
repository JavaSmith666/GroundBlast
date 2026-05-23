// Copyright Epic Games, Inc. All Rights Reserved.

#include "DemoGameMode.h"
#include "DemoGameInstance.h"
#include "DemoGameState.h"

DEFINE_LOG_CATEGORY(LogDemoGameMode);

ADemoGameMode::ADemoGameMode()
{
}

void ADemoGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
}

void ADemoGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	
	if (GetCurrentPlayerCount() <= 1)
	{
		ReturnRoomLevel();
	}
}

int32 ADemoGameMode::GetCurrentPlayerCount() const
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

void ADemoGameMode::ReturnRoomLevel()
{
	UDemoGameInstance* DemoGameInstance = Cast<UDemoGameInstance>(GetWorld()->GetGameInstance());
	if (!DemoGameInstance)
	{
		return;
	}
	
	FString Ipv4Address = DemoGameInstance->GetCurrentIPv4Address();
	FString FinalAddress = Ipv4Address + RoomLevelAbsoluteURL;
	GetWorld()->ServerTravel(FinalAddress, true);
	
	UE_LOG(LogDemoGameMode, Warning, TEXT("=== Returning to room level ==="));
	UE_LOG(LogDemoGameMode, Warning, TEXT("    FinalAddress: %s"), *FinalAddress);
}
