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
				"CoreUObject", "Engine", "Slate", "SlateCore" // Core
				, "UnrealEd" // FEditorDelegates
				, "GameFeatures" // Game Feature Plugins (GFP) framework
				, "GameFeaturePluginsManager" // Own runtime module
				 // Switcher dockable tab (SDockTab, FGlobalTabmanager)
				 , "UMG"
				, "WorkspaceMenuStructure" // Switcher tab Window-menu category
				, "LevelEditor" // Dock switcher tab next to Scene Outliner via layout extension
				, "UATHelper" // Run BuildCookRun mod cook from editor
				, "DesktopPlatform" // Build folder picker
				, "DeveloperToolSettings" // Package Project platform + build configuration settings
			}
		);
	}
}
