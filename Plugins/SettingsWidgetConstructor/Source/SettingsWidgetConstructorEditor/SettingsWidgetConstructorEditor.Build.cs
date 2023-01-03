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
				//My modules
				, "MyEditorUtils" // Created FSettingsPickerCustomization, FAssetTypeActions_SettingsDataTable, FAssetTypeActions_SettingsUserWidget
			}
		);

		PrivateDependencyModuleNames.AddRange(new[]
			{
				"CoreUObject", "Engine", "Slate", "SlateCore" // Core
				, "GameplayTagsEditor" // FGameplayTagCustomizationPublic
				, "AssetTools" // RegisterAdvancedAssetCategory
				, "UnrealEd" // Editor globals like GEditor
				, "Projects" // IPluginManager::Get()
				// My modules
				, "SettingsWidgetConstructor" // USettingsDataTable
				, "FunctionPickerEditor" // FFunctionPickerCustomization
			}
		);
	}
}
