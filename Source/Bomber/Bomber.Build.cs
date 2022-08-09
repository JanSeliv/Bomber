// Copyright (c) Yevhenii Selivanov.

using UnrealBuildTool;

public class Bomber : ModuleRules
{
	public Bomber(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core", "CoreUObject", "Engine", "InputCore", // Default
			"HeadMountedDisplay", "UMG", "Slate", "SlateCore", // UMG
			"AIModule", // AI
			"GameplayTags", // Tags
			"RHI", "ApplicationCore", // Resolutions
			"EnhancedInput", // Enhanced Input plugin
			"Niagara", // VFX
			"RenderCore", // Render threads,
			"MyUtils", // FFunctionPicker etc.
			"MySettings"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		if (Target.Type == TargetType.Editor)
			PrivateDependencyModuleNames.AddRange(
				new[]
				{
					"UnrealEd", // FEditorDelegates
					"BomberEditor", // UMyUnrealEdEngine
					"Blutility" // UEditorUtilityLibrary::GetSelectionSet()
				});

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
