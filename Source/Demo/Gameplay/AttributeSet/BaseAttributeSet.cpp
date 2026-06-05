#include "BaseAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "Gameplay/Character/DemoCharacter.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY(LogBaseAttributeSet);

UBaseAttributeSet::UBaseAttributeSet()
{
}

void UBaseAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UBaseAttributeSet, HP);
	DOREPLIFETIME(UBaseAttributeSet, MaxHP);
	DOREPLIFETIME(UBaseAttributeSet, MP);
	DOREPLIFETIME(UBaseAttributeSet, MaxMP);
	DOREPLIFETIME(UBaseAttributeSet, Strength);
	DOREPLIFETIME(UBaseAttributeSet, MaxStrength);
}

bool UBaseAttributeSet::PreGameplayEffectExecute(struct FGameplayEffectModCallbackData& Data)
{
	LastInstigator = Data.EffectSpec.GetEffectContext().GetOriginalInstigator();
	UE_LOG(LogBaseAttributeSet, Warning, TEXT("[UBaseAttributeSet::PostGameplayEffectExecute] LastInstigator: %s"), LastInstigator ? *LastInstigator->GetName() : TEXT("None"));
	
	return Super::PreGameplayEffectExecute(Data);
}

void UBaseAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetHPAttribute())
	{
		SetHP(FMath::Clamp(GetHP(), 0.0f, GetMaxHP()));
	}
	else if (Data.EvaluatedData.Attribute == GetMPAttribute())
	{
		SetMP(FMath::Clamp(GetMP(), 0.0f, GetMaxMP()));
	}
	else if (Data.EvaluatedData.Attribute == GetStrengthAttribute())
	{
		SetStrength(FMath::Clamp(GetStrength(), 0.0f, GetMaxStrength()));
	}
}

void UBaseAttributeSet::OnRep_HP(FGameplayAttributeData& RepData)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBaseAttributeSet, HP, RepData);
}

void UBaseAttributeSet::OnRep_MP(FGameplayAttributeData& RepData)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBaseAttributeSet, MP, RepData);
}

void UBaseAttributeSet::OnRep_Strength(FGameplayAttributeData& RepData)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBaseAttributeSet, Strength, RepData);
}
