// Copyright (c) Yevhenii Selivanov.

using UnrealBuildTool;

public class MySettingsWidgetConstructorEditor : ModuleRules
{
	public MySettingsWidgetConstructorEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Latest;

		PublicDependencyModuleNames.AddRange(new[]
			{
				"Core",
				"AssetTools", // RegisterAdvancedAssetCategory
				"UnrealEd", // Editor globals like GEditor
				"MySettingsWidgetConstructor", // Runtime module of this plugin
				"GameplayTagsEditor", // FSettingTag customization
				"MyEditorUtils" // FMyPropertyTypeCustomization
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
	}
}
