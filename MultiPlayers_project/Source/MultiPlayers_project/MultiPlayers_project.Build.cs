// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MultiPlayers_project : ModuleRules
{
	public MultiPlayers_project(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"MultiPlayers_project",
			"MultiPlayers_project/Variant_Platforming",
			"MultiPlayers_project/Variant_Platforming/Animation",
			"MultiPlayers_project/Variant_Combat",
			"MultiPlayers_project/Variant_Combat/AI",
			"MultiPlayers_project/Variant_Combat/Animation",
			"MultiPlayers_project/Variant_Combat/Gameplay",
			"MultiPlayers_project/Variant_Combat/Interfaces",
			"MultiPlayers_project/Variant_Combat/UI",
			"MultiPlayers_project/Variant_SideScrolling",
			"MultiPlayers_project/Variant_SideScrolling/AI",
			"MultiPlayers_project/Variant_SideScrolling/Gameplay",
			"MultiPlayers_project/Variant_SideScrolling/Interfaces",
			"MultiPlayers_project/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
