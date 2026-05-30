// Fill out your copyright notice in the Description page of Project Settings.

#include "Gameplay/Abilities/ChaGA_FireBlast.h"

#include "DemoAbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Engine/OverlapResult.h"
#include "Gameplay/Character/DemoCharacter.h"

void UChaGA_FireBlast::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);
	
	FireBlastConfig = Cast<UFireBlastConfig>(RoleSkillConfig);
}

bool UChaGA_FireBlast::OnDemoCharacterGameplayAbilityActivated(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	bool res =  Super::OnDemoCharacterGameplayAbilityActivated(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	UAbilityTask_WaitGameplayEvent* WaitPullEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FireBlastConfig->FireBlastPullEventTag);
	if (WaitPullEventTask)
	{
		WaitPullEventTask->EventReceived.AddDynamic(this, &UChaGA_FireBlast::OnWaitPullEventTaskReady);
		WaitPullEventTask->ReadyForActivation();
	}
	
	return res;
}

void UChaGA_FireBlast::OnWaitPullEventTaskReady(FGameplayEventData Payload)
{
	if (!OwnerCharacter)
	{
		return;
	}
	
	if (OwnerCharacter->GetNetMode() < NM_Client)
	{
		CacheOverlapedEnemies.Reset();
		ScanEnemies(CacheOverlapedEnemies);
	
		// Pull Enemies
		for (auto Enemy : CacheOverlapedEnemies)
		{
			if (!Enemy)
			{
				continue;
			}
		
			FVector Direction = (OwnerCharacter->GetActorLocation() - Enemy->GetActorLocation()).GetSafeNormal();
			Enemy->PushAway(Direction, FireBlastConfig->ImpulseValue, 1.f);
		}	
	}
	
	UAbilityTask_WaitGameplayEvent* WaitPushEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FireBlastConfig->FireBlastPushEventTag);
	if (WaitPushEventTask)
	{
		WaitPushEventTask->EventReceived.AddDynamic(this, &UChaGA_FireBlast::OnWaitPushEventTaskReady);
		WaitPushEventTask->ReadyForActivation();
	}
}

void UChaGA_FireBlast::OnWaitPushEventTaskReady(FGameplayEventData Payload)
{
	if (OwnerCharacter->GetNetMode() < NM_Client)
	{
		// Push Enemies
		for (ADemoCharacter* Enemy : CacheOverlapedEnemies)
		{
			if (!Enemy)
			{
				continue;
			}
		
			FVector Direction = (Enemy->GetActorLocation() - OwnerCharacter->GetActorLocation()).GetSafeNormal();
			Enemy->PushAway(Direction, FireBlastConfig->ImpulseValue, 1.f);
			
			if (UDemoAbilitySystemComponent* ASC = Enemy->GetAbilitySystemComponent())
			{
				FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
				ContextHandle.AddInstigator(OwnerCharacter, OwnerCharacter);
				FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(FireBlastConfig->FireBlastDamageEffect, 1, ContextHandle);
				ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}
	}
}

void UChaGA_FireBlast::ScanEnemies(TArray<ADemoCharacter*>& Enemies)
{
	Enemies.Reset();
	
	FVector StartLocation = OwnerCharacter->GetActorLocation();
	TArray<FOverlapResult> OverlapResults;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(OwnerCharacter);
	FCollisionShape CollisionShape = FCollisionShape::MakeSphere(FireBlastConfig->SelectRadius);
	
	bool bHit = GetWorld()->OverlapMultiByObjectType(
		OverlapResults,
		StartLocation,
		FQuat::Identity,
		ECC_Pawn,
		CollisionShape,
		CollisionParams
	);
	
	for (const FOverlapResult& OverlapResult : OverlapResults)
	{
		ADemoCharacter* HitCharacter = Cast<ADemoCharacter>(OverlapResult.GetActor());
		if (!HitCharacter || HitCharacter == OwnerCharacter || HitCharacter->GetTeamID() == OwnerCharacter->GetTeamID() || HitCharacter->IsDead())
		{
			continue;
		}
		
		Enemies.AddUnique(HitCharacter);
	}
}
