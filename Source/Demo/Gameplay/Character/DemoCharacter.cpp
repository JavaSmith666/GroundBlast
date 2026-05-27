// Copyright Epic Games, Inc. All Rights Reserved.

#include "DemoCharacter.h"

#include "DemoCharacterGlobalConfig.h"
#include "DemoCharacterSettings.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "DemoAICharacter.h"
#include "Gameplay/Abilities/DemoAbilitySystemComponent.h"
#include "GameplayAbilitySpec.h"
#include "Gameplay/Abilities/DemoCharacterGameplayAbility.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"
#include "Gameplay/Settings/DemoAbilitySettings.h"
#include "Gameplay/Abilities/DataTables/CharacterSkillSlotsRow.h"
#include "Gameplay/Abilities/DataAssets/SkillConfig.h"
#include "Gameplay/AttributeSet/BaseAttributeSet.h"
#include "Components/ArrowComponent.h"
#include "Components/SphereComponent.h"

DEFINE_LOG_CATEGORY(LogDemoCharacter);

ADemoCharacter::ADemoCharacter()
{
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	
	if (!IsA<ADemoAICharacter>())
	{
		CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
		CameraBoom->SetupAttachment(RootComponent);
		CameraBoom->TargetArmLength = 400.0f;
		CameraBoom->bUsePawnControlRotation = true;
		
		FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
		FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
		FollowCamera->bUsePawnControlRotation = false;
		
		LaserPoint = CreateDefaultSubobject<UArrowComponent>(TEXT("LaserPoint"));
		LaserPoint->SetupAttachment(RootComponent);
		
		DashDamageSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DashDamageSphere"));
		DashDamageSphere->SetupAttachment(RootComponent);
	}
	
	MeleeDamageCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("MeleeDamageCapsule"));
	MeleeDamageCapsule->SetupAttachment(GetMesh(), MeleeDamageCapsuleSocketName);

	AbilitySystemComponent = CreateDefaultSubobject<UDemoAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
}

void ADemoCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	InitializeCharacterGlobalConfig();

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		InitializeSkillDataFromDataTable();
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UBaseAttributeSet::GetHPAttribute()).AddUObject(this, &ADemoCharacter::OnHPAttributeChanged);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UBaseAttributeSet::GetMPAttribute()).AddUObject(this, &ADemoCharacter::OnMPAttributeChanged);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UBaseAttributeSet::GetStrengthAttribute()).AddUObject(this, &ADemoCharacter::OnStrengthAttributeChanged);
		
		if (DemoCharacterGlobalConfig && DemoCharacterGlobalConfig->HealthRegenInfiniteEffect && !IsA<ADemoAICharacter>())
		{
			FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(DemoCharacterGlobalConfig->HealthRegenInfiniteEffect, 1.f, AbilitySystemComponent->MakeEffectContext());
			HealthRegenInfiniteEffectSpecHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}
	
	if (DashDamageSphere)
	{
		DashDamageSphere->OnComponentBeginOverlap.AddDynamic(this, &ADemoCharacter::OnDashDamageSphereOverlap);
	}
}

void ADemoCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->RemoveActiveGameplayEffect(HealthRegenInfiniteEffectSpecHandle);
	}
	
	if (DashDamageSphere)
	{
		DashDamageSphere->OnComponentBeginOverlap.RemoveDynamic(this, &ADemoCharacter::OnDashDamageSphereOverlap);
	}
}

void ADemoCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ADemoCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &ADemoCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ADemoCharacter::Look);

		InitializeCharacterGlobalConfig();
		TryBindPendingSkills();
		BindOtherActions();
	}
}

void ADemoCharacter::InitializeCharacterGlobalConfig()
{
	const UDemoCharacterSettings* Settings = GetDefault<UDemoCharacterSettings>();
	if (!DemoCharacterGlobalConfig && Settings && !Settings->CharacterGlobalConfig.IsNull())
	{
		UDemoCharacterGlobalConfig* CharacterGlobalConfig = Settings->CharacterGlobalConfig.LoadSynchronous();
		DemoCharacterGlobalConfig = CharacterGlobalConfig;
	}
}

