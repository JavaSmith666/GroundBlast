// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Demo : ModuleRules
{
	public Demo(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		OptimizeCode = CodeOptimization.Never;
	
		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"Networking",
			"Sockets",
			"EnhancedInput",
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks",
			"DeveloperSettings",
			"UMG",
			"ModelViewViewModel",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"AIModule"
		});
		
		#if WITH_EDITOR
		PrivateDependencyModuleNames.AddRange(new string[]
		{
		    "ModelViewViewModelEditor",
		    "ModelViewViewModelDebugger",
		    "ModelViewViewModelDebuggerEditor"
		});
		#endif
		
		PublicIncludePaths.AddRange(new string[] {
			"Demo",
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
