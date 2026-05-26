// Fill out your copyright notice in the Description page of Project Settings.

#include "Gameplay/Spawner/DemoAISpawner.h"
#include "Gameplay/Subsystem/DemoGameInstanceSubsystem.h"

DEFINE_LOG_CATEGORY(LogDemoAISpawner);

ADemoAISpawner::ADemoAISpawner()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ADemoAISpawner::SpawnSeveralAI(int32 InCount)
{
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindUObject(this, &ADemoAISpawner::SpawnSingleAI, InCount);
	GetWorld()->GetTimerManager().SetTimer(SpawnTimerHandle, TimerDelegate, SpawnInterval, true);	
}

void ADemoAISpawner::SpawnSingleAI(int32 InMaxCount)
{
	if (CurrentSpawnCount >= InMaxCount)
	{
		CurrentSpawnCount = 0;
		GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
		UE_LOG(LogDemoAISpawner, Warning, TEXT("已达到最大刷怪数量，停止生成！"));
		
		return;
	}

	UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		return;
	}

	UDemoGameInstanceSubsystem* DemoGameInstanceSubsystem = GI->GetSubsystem<UDemoGameInstanceSubsystem>();
	if (!DemoGameInstanceSubsystem)
	{
		return;
	}

	ADemoAICharacter* SpawnedAI = DemoGameInstanceSubsystem->GetAICharacterFromPool(GetActorLocation(), GetActorRotation());
	if (SpawnedAI)
	{
		CurrentSpawnCount++;
		UE_LOG(LogDemoAISpawner, Warning, TEXT("成功从对象池生成 AI，当前AI总数: %d"), CurrentSpawnCount);
	}
	else
	{
		UE_LOG(LogDemoAISpawner, Warning, TEXT("AI生成失败！"));
	}
}