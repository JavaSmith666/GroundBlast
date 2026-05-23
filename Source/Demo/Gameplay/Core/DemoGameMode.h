// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DemoGameMode.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDemoGameMode, Log, All);

/**
 *  Simple GameMode for a third person game
 */
UCLASS(abstract)
class ADemoGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	
	/** Constructor */
	ADemoGameMode();
	
	virtual void PostLogin(APlayerController* NewPlayer) override;
	
	virtual void Logout(AController* Exiting) override;
	
protected:
	UPROPERTY(EditAnywhere, Category = "RoomLevel")
	FString RoomLevelAbsoluteURL;
	
	int32 GetCurrentPlayerCount() const;
	
	void ReturnRoomLevel();
};



