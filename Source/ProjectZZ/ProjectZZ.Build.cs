// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ProjectZZ : ModuleRules
{
	public ProjectZZ(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "EnhancedInput",
			"UMG", "AIModule", "NavigationSystem", "Niagara", "SlateCore", "Sockets",
			"Networking", "NetCore", "Json", "GameplayAbilities", "GameplayTags", "GameplayTasks", "AdvancedWidgets", "CommonUI"
		});
	}
}
