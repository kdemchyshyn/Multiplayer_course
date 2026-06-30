// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MP_game_struct : ModuleRules
{
	public MP_game_struct(ReadOnlyTargetRules Target) : base(Target)
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
			"MP_game_struct",
			"MP_game_struct/Variant_Platforming",
			"MP_game_struct/Variant_Platforming/Animation",
			"MP_game_struct/Variant_Combat",
			"MP_game_struct/Variant_Combat/AI",
			"MP_game_struct/Variant_Combat/Animation",
			"MP_game_struct/Variant_Combat/Gameplay",
			"MP_game_struct/Variant_Combat/Interfaces",
			"MP_game_struct/Variant_Combat/UI",
			"MP_game_struct/Variant_SideScrolling",
			"MP_game_struct/Variant_SideScrolling/AI",
			"MP_game_struct/Variant_SideScrolling/Gameplay",
			"MP_game_struct/Variant_SideScrolling/Interfaces",
			"MP_game_struct/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
