// Copyright (c) Yevhenii Selivanov.

using UnrealBuildTool;

public class BomberEditor : ModuleRules
{
	public BomberEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Latest;

		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core"
			, "UnrealEd" // Created UMyUnrealEdEngine
			, "GameplayTagsEditor" // FGameplayTagCustomizationPublic
			// My modules
			, "MyEditorUtils" // Created Created FMorphDataCustomization, FAttachedMeshCustomization
		});

		PrivateDependencyModuleNames.AddRange(new[]
			{
				"CoreUObject", "Engine", "InputCore", "Slate", "SlateCore" // Core
				, "EditorFramework" // FEditorDelegates::FToolkitManager
				, "EditorStyle" // SSocketChooserPopup
				, "ToolWidgets" // SSearchableComboBox
				, "PropertyEditor" // IPropertyTypeCustomization
			}
		);
	}
}
