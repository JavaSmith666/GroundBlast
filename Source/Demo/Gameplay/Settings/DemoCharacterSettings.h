#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "DemoCharacterSettings.generated.h"

class UDemoCharacterGlobalConfig;

UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Demo Character Settings"))
class UDemoCharacterSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "DataAssets")
	TSoftObjectPtr<UDemoCharacterGlobalConfig> CharacterGlobalConfig;
};
