// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DemoCharacterGameplayAbility.h"
#include "ChaGA_Melee.generated.h"

/**
 * Melee ability that randomly chooses montage start section "1" or "2".
 */
UCLASS()
class UChaGA_Melee : public UDemoCharacterGameplayAbility
{
    GENERATED_BODY()

public:
    virtual bool PlayFireMontage() override;
};

