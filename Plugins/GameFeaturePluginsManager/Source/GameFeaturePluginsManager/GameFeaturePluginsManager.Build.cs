// Copyright (c) Yevhenii Selivanov.

using UnrealBuildTool;

public class GameFeaturePluginsManager : ModuleRules
{
	public GameFeaturePluginsManager(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		CppCompileWarningSettings.NonInlinedGenCppWarningLevel = WarningLevel.Error;

		PublicDependencyModuleNames.AddRange(new[]
			{
				"Core"
				, "GameFeatures" // Game Feature Plugins (GFP) framework
				, "ModularGameplay" // Created UGameFrameworkComponentManager extension handlers
				, "GameplayTags" // Tags
			}
		);

		PrivateDependencyModuleNames.AddRange(new[]
			{
				"CoreUObject", "Engine", "Slate", "SlateCore" // Core
				, "GameplayAbilities" // Ability System Component tags listening
                , "AsyncMessageSystem" // Listen for world-ASC-ready event
			}
		);

		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(new[]
				{
					"UnrealEd" // FEditorDelegates, GEditor
				}
			);
		}
	}
}
