#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "DemoAICharacterSettings.generated.h"

class UDemoAICharacterGlobalConfig;

UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Demo AI Character Settings"))
class UDemoAICharacterSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "DataAssets")
	TSoftObjectPtr<UDemoAICharacterGlobalConfig> AICharacterGlobalConfig;
};
