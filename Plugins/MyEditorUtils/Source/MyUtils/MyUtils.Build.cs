// Copyright (c) Yevhenii Selivanov.

using UnrealBuildTool;

public class MyUtils : ModuleRules
{
	public MyUtils(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Latest;
		bEnableNonInlinedGenCppWarnings = true;

		PublicDependencyModuleNames.AddRange(new[]
			{
				"Core"
				, "ModelViewViewModel" // Created MVVM base classes
			}
		);

		PrivateDependencyModuleNames.AddRange(new[]
			{
				"CoreUObject", "Engine", "Slate", "SlateCore" // Core
				, "UMG" // UUserWidget
				, "MovieScene", "MovieSceneTracks" // UCinematicUtils
				, "EnhancedInput", "InputCore" // UInputUtilsLibrary
				, "NavigationSystem", "AIModule" // UAIUtilsLibrary
			}
		);

		if (Target.bBuildEditor)
		{
			// Include Editor modules that are used in this Runtime module
			PrivateDependencyModuleNames.AddRange(new[]
				{
					"MyEditorUtils" // FEditorUtilsLibrary
				}
			);
		}
	}
}
