#include "DemoCharacterHoldingAbility.h"
#include "EnhancedInputSubsystems.h"
#include "DemoAbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "Gameplay/Character/DemoCharacter.h"
#include "Gameplay/Character/DemoCharacterGlobalConfig.h"
#include "Gameplay/AbilityActors//GroundBlastActor.h"
#include "Kismet/GameplayStatics.h"

void UDemoCharacterHoldingAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo,
                                                 const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);
}

void UDemoCharacterHoldingAbility::AuthConfirmHoldingAbility(FPredictionKey PredictionKey)
{
	UDemoAbilitySystemComponent* ASC = OwnerCharacter ? OwnerCharacter->GetAbilitySystemComponent() : nullptr;
	if (!ASC || !SummonItem)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}
	
	bHasConfirmed = true;
	
	TArray<AActor*> FilterActors;
	SummonItem->ConfirmHoldingAbility(FilterActors);
	
	{
		FScopedPredictionWindow ScopedPredictionWindow(ASC, PredictionKey, true);
		ApplyCostAndCooldown(PredictionKey);
		SummonItem->ExecuteDamageGameplayCue();
	}
	
	// 伤害不参与预测
	SummonItem->ApplyEffectsToFilterActors(FilterActors);
	
	if (!PlayFireMontage())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}
}

bool UDemoCharacterHoldingAbility::PlayFireMontage()
{
	if (SummonItem)
	{
		if (SummonItem->GetNetMode() < NM_Client)
		{
			SummonItem->SetDead();
		}
		else
		{
			SummonItem->HiddenSelf();
		}
	}
	
	return Super::PlayFireMontage();
}

bool UDemoCharacterHoldingAbility::OnDemoCharacterGameplayAbilityActivated(
	const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!ActorInfo || !RoleSkillConfig || !OwnerCharacter)
	{
		return false;
	}
	
	if (!PlayHoldingMontage())
	{
		return false;
	}
	
	bHasConfirmed = false;
	OwnerCharacter->UpdateCameraLockState(true);
	
	if (OwnerCharacter->GetNetMode() != NM_DedicatedServer)
	{
		OwnerCharacter->OnSkillConfirmed.BindUObject(this, &UDemoCharacterHoldingAbility::OnSkillConfirmed);	
	}
	
	if (RoleSkillConfig->SummonItemClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = OwnerCharacter;
		SpawnParams.Instigator = OwnerCharacter;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SummonItem = GetWorld()->SpawnActor<ASummonItemBase>(RoleSkillConfig->SummonItemClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	}
	
	AddSkillIMC();
	
	return true;
}

void UDemoCharacterHoldingAbility::PreDemoCharacterGameplayAbilityEnded(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	HoldingMontageTask = nullptr;
	
	if (APlayerController* PC = OwnerCharacter->GetPlayerController())
	{
		OwnerCharacter->UpdateCameraLockState(false);
	}
	
	if (OwnerCharacter->GetNetMode() != NM_DedicatedServer)
	{
		OwnerCharacter->OnSkillConfirmed.Unbind();
	}
	
	if (SummonItem)
	{
		SummonItem->HiddenSelf();
	}
	
	RemoveSkillIMC();
	
	Super::PreDemoCharacterGameplayAbilityEnded(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UDemoCharacterHoldingAbility::PlayHoldingMontage()
{
	UAnimMontage* Montage = HoldingMontage.IsNull() ? nullptr : HoldingMontage.LoadSynchronous();
	if (!Montage)
	{
		return false;
	}
	
	HoldingMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, Montage);
	if (!HoldingMontageTask)
	{
		return false;
	}
	
	HoldingMontageTask->OnCancelled.AddDynamic(this, &UDemoCharacterHoldingAbility::OnHoldingMontageCancelled);
	HoldingMontageTask->ReadyForActivation();

	return true;
}

void UDemoCharacterHoldingAbility::AddSkillIMC()
{
	if (OwnerPlayerController && OwnerPlayerController->IsLocalPlayerController())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(OwnerPlayerController->GetLocalPlayer()))
		{
			UDemoCharacterGlobalConfig* CharacterGlobalConfig = OwnerCharacter->GetDemoCharacterGlobalConfig();
			for (UInputMappingContext* CurrentContext : CharacterGlobalConfig->SkillMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 1);
			}
		}
	}
}

void UDemoCharacterHoldingAbility::RemoveSkillIMC()
{
	if (OwnerPlayerController && OwnerPlayerController->IsLocalPlayerController())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(OwnerPlayerController->GetLocalPlayer()))
		{
			UDemoCharacterGlobalConfig* CharacterGlobalConfig = OwnerCharacter->GetDemoCharacterGlobalConfig();
			for (UInputMappingContext* CurrentContext : CharacterGlobalConfig->SkillMappingContexts)
			{
				Subsystem->RemoveMappingContext(CurrentContext);
			}
		}
	}
}

void UDemoCharacterHoldingAbility::OnHoldingMontageCancelled()
{
	HandleHoldingMontageEnded(true);
}

void UDemoCharacterHoldingAbility::OnSkillConfirmed()
{
	if (!OwnerCharacter || !SummonItem || !PlayFireMontage())
	{
		return;
	}
	
	bHasConfirmed = true;
	OwnerCharacter->StartCD(AbilityIndex);
	if (OwnerCharacter->GetNetMode() == NM_Client)
	{
		if (UDemoAbilitySystemComponent* ASC = OwnerCharacter->GetAbilitySystemComponent())
		{
			FScopedPredictionWindow ScopedPredictionWindow(ASC, true);
			FPredictionKey PredictionKey = ASC->ScopedPredictionKey;
			ApplyCostAndCooldown(PredictionKey);
			
			TArray<AActor*> FilterActors;
			SummonItem->ConfirmHoldingAbility(FilterActors);
			SummonItem->ExecuteDamageGameplayCue();
			
			ASC->ServerConfirmHoldingAbility(CurrentSpecHandle, PredictionKey);
		}
	}
	else
	{
		AuthConfirmHoldingAbility();
	}
}

void UDemoCharacterHoldingAbility::HandleHoldingMontageEnded(bool bWasCancelled)
{
	if (HoldingMontageTask)
	{
		HoldingMontageTask->OnCancelled.RemoveDynamic(this, &UDemoCharacterHoldingAbility::OnHoldingMontageCancelled);
		HoldingMontageTask = nullptr;
	}
	
	if (!IsActive())
	{
		return;
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, bWasCancelled);
}

void UDemoCharacterHoldingAbility::ApplyCostAndCooldown(FPredictionKey PredictionKey)
{
	if (!OwnerCharacter)
	{
		return;
	}
    
	UDemoAbilitySystemComponent* ASC = OwnerCharacter->GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	UGameplayEffect* CostGE = GetCostGameplayEffect();
	UGameplayEffect* CooldownGE = GetCooldownGameplayEffect();
	if (!CostGE || !CooldownGE)
	{
		return;
	}

	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	FGameplayEffectSpecHandle CostSpec = ASC->MakeOutgoingSpec(CostGE->GetClass(), 1.0f, Context);
	ASC->ApplyGameplayEffectSpecToSelf(*CostSpec.Data.Get(), PredictionKey);
	FGameplayEffectSpecHandle CooldownSpec = ASC->MakeOutgoingSpec(CooldownGE->GetClass(), 1.0f, Context);
	ASC->ApplyGameplayEffectSpecToSelf(*CooldownSpec.Data.Get(), PredictionKey);
}
