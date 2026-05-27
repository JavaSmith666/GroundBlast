// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Character/DemoAICharacter.h"
#include "Gameplay/Abilities/DemoAbilitySystemComponent.h"
#include "Gameplay/AttributeSet/BaseAttributeSet.h"
#include "Runtime/AIModule/Classes/AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"

ADemoAICharacter::ADemoAICharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	
	HPBar = CreateDefaultSubobject<UWidgetComponent>(TEXT("HPBar"));
	HPBar->SetupAttachment(RootComponent);
	HPBar->SetVisibility(false);
}

void ADemoAICharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	}
}

void ADemoAICharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ADemoAICharacter, bIsActive);
}

void ADemoAICharacter::Activate(const FVector& Location, const FRotator& Rotation)
{
	if (bIsActive || GetNetMode() == NM_Client)
	{
		return;
	}

	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
    
	SetActorLocation(Location);
	SetActorRotation(Rotation);

	if (UAbilitySystemComponent* ASC = FindComponentByClass<UAbilitySystemComponent>())
	{
		if (UBaseAttributeSet* AttributeSet = const_cast<UBaseAttributeSet*>(ASC->GetSet<UBaseAttributeSet>()))
		{
			AttributeSet->SetHP(AttributeSet->GetMaxHP());
		}
	}
	
	// 开启AI行为树
	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		if (UBehaviorTree* TempBehaviorTree = BehaviorTreeClass.IsNull() ? nullptr : BehaviorTreeClass.LoadSynchronous())
		{
			AIController->RunBehaviorTree(TempBehaviorTree);	
		}
	}
	
	bIsActive = true;
	if (GetNetMode() == NM_Standalone && HPBar)
	{
		HPBar->SetVisibility(bIsActive);
	}
}

void ADemoAICharacter::Deactivate()
{
	if (!bIsActive || GetNetMode() == NM_Client)
	{
		return;
	}

	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	
	bIsActive = false;
	if (GetNetMode() == NM_Standalone && HPBar)
	{
		HPBar->SetVisibility(bIsActive);
	}
}

void ADemoAICharacter::OnRep_bIsActive()
{
	if (HPBar)
	{
		HPBar->SetVisibility(bIsActive);
	}
}
