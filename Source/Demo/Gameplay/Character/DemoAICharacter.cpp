// Fill out your copyright notice in the Description page of Project Settings.

#include "Gameplay/Character/DemoAICharacter.h"
#include "DemoAICharacterGlobalConfig.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Gameplay/Abilities/DemoAbilitySystemComponent.h"
#include "Gameplay/AttributeSet/BaseAttributeSet.h"
#include "Runtime/AIModule/Classes/AIController.h"
#include "Components/WidgetComponent.h"
#include "Engine/AssetManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Gameplay/Settings/DemoAICharacterSettings.h"
#include "Gameplay/Abilities/DataAssets/SkillConfig.h"
#include "Gameplay/Core/DemoGameState.h"
#include "Gameplay/Core/DemoPlayerState.h"

DEFINE_LOG_CATEGORY(LogDemoAICharacter);

ADemoAICharacter::ADemoAICharacter()
{
	bIsActive = false;
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

void ADemoAICharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
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
					UE_LOG(LogDemoAICharacter, Warning, TEXT("Self: %s, Instigator: %s"), *GetName(), *TempPlayerState->GetName());
				}
			}
		}
		
		if (!DemoAICharacterGlobalConfig || !DemoAICharacterGlobalConfig->DeathMontage)
		{
			return;
		}
		
		if (GetNetMode() != NM_DedicatedServer)
		{
			// 本地预测播放死亡动画
			if (UAnimInstance* AnimInst = GetMesh()->GetAnimInstance())
			{
				AnimInst->Montage_Play(DemoAICharacterGlobalConfig->DeathMontage);
			}
		}
		else
		{
			// 多播RPC让模拟端播放死亡动画
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
	if (GetNetMode() == NM_Client)
	{
		return;
	}
	
	UE_LOG(LogDemoAICharacter, Warning, TEXT("Activating AI Character: %s"), *GetName());

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
	
	// 开启Movement
	if (UCharacterMovementComponent* TempCharacterMovementComponent = GetCharacterMovement())
	{
		TempCharacterMovementComponent->SetActive(true);
		TempCharacterMovementComponent->SetMovementMode(MOVE_Walking);
		TempCharacterMovementComponent->SetComponentTickEnabled(true);
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
		if (DemoAICharacterGlobalConfig && DemoAICharacterGlobalConfig->BehaviorTreeClass)
		{
			AIController->RunBehaviorTree(DemoAICharacterGlobalConfig->BehaviorTreeClass);
			UE_LOG(LogDemoAICharacter, Warning, TEXT("Successfully started Behavior Tree: %s"), *DemoAICharacterGlobalConfig->BehaviorTreeClass->GetName());
		}
	}
	
	bIsActive = true;
	bIsDead = false;
	if (GetNetMode() == NM_Standalone)
	{
		if (HPBar)
		{
			HPBar->SetVisibility(bIsActive);	
		}
		
		ChangeSkeletalMesh();
	}
}

void ADemoAICharacter::Deactivate()
{
	if (GetNetMode() == NM_Client)
	{
		return;
	}

	UE_LOG(LogDemoAICharacter, Warning, TEXT("Deactivating AI Character: %s"), *GetName());
	
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);
	
	// 关闭Movement
	if (UCharacterMovementComponent* TempCharacterMovementComponent = GetCharacterMovement())
	{
		TempCharacterMovementComponent->SetMovementMode(MOVE_None);
		TempCharacterMovementComponent->StopMovementImmediately();
		TempCharacterMovementComponent->SetComponentTickEnabled(false);
		TempCharacterMovementComponent->SetActive(false);
	}
	
	// 结束死亡动画
	if (DemoAICharacterGlobalConfig && DemoAICharacterGlobalConfig->DeathMontage)
	{
		StopAnimMontage(DemoAICharacterGlobalConfig->DeathMontage);
	}
	
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
	
	if (DemoAICharacterGlobalConfig && DemoAICharacterGlobalConfig->DeathMontage)
	{
		StopAnimMontage(DemoAICharacterGlobalConfig->DeathMontage);
	}
	
	if (bIsActive)
	{
		ChangeSkeletalMesh();	
	}
}

void ADemoAICharacter::OnAICharacterGlobalConfigLoaded()
{
	const UDemoAICharacterSettings* Settings = GetDefault<UDemoAICharacterSettings>();
	if (!DemoAICharacterGlobalConfig && Settings && !Settings->AICharacterGlobalConfig.IsNull())
	{
		DemoAICharacterGlobalConfig = Settings->AICharacterGlobalConfig.Get();
		TryGrantSkills();
		
		if (AutoActivateParameters.bAutoActivateOnAssetLoaded && GetNetMode() < NM_Client)
		{
			Activate(AutoActivateParameters.Location, AutoActivateParameters.Rotation);
		}
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
	if (!DemoAICharacterGlobalConfig || !AbilitySystemComponent || GetNetMode() == NM_Client)
	{
		return;
	}
	
	FGameplayAbilitySpec Spec(DemoAICharacterGlobalConfig->AbilityClass, 1, INDEX_NONE, this);
	Spec.SourceObject = DemoAICharacterGlobalConfig->SkillConfig.Get();
	AbilitySystemComponent->GiveAbility(Spec);
}

void ADemoAICharacter::ChangeSkeletalMesh()
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}
	
	ADemoGameState* DemoGameState = GetWorld()->GetGameState<ADemoGameState>();
	if (!DemoGameState)
	{
		return;
	}
	
	int32 CurrentRoundIndex = DemoGameState->GetCurrentRoundIndex();
	if (CurrentRoundIndex <= 0)
	{
		return;
	}
	
	if (auto FoundMesh = RoundIndexToAISkeletalMeshMap.Find(CurrentRoundIndex))
	{
		USkeletalMesh* TempMesh = FoundMesh->IsNull() ? nullptr : FoundMesh->LoadSynchronous();
		GetMesh()->SetSkeletalMesh(TempMesh);
        UE_LOG(LogDemoAICharacter, Warning, TEXT("Successfully changed SkeletalMesh for Round %d"), CurrentRoundIndex);
	}
}