void ADemoCharacter::OnDashDamageSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == this || DashOverlapActors.Contains(OtherActor) || !DemoCharacterGlobalConfig)
	{
		return;
	}
	
	DashOverlapActors.Add(OtherActor);
	if (ADemoCharacter* OtherCharacter = Cast<ADemoCharacter>(OtherActor))
	{
		if (TeamID == OtherCharacter->GetTeamID())
		{
			return;
		}
		
		if (GetNetMode() < NM_Client)
		{
			OtherCharacter->MultiPlayMontage(DemoCharacterGlobalConfig->StunMontage);
			if (UDemoAbilitySystemComponent* OtherASC = OtherCharacter->GetAbilitySystemComponent())
			{
				FGameplayEffectSpecHandle SpecHandle = OtherASC->MakeOutgoingSpec(DemoCharacterGlobalConfig->DashDamageEffect, 1.f, OtherASC->MakeEffectContext());
				OtherASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
				FVector Direction = (OtherCharacter->GetActorLocation() - GetActorLocation()).GetSafeNormal();
				OtherCharacter->PushAway(Direction, DemoCharacterGlobalConfig->DashImpulse, 1.f);
			}
		}
		else
		{
			if (UAnimInstance* AnimInstance = OtherCharacter->GetMesh()->GetAnimInstance())
			{
				AnimInstance->Montage_Play(DemoCharacterGlobalConfig->StunMontage);
			}
		}
		
		if (OtherCharacter->GetTeamID() != TeamID)
		{
			OtherCharacter->Stun(1.f);
		}
	}
}

APlayerController* ADemoCharacter::GetPlayerController() const
{
	return Cast<APlayerController>(GetController());
}

void ADemoCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void ADemoCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void ADemoCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void ADemoCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		AddControllerYawInput(Yaw);
		const FRotator ControlRot = GetController()->GetControlRotation();
		const float CurrentPitch = FRotator::NormalizeAxis(ControlRot.Pitch);
		float DesiredPitch = CurrentPitch - Pitch;
		if (DesiredPitch >= MinCameraPitch && DesiredPitch <= MaxCameraPitch)
		{
			FRotator NewRot = GetController()->GetControlRotation();
			NewRot.Pitch = ControlRot.Pitch - Pitch;
			GetController()->SetControlRotation(NewRot);
		}
	}
}

void ADemoCharacter::DoJumpStart()
{
	// signal the character to jump
	Jump();
}

void ADemoCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}

void ADemoCharacter::OnMainUICreatedEvent()
{
	OnMainUICreated.Broadcast();
}

void ADemoCharacter::MultiPlayMontage_Implementation(UAnimMontage* MontageToPlay)
{
	if (!MontageToPlay)
	{
		return;
	}
	
	if (IsLocallyControlled())
	{
		return;
	}
	
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->Montage_Play(MontageToPlay);
	}
}

void ADemoCharacter::ClearGrantedAbilities()
{
	if (GetNetMode() == NM_Client)
	{
		return;
	}
	
	if (UDemoAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		for (auto& SpecHandle : GrantedAbilityHandles)
		{
			ASC->ClearAbility(SpecHandle);
		}
	}
}

void ADemoCharacter::OnSkillConfigsLoaded()
{
	auto GrantAndBind = [&](const FSkillSlotEntry& Entry, int32 AbilityIndex) -> void
	{
		UClass* AbilityClass = Entry.AbilityClass.Get();		
		if (AbilityClass && AbilitySystemComponent)
		{
			if (GetNetMode() < NM_Client)
			{
				FGameplayAbilitySpec Spec(AbilityClass, 1, INDEX_NONE, this);
				Spec.InputID = AbilityIndex;
				
				// Use SourceObject to carry the config to the ability instances
				if (USkillConfig* Config = Entry.SkillConfig.Get())
				{
					Spec.SourceObject = Config;
				}
				
				FGameplayAbilitySpecHandle SpecHandle = AbilitySystemComponent->GiveAbility(Spec);
				GrantedAbilityHandles.Add(SpecHandle);
			}
			
			if (GetNetMode() != NM_DedicatedServer)
			{
				if (UInputAction* Action = Entry.ActivateAction.Get())
				{
					FPendingAbilityBinding Binding;
					Binding.AbilityClass = AbilityClass;
					Binding.AbilityTags = Entry.AbilityTags;
					Binding.ActivateAction = Action;
					PendingBindings.Add(Binding);
				}
			}
		}
	};
	
	UE_LOG(LogDemoCharacter, Log, TEXT("[ADemoCharacter::OnSkillConfigsLoaded] Authority: %d"), GetNetMode() < NM_Client);
	UE_LOG(LogDemoCharacter, Log, TEXT("[ADemoCharacter::OnSkillConfigsLoaded] PendingSkillEntries.Num(): %d"), PendingSkillEntries.Num());
	
	GrantAndBind(PendingDefaultSkill, -1);
	
	if (!IsA<ADemoAICharacter>())
	{
		for (int32 i = 0; i < PendingSkillEntries.Num(); i++)
		{
			GrantAndBind(PendingSkillEntries[i], i);
		}	
	}
	
	// Attempt binding immediately if InputComponent is already ready
	TryBindPendingSkills();
}

