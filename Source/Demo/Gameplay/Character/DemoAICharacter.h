// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Character/DemoCharacter.h"
#include "DemoAICharacter.generated.h"

/**
 * 
 */
UCLASS()
class ADemoAICharacter : public ADemoCharacter
{
	GENERATED_BODY()
	
public:
	ADemoAICharacter();
	
	virtual void BeginPlay() override;
};
