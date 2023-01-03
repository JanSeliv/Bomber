// Copyright (c) Yevhenii Selivanov.

using UnrealBuildTool;

public class SettingsWidgetConstructor : ModuleRules
{
	public SettingsWidgetConstructor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Latest;

		PublicDependencyModuleNames.AddRange(new[]
			{
				"Core"
				, "UMG" // Created USettingsWidget
				, "GameplayTags" // Created FSettingTag
				, "DeveloperSettings" // Created USettingsDataAsset
				// My modules
                , "MyUtils" // Created USettingsDataTable
                , "FunctionPicker" // Used by SettingsRow.h
			}
		);

		PrivateDependencyModuleNames.AddRange(new[]
			{
				"CoreUObject", "Engine", "Slate", "SlateCore" // Core
			}
		);

		if (Target.bBuildEditor)
		{
			// Include Editor modules that are used in this Runtime module
			PrivateDependencyModuleNames.AddRange(new[]
				{
					"UnrealEd" // FDataTableEditorUtils
					// My plugins
					, "MyEditorUtils" // UEditorUtilsLibrary
				}
			);
		}
	}
}
