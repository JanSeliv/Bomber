// Copyright (c) Yevhenii Selivanov.

using UnrealBuildTool;

public class SettingsWidgetConstructorEditor : ModuleRules
{
	public SettingsWidgetConstructorEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Latest;

		PublicDependencyModuleNames.AddRange(new[]
			{
				"Core"
				, "AssetTools" // Created FAssetTypeActions_MyDataTable
				, "UnrealEd" // Created UMyDataTableFactory
				, "UMGEditor" // Created UMyUserWidgetFactory
			}
		);

		PrivateDependencyModuleNames.AddRange(new[]
			{
				"CoreUObject", "Engine", "Slate", "SlateCore" // Core
				, "GameplayTagsEditor" // FGameplayTagCustomizationPublic
				, "Projects" // IPluginManager::Get()
				, "ToolWidgets" // SSearchableComboBox
				, "DataTableEditor", "DesktopPlatform", "EditorFramework", "ToolMenus" // Editor data table
				, "UMG", "Kismet", "KismetCompiler" // Editor user widget
				// My modules
				, "SettingsWidgetConstructor" // USettingsDataTable
			}
		);
	}
}
