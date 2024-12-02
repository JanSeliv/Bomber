﻿// Copyright (c) Yevhenii Selivanov.

using UnrealBuildTool;

public class NewMainMenu : ModuleRules
{
	public NewMainMenu(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Latest;
		bEnableNonInlinedGenCppWarnings = true;

		PublicDependencyModuleNames.AddRange(new[]
			{
				"Core", "Engine"
				, "UMG" // UUserWidget creation
				// My modules
				, "Bomber"
				, "MetaCheatManager" // UNMMCheatExtension
			}
		);

		PrivateDependencyModuleNames.AddRange(new[]
			{
				"CoreUObject", "Slate", "SlateCore" // Core
				, "MovieScene", "LevelSequence" // Cinematics
				, "AdvancedWidgets" // URadialSlider
				, "CineCameraRigs" // Camera rails
				, "GameplayTags" // UE_DEFINE_GAMEPLAY_TAG_STATIC
				// My modules
				, "MyUtils"
				, "SettingsWidgetConstructor"
			}
		);
	}
}