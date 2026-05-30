// Fill out your copyright notice in the Description page of Project Settings.

#include "Gameplay/Character/DemoAICharacter.h"
#include "DemoAICharacterGlobalConfig.h"
#include "Gameplay/Abilities/DemoAbilitySystemComponent.h"
#include "Gameplay/AttributeSet/BaseAttributeSet.h"
#include "Runtime/AIModule/Classes/AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/AssetManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Gameplay/Settings/DemoAICharacterSettings.h"
#include "Gameplay/Abilities/DataAssets/SkillConfig.h"
#include "Gameplay/Core/DemoGameState.h"
#include "Gameplay/Core/DemoPlayerState.h"

ADemoAICharacter::ADemoAICharacter()
{
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
	
	InitializeAICharacterGlobalConfig();
}

void ADemoAICharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ADemoAICharacter, bIsActive);
}

void ADemoAICharacter::CheckDeath(float InCurrentHP)
{
	if (!bIsDead && InCurrentHP <= 0.f && DemoCharacterGlobalConfig)
	{
		bIsDead = true;
		
		if (AbilitySystemComponent)
		{
			if (ADemoGameState* GameState = GetWorld()->GetGameState<ADemoGameState>())
			{
				GameState->SetCurrentAICount(GameState->GetCurrentAICount() - 1);
			}
			
			const UBaseAttributeSet* AttributeSet = Cast<UBaseAttributeSet>(AbilitySystemComponent->GetAttributeSet(UBaseAttributeSet::StaticClass()));
			if (ADemoCharacter* TempInstigator = Cast<ADemoCharacter>(AttributeSet->GetLastInstigator()))	
			{
				if (ADemoPlayerState* TempPlayerState = TempInstigator->GetPlayerState<ADemoPlayerState>())
				{
					TempPlayerState->OnEnemyDefeated();
				}
			}
		}
		
		if (!DemoAICharacterGlobalConfig || !DemoAICharacterGlobalConfig->DeathMontage)
		{
			return;
		}
		
		if (GetNetMode() != NM_DedicatedServer)
		{
			// 本地预测播放蒙太奇
			if (UAnimInstance* AnimInst = GetMesh()->GetAnimInstance())
			{
				AnimInst->Montage_Play(DemoAICharacterGlobalConfig->DeathMontage);
			}
		}
		else
		{
			// 多播RPC让模拟端播蒙太奇
			MultiPlayMontage(DemoAICharacterGlobalConfig->DeathMontage);	
		}
		
		if (IsLocallyControlled())
		{
			if (APlayerController* PC = GetPlayerController())
			{
				PC->DisableInput(PC);
			}
		}
	}
}

void ADemoAICharacter::Activate(const FVector& Location, const FRotator& Rotation)
{
	if (bIsActive || GetNetMode() == NM_Client)
	{
		return;
	}

	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	SetActorTickEnabled(true);
    
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
	AAIController* AIController = Cast<AAIController>(GetController());
	if (!AIController)
	{
		AIController = CachedAIController;
	}
	
	if (AIController)
	{
		AIController->Possess(this);
		if (UBehaviorTree* TempBehaviorTree = BehaviorTreeClass.IsNull() ? nullptr : BehaviorTreeClass.LoadSynchronous())
		{
			AIController->RunBehaviorTree(TempBehaviorTree);
		}
	}
	
	bIsActive = true;
	bIsDead = false;
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
	SetActorTickEnabled(false);
	
	// 关闭AI行为树
	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		CachedAIController = AIController;
		AIController->UnPossess();
	}
	
	bIsActive = false;
	bIsDead = true;
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

void ADemoAICharacter::OnAICharacterGlobalConfigLoaded()
{
	const UDemoAICharacterSettings* Settings = GetDefault<UDemoAICharacterSettings>();
	if (!DemoAICharacterGlobalConfig && Settings && !Settings->AICharacterGlobalConfig.IsNull())
	{
		DemoAICharacterGlobalConfig = Settings->AICharacterGlobalConfig.Get();
		TryGrantSkills();
	}
}

void ADemoAICharacter::InitializeAICharacterGlobalConfig()
{
	const UDemoAICharacterSettings* Settings = GetDefault<UDemoAICharacterSettings>();
	if (!DemoAICharacterGlobalConfig && Settings && !Settings->AICharacterGlobalConfig.IsNull())
	{
		UAssetManager::GetStreamableManager().RequestAsyncLoad(Settings->AICharacterGlobalConfig.ToSoftObjectPath(), 
			FStreamableDelegate::CreateUObject(this, &ADemoAICharacter::OnAICharacterGlobalConfigLoaded));
	}
}

void ADemoAICharacter::TryGrantSkills()
{
	if (!DemoAICharacterGlobalConfig || !AbilitySystemComponent)
	{
		return;
	}
	
	FGameplayAbilitySpec Spec(DemoAICharacterGlobalConfig->AbilityClass, 1, INDEX_NONE, this);
	Spec.SourceObject = DemoAICharacterGlobalConfig->SkillConfig.Get();
	AbilitySystemComponent->GiveAbility(Spec);
}