void ADemoCharacter::InitializeSkillDataFromDataTable()
{
	// Get DT from settings
	const UDemoAbilitySettings* Settings = GetDefault<UDemoAbilitySettings>();
	if (Settings && !Settings->CharacterSkillTable.IsNull())
	{
		UDataTable* SkillTable = Settings->CharacterSkillTable.LoadSynchronous();
		if (SkillTable)
		{
			// Find row by CharacterID
			FCharacterSkillSlotsRow* Row = SkillTable->FindRow<FCharacterSkillSlotsRow>(FName(*FString::FromInt(CharacterID)), TEXT("Skill Grant"));
			if (Row)
			{
				PendingSkillEntries = Row->SlotSkills;
				PendingDefaultSkill = Row->DefaultSkill;
				TArray<FSoftObjectPath> PathsToLoad;
				if (!Row->DefaultSkill.AbilityClass.IsNull())
				{
					PathsToLoad.Add(Row->DefaultSkill.AbilityClass.ToSoftObjectPath());
				}
				if (!Row->DefaultSkill.SkillConfig.IsNull())
				{
					PathsToLoad.Add(Row->DefaultSkill.SkillConfig.ToSoftObjectPath());
				}
				if (!Row->DefaultSkill.ActivateAction.IsNull())
				{
					PathsToLoad.Add(Row->DefaultSkill.ActivateAction.ToSoftObjectPath());
				}
				
				if (TeamID != ETeamID::Enemy)
				{
					for (auto& Entry : Row->SlotSkills)
					{
						if (!Entry.AbilityClass.IsNull())
						{
							PathsToLoad.Add(Entry.AbilityClass.ToSoftObjectPath());
						}
						if (!Entry.SkillConfig.IsNull())
						{
							PathsToLoad.Add(Entry.SkillConfig.ToSoftObjectPath());
						}
						if (!Entry.ActivateAction.IsNull())
						{
							PathsToLoad.Add(Entry.ActivateAction.ToSoftObjectPath());
						}
					}	
				}

				if (PathsToLoad.Num() > 0)
				{
					UAssetManager::GetStreamableManager().RequestAsyncLoad(PathsToLoad, FStreamableDelegate::CreateUObject(this, &ADemoCharacter::OnSkillConfigsLoaded));
				}
			}
		}
	}
}

void ADemoCharacter::BindOtherActions()
{
	if (InputComponent && IsLocallyControlled() && DemoCharacterGlobalConfig)
	{
		if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
		{
			if (DemoCharacterGlobalConfig->CancelAbilityAction)
			{
				EIC->BindAction(DemoCharacterGlobalConfig->CancelAbilityAction, ETriggerEvent::Started, this, &ADemoCharacter::OnCancelActionBound);	
			}
					
			if (DemoCharacterGlobalConfig->ConfirmSkillAbilityAction)
			{
				EIC->BindAction(DemoCharacterGlobalConfig->ConfirmSkillAbilityAction, ETriggerEvent::Started, this, &ADemoCharacter::OnConfirmSkillActionBound);	
			}
		}
	}
}

void ADemoCharacter::OnCancelActionBound()
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->CancelAllAbilities();
	}
}

void ADemoCharacter::OnConfirmSkillActionBound()
{
	OnSkillConfirmed.ExecuteIfBound();
}

