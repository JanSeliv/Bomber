// Copyright (c) Yevhenii Selivanov.

using UnrealBuildTool;

public class MyEditorUtils : ModuleRules
{
	public MyEditorUtils(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Latest;

		PublicDependencyModuleNames.AddRange(
			new[]
			{
				"Core",
				"UnrealEd", // Editor globals like GEditor
				"EditorFramework", // FEditorDelegates::FToolkitManager
				"PropertyEditor", "EditorStyle", // Property types customizations
				"ToolWidgets", // SSearchableComboBox
				"AssetTools", // RegisterAdvancedAssetCategory
				"DataTableEditor", "DesktopPlatform", "EditorFramework", "ToolMenus" // FAssetTypeActions_MyDataTable
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore"
			}
		);
	}
}
