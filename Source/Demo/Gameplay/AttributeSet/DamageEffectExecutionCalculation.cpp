// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/AttributeSet/DamageEffectExecutionCalculation.h"

#include "AbilitySystemComponent.h"
#include "BaseAttributeSet.h"

UDamageEffectExecutionCalculation::UDamageEffectExecutionCalculation()
{
}

void UDamageEffectExecutionCalculation::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	const FGameplayTagContainer* TagContainer = Spec.CapturedTargetTags.GetAggregatedTags();
	float ActualDamage = BaseDamage;
	if (TagContainer && TagContainer->HasTag(ExtraDamageTag))
	{
		ActualDamage = SpecialDamage; 
	}
	
	FGameplayModifierEvaluatedData EvalData;
	EvalData.Attribute = UBaseAttributeSet::GetHPAttribute();
	EvalData.ModifierOp = EGameplayModOp::AddBase; 
	EvalData.Magnitude = -ActualDamage;
	OutExecutionOutput.AddOutputModifier(EvalData);
}
