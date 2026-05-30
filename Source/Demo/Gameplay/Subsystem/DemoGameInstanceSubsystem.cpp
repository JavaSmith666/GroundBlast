// Fill out your copyright notice in the Description page of Project Settings.

#include "Gameplay/Subsystem/DemoGameInstanceSubsystem.h"
#include "Gameplay/Core/DemoGameInstance.h"
#include "SocketSubsystem.h"
#include "Gameplay/Character/DemoAICharacter.h"
#include "Gameplay/Core/DemoGameState.h"
#include "Gameplay/Core/DemoPlayerController.h"
#include "Gameplay/Settings/DemoSpawnerSettings.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "Kismet/GameplayStatics.h"

void UDemoGameInstanceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	if (const UDemoSpawnerSettings* Settings = GetDefault<UDemoSpawnerSettings>())
	{
		AICharacterClass = Settings->AICharacterClass;
		PoolSize = Settings->PoolSize;
	}
}

void UDemoGameInstanceSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UDemoGameInstanceSubsystem::CreateNewRoom(FString InURL)
{
	ADemoPlayerController* PC = Cast<ADemoPlayerController>(GetWorld()->GetFirstPlayerController());
	if (!PC)
	{
		return;
	}
	
	FString IPv4Address = GetCurrentIPv4Address();
	FString FinalAddress = IPv4Address + InURL;
	PC->ClientTravel(FinalAddress, ETravelType::TRAVEL_Absolute);
}

FString UDemoGameInstanceSubsystem::GetCurrentIPv4Address()
{
	ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	if (!SocketSubsystem)
	{
		return FString();
	}
    
	TArray<TSharedPtr<FInternetAddr>> LocalAddresses;
	if (SocketSubsystem->GetLocalAdapterAddresses(LocalAddresses))
	{
		for (const TSharedPtr<FInternetAddr>& Addr : LocalAddresses)
		{
			if (Addr.IsValid())
			{
				FIPv4Address IPv4Address;
				uint32 RawIP = 0;
				Addr->GetIp(RawIP);
				
				// 筛选局域网地址（192.168.x.x, 10.x.x.x, 172.16-31.x.x）
				IPv4Address = FIPv4Address(RawIP);
				if (IPv4Address.IsSiteLocalAddress())
				{
					return IPv4Address.ToString();
				}
			}
		}
	}
	
	return FString();
}

void UDemoGameInstanceSubsystem::StartGame(FString InURL, bool bAbsolute, float LoadLevelDelay)
{
	if (GetWorld()->GetNetMode() == NM_Client)
	{
		if (ADemoGameState* GameState = Cast<ADemoGameState>(GetWorld()->GetGameState()))
		{
			GameState->SetHasGameStartedInRoom(true);
		}
		
		if (ADemoPlayerController* PC = Cast<ADemoPlayerController>(GetWorld()->GetFirstPlayerController()))
		{
			UE_LOG(LogNet, Warning, TEXT("PC->IsLocalPlayerController: %d"), PC->IsLocalPlayerController());
			UE_LOG(LogNet, Warning, TEXT("PC->GetNetConnection: %s"), PC->GetNetConnection() ? TEXT("Valid") : TEXT("NULL"));
			UE_LOG(LogNet, Warning, TEXT("PC->Role: %d"), (int32)PC->GetLocalRole());
			
			PC->ServerStartGameInRoom(InURL, bAbsolute, LoadLevelDelay);	
		}	
	}
	else
	{
		AuthStartGameInRoom(InURL, bAbsolute, LoadLevelDelay);
	}
}

void UDemoGameInstanceSubsystem::AuthStartGameInRoom(FString InURL, bool bAbsolute, float LoadLevelDelay)
{
	UE_LOG(LogNet, Warning, TEXT("AuthStartGameInRoom called - InURL: %s, Delay: %f"), *InURL, LoadLevelDelay);
	
	bool bIsStandalone = GetWorld()->GetNetMode() == NM_Standalone;
	FTimerHandle TimerHandle;
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindUObject(this, &UDemoGameInstanceSubsystem::OnLoadLevelDelayTimeReached, InURL, bAbsolute, bIsStandalone);
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, LoadLevelDelay, false);
}

void UDemoGameInstanceSubsystem::JoinRoom(FString InIPv4Address, FString InURL)
{
	UDemoGameInstance* GameInstance = Cast<UDemoGameInstance>(GetWorld()->GetGameInstance());
	if (!GameInstance)
	{
		return;
	}
	
	if (APlayerController* PC = GameInstance->GetFirstLocalPlayerController())
	{
		FString FinalAddress = InIPv4Address + InURL;
		PC->ClientTravel(FinalAddress, ETravelType::TRAVEL_Absolute);
	}
}

void UDemoGameInstanceSubsystem::OnLoadLevelDelayTimeReached(FString InURL, bool bAbsolute, bool bIsStandalone)
{
	UE_LOG(LogNet, Warning, TEXT("=== OnLoadLevelDelayTimeReached CALLED ==="));
	UE_LOG(LogNet, Warning, TEXT("    InURL: %s, bAbsolute: %d"), *InURL, bAbsolute);
	
	if (bIsStandalone)
	{
		UGameplayStatics::OpenLevel(GetWorld(), FName(*InURL), bAbsolute);
	}
	else
	{
		GetWorld()->ServerTravel(InURL, bAbsolute);
	}
}

void UDemoGameInstanceSubsystem::InitializeAIPool()
{
	if (!AICharacterClass)
	{
		return;
	}

	for (int32 i = 0; i < PoolSize; ++i)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ADemoAICharacter* NewAI = GetWorld()->SpawnActor<ADemoAICharacter>(AICharacterClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
		if (NewAI)
		{
			NewAI->Deactivate();
			AIPool.Add(NewAI);
		}
	}
}

void UDemoGameInstanceSubsystem::DeInitializeAIPool()
{
	for (ADemoAICharacter* AI : AIPool)
	{
		if (AI)
		{
			AI->Destroy(); 
		}
	}

	AIPool.Empty(); 
}

ADemoAICharacter* UDemoGameInstanceSubsystem::GetAICharacterFromPool(const FVector& Location, const FRotator& Rotation)
{
	for (ADemoAICharacter* AI : AIPool)
	{
		if (AI && !AI->IsActive())
		{
			AI->Activate(Location, Rotation);
			return AI;
		}
	}

	// 如果没有可用AI对象，则创建一个新的
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ADemoAICharacter* NewAI = GetWorld()->SpawnActor<ADemoAICharacter>(AICharacterClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
	if (NewAI)
	{
		NewAI->Activate(Location, Rotation);
		AIPool.Add(NewAI);
	}
	
	return NewAI;
}

void UDemoGameInstanceSubsystem::ReturnAICharacterToPool(ADemoAICharacter* CharacterToReturn)
{
	if (CharacterToReturn)
	{
		CharacterToReturn->Deactivate();
	}
}