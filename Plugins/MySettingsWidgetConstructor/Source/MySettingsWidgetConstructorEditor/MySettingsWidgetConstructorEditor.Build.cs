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