void ADemoCharacter::CheckDeath(float InCurrentHP)
{
	if (!bIsDead && InCurrentHP <= 0.f && DemoCharacterGlobalConfig)
	{
		bIsDead = true;
		
		if (GetNetMode() != NM_DedicatedServer)
		{
			// 本地预测播放蒙太奇
			if (UAnimInstance* AnimInst = GetMesh()->GetAnimInstance())
			{
				AnimInst->Montage_Play(DemoCharacterGlobalConfig->DeathMontage);
			}
		}
		else
		{
			// 多播RPC让模拟端播蒙太奇
			MultiPlayMontage(DemoCharacterGlobalConfig->DeathMontage);	
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

void ADemoCharacter::OnHPAttributeChanged(const FOnAttributeChangeData& Data)
{
	OnHPChange.Broadcast(Data.NewValue);
	CheckDeath(Data.NewValue);
	if (GetNetMode() != NM_DedicatedServer)
	{
		UpdateHPBar();
		UpdateHPAttributeBar();
	}
}

void ADemoCharacter::OnMPAttributeChanged(const FOnAttributeChangeData& Data)
{
	OnMPChange.Broadcast(Data.NewValue);
	if (GetNetMode() != NM_DedicatedServer)
	{
		UpdateMPAttributeBar();
	}
}

void ADemoCharacter::OnStrengthAttributeChanged(const FOnAttributeChangeData& Data)
{
	OnStrengthChange.Broadcast(Data.NewValue);
	if (GetNetMode() != NM_DedicatedServer)
	{
		UpdateStrengthAttributeBar();
	}
}

void ADemoCharacter::SetFrictionZero()
{
	if (UCharacterMovementComponent* CharacterMovementComponent = GetCharacterMovement())
	{
		LastFriction = CharacterMovementComponent->GroundFriction;
		CharacterMovementComponent->GroundFriction = 0.f;
	}
}

void ADemoCharacter::ResetFriction()
{
	if (UCharacterMovementComponent* CharacterMovementComponent = GetCharacterMovement())
	{
		CharacterMovementComponent->GroundFriction = LastFriction;
	}
}

void ADemoCharacter::PushAway(const FVector& Dir, float Strength, float DelayTime)
{
	SetFrictionZero();
	if (UCharacterMovementComponent* TempCharacterMovement = GetCharacterMovement())
	{
		TempCharacterMovement->StopMovementImmediately();
		TempCharacterMovement->AddImpulse(Dir.GetSafeNormal() * Strength, true);
		GetWorld()->GetTimerManager().SetTimer(PushAwayTimerHandle, FTimerDelegate::CreateUObject(this, &ADemoCharacter::OnPushAwayDelayTimeReached), DelayTime, false);
	}
}

void ADemoCharacter::OnPushAwayDelayTimeReached()
{
	ResetFriction();
}

void ADemoCharacter::Stun(float Duration)
{
	if (!IsLocallyControlled())
	{
		return;
	}
	
	if (GetNetMode() != NM_DedicatedServer)
	{
		if (APlayerController* PC = GetPlayerController())
		{
			DisableInput(PC);
			GetWorld()->GetTimerManager().SetTimer(StunTimerHandle, FTimerDelegate::CreateUObject(this, &ADemoCharacter::OnStunDurationReached), Duration, false);
		}
	}
}

void ADemoCharacter::OnStunDurationReached()
{
	if (GetNetMode() != NM_DedicatedServer)
	{
		if (APlayerController* PC = GetPlayerController())
		{
			EnableInput(PC);
		}
	}
}

void ADemoCharacter::TryBindPendingSkills()
{
	if (InputComponent && IsLocallyControlled() && PendingBindings.Num() > 0)
	{
		if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
		{
			for (const FPendingAbilityBinding& Binding : PendingBindings)
			{
				if (Binding.ActivateAction && Binding.AbilityClass)
				{
					EIC->BindAction(Binding.ActivateAction, ETriggerEvent::Started, this, &ADemoCharacter::OnSkillActionStarted, Binding.AbilityTags);
				}
			}
			// Clear pending bindings after they are successfully bound to avoid duplicate bindings
			PendingBindings.Empty();
		}
	}
}

void ADemoCharacter::OnSkillActionStarted(FGameplayTagContainer AbilityTags)
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->TryActivateAbilitiesByTag(AbilityTags);
	}
}

void ADemoCharacter::UpdateCameraLockState(bool bLock)
{
	bUseControllerRotationYaw = bLock;
	if (CameraBoom && DemoCharacterGlobalConfig)
	{
		CameraBoom->bUsePawnControlRotation	= !bLock;
		CameraBoom->SetRelativeLocation(bLock ? DemoCharacterGlobalConfig->LockCameraRelativeLocation : FVector::ZeroVector);
		CameraBoom->SetRelativeRotation(bLock ? DemoCharacterGlobalConfig->LockCameraRelativeRotation : FRotator::ZeroRotator);
	}
	
	if (UCharacterMovementComponent* CharacterMovementComponent = GetCharacterMovement())
	{
		CharacterMovementComponent->bOrientRotationToMovement = !bLock;
	}
}
