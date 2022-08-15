// Copyright (c) Yevhenii Selivanov.

using UnrealBuildTool;

public class MySettingsWidgetConstructor : ModuleRules
{
	public MySettingsWidgetConstructor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Latest;

		PublicDependencyModuleNames.AddRange(new[]
			{
				"Core",
				"UMG",
				"GameplayTags", // Tags
				"MyUtils" // FFunctionPicker
			}
		);

		PrivateDependencyModuleNames.AddRange(new[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore"
			}
		);

		if (Target.bBuildEditor)
		{
			// Include Editor modules that are used in this Runtime module
			PrivateDependencyModuleNames.AddRange(new[]
				{
					// Include Editor modules that are used in this Runtime module
					"MyEditorUtils" // UEditorUtilsLibrary
				}
			);
		}
	}
}
