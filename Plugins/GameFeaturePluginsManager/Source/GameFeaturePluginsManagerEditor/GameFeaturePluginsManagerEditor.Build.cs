// Copyright (c) Yevhenii Selivanov.

using UnrealBuildTool;

public class GameFeaturePluginsManagerEditor : ModuleRules
{
	public GameFeaturePluginsManagerEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		CppCompileWarningSettings.NonInlinedGenCppWarningLevel = WarningLevel.Error;

		PublicDependencyModuleNames.AddRange(new[]
			{
				"Core"
				, "EditorSubsystem" // Created UGfpmEditorLoaderSubsystem
			}
		);

		PrivateDependencyModuleNames.AddRange(new[]
			{
				"CoreUObject", "Engine" // Core
				, "UnrealEd" // FEditorDelegates
				, "GameFeatures" // Game Feature Plugins (GFP) framework
				, "GameFeaturePluginsManager" // Own runtime module
			}
		);
	}
}
