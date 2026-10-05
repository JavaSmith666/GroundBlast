#include "Gameplay/Core/DemoGameInstance.h"
#include "MVVMGameSubsystem.h"
#include "Gameplay/MVVM/GameStateViewModel.h"

void UDemoGameInstance::Init()
{
	Super::Init();
	
	if (UMVVMGameSubsystem* MVVMSubsystem = GetSubsystem<UMVVMGameSubsystem>())
	{
		UGameStateViewModel* VM = NewObject<UGameStateViewModel>(this);
		FMVVMViewModelContext Context;
		Context.ContextClass = UGameStateViewModel::StaticClass();
		Context.ContextName = TEXT("PlayerStatus");
		MVVMSubsystem->GetViewModelCollection()->AddViewModelInstance(Context, VM);
		CachedGameStateViewModel = VM;
	}
}
