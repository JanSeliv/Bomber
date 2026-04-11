// Copyright (c) Yevhenii Selivanov.

using UnrealBuildTool;

public class MyEditorUtils : ModuleRules
{
	public MyEditorUtils(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		CppCompileWarningSettings.NonInlinedGenCppWarningLevel = WarningLevel.Error;

		PublicDependencyModuleNames.AddRange(
			new[]
			{
				"Core"
				, "AssetTools" // Created FAssetTypeActions_MyDataTable
				, "EditorSubsystem" // Created UMyWorldPartitionEditorSubsystem
				, "UnrealEd" // Created UMyDataTableFactory
				, "UMGEditor" // Created UMyUserWidgetFactory
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new[]
			{
				"CoreUObject", "Engine", "Slate", "SlateCore" // Core
				, "EditorStyle" // Property types customizations
				, "ToolWidgets" // SSearchableComboBox
				, "DataTableEditor", "DesktopPlatform", "EditorFramework", "ToolMenus" // Editor data table
				, "UMG", "Kismet", "KismetCompiler" // Editor user widget
			}
		);
	}
}
