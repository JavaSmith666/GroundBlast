// Fill out your copyright notice in the Description page of Project Settings.

#include "Gameplay/AnimInstance/AnimInst_Shinbi.h"

#include "Components/CapsuleComponent.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Gameplay/Abilities/DemoAbilitySystemComponent.h"
#include "Gameplay/Character/DemoCharacter.h"

void UAnimInst_Shinbi::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	OwnerCharacter = Cast<ADemoCharacter>(TryGetPawnOwner());
}

void UAnimInst_Shinbi::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);
	
	if (OwnerCharacter)
	{
		FVector Velocity = OwnerCharacter->GetVelocity();
		Speed = Velocity.Size2D();
		Angle = CalculateDirection(Velocity, OwnerCharacter->GetActorRotation());
		if (UCharacterMovementComponent* MoveComp = OwnerCharacter->GetCharacterMovement())
		{
			bInAir = MoveComp->IsFalling();
		}
	}
}

void UAnimInst_Shinbi::OnStartMontageNotify()
{
	bNeedMontage = true;
	USkeletalMeshComponent* SkeletalMeshComp = OwnerCharacter->GetMesh();
	if (!SkeletalMeshComp || !OwnerCharacter || OwnerCharacter->GetNetMode() == NM_Client)
	{
		return;
	}
	
	if (UCapsuleComponent* DamageCapsuleComponent = OwnerCharacter->GetMeleeDamageCapsule())
	{
		DamageCapsuleComponent->SetCollisionResponseToChannel(ECC_Pawn, ECollisionResponse::ECR_Overlap);
	}
	
	FVector StartLocation = SkeletalMeshComp->GetBoneLocation(StartAttackBoneName);
	FVector OwnerCharacterForwardVector = OwnerCharacter->GetActorForwardVector();
	FCollisionShape CollisionShape = FCollisionShape::MakeCapsule(AttackCapsuleRadius, AttackCapsuleHalfHeight);
	TArray<FOverlapResult> OutOverlaps;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerCharacter);
	
	TSet<ADemoCharacter*> OverlappingCharacters;
	GetWorld()->OverlapMultiByObjectType(OutOverlaps, StartLocation, FQuat::Identity, ECC_Pawn, CollisionShape, QueryParams);
	for (const FOverlapResult& OverlapResult : OutOverlaps)
	{
		AActor* HitActor = OverlapResult.GetActor();
		ADemoCharacter* HitCharacter = Cast<ADemoCharacter>(HitActor);
		if (!HitCharacter)
		{
			continue;
		}
		
		if (OverlappingCharacters.Contains(HitCharacter))
		{
			continue;
		}
		
		if (HitCharacter->GetTeamID() == OwnerCharacter->GetTeamID())
		{
			continue;
		}
		
		OverlappingCharacters.Add(HitCharacter);
		FVector SourceToTargetVector = HitActor->GetActorLocation() - OwnerCharacter->GetActorLocation();
		if (OwnerCharacterForwardVector.Dot(SourceToTargetVector) <= 0.f)
		{
			continue;
		}
		
		if (UDemoAbilitySystemComponent* ASC = HitCharacter->GetAbilitySystemComponent())
		{
			UClass* GEClass = MeleeDamageEffect.IsNull() ? nullptr : MeleeDamageEffect.LoadSynchronous();
			UGameplayEffect* DamageEffect = GEClass ? GEClass->GetDefaultObject<UGameplayEffect>() : nullptr;
			ASC->ApplyGameplayEffectToSelf(DamageEffect, 1.f, ASC->MakeEffectContext());
		}
	}
}

void UAnimInst_Shinbi::OnEndMontageNotify()
{
	bNeedMontage = false;
	
	if (UCapsuleComponent* DamageCapsuleComponent = OwnerCharacter->GetMeleeDamageCapsule())
	{
		DamageCapsuleComponent->SetCollisionResponseToChannel(ECC_Pawn, ECollisionResponse::ECR_Ignore);
	}
}
