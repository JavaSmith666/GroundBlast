// Copyright Epic Games, Inc. All Rights Reserved.

#include "DemoPlayerController.h"
#include "DemoGameInstance.h"
#include "DemoGameState.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Gameplay/Character/DemoCharacter.h"
#include "Gameplay/Subsystem/DemoGameInstanceSubsystem.h"

void ADemoPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}
		}
	}
}

void ADemoPlayerController::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);
}

void ADemoPlayerController::SetOwner(AActor* NewOwner)
{
	Super::SetOwner(NewOwner);
}

void ADemoPlayerController::ServerStartGameInRoom_Implementation(const FString& InURL, bool bAbsolute, float LoadLevelDelay)
{
	UE_LOG(LogNet, Warning, TEXT("=== ServerStartGameInRoom_Implementation CALLED on server ==="));
	UE_LOG(LogNet, Warning, TEXT("    InURL: %s"), *InURL);
	UE_LOG(LogNet, Warning, TEXT("    bAbsolute: %d"), bAbsolute);
	UE_LOG(LogNet, Warning, TEXT("    LoadLevelDelay: %f"), LoadLevelDelay);
	
	if (UDemoGameInstance* DemoGameInstance = Cast<UDemoGameInstance>(GetWorld()->GetGameInstance()))
	{
		if (UDemoGameInstanceSubsystem* DemoGameInstanceSubsystem = DemoGameInstance->GetSubsystem<UDemoGameInstanceSubsystem>())
		{
			DemoGameInstanceSubsystem->AuthStartGameInRoom(InURL, bAbsolute, LoadLevelDelay);
		}
	}
	
	// 通知UI
	if (ADemoGameState* DemoGameState = Cast<ADemoGameState>(GetWorld()->GetGameState()))
	{
		DemoGameState->MultiNotifyGameStartedInRoom();
	}
	
	// 开启重力
	if (ADemoCharacter* DemoCharacter = Cast<ADemoCharacter>(GetOwner()))
	{
		if (UCharacterMovementComponent* CharacterMovementComponent = DemoCharacter->GetCharacterMovement())
		{
			CharacterMovementComponent->GravityScale = 1.f;
		}
	}
}

bool ADemoPlayerController::ServerStartGameInRoom_Validate(const FString& InURL, bool bAbsolute, float LoadLevelDelay)
{
	return true;
}
