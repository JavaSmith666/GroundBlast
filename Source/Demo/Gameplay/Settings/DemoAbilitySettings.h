#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "DemoAbilitySettings.generated.h"

UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Demo Ability Settings"))
class UDemoAbilitySettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UDemoAbilitySettings();

	/** Global Data Table for character skill slots. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "DataTables")
	TSoftObjectPtr<UDataTable> CharacterSkillTable;
};
