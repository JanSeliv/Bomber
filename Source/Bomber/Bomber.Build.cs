// Copyright (c) Yevhenii Selivanov.

using UnrealBuildTool;

public class Bomber : ModuleRules
{
	public Bomber(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Cpp17; // Fails on CppStandardVersion.Latest by Niagara

		PublicDependencyModuleNames.AddRange(new[]
		    {
                "Core" // Core
                , "UMG" // UUserWidget creation
                , "EnhancedInput" // Created UMyInputAction, UMyInputMappingContext
		    }
		);

		PrivateDependencyModuleNames.AddRange(new[]
			{
				"CoreUObject", "Engine", "InputCore", "Slate", "SlateCore" // Core
				, "RHI", "ApplicationCore" // Resolutions
				, "AIModule" // AI
				, "Niagara" // VFX
				, "RenderCore" // Render threads
                , "GameplayTags" // FGameplayTag
				//My modules
				, "MyUtils" // FFunctionPicker etc.
				, "MySettingsWidgetConstructor" // Generates settings
				, "ModularGameplay" // UGameFrameworkComponentManager
			}
		);

		if (Target.bBuildEditor)
		{
			// Include Editor modules that are used in this Runtime module
			PrivateDependencyModuleNames.AddRange(new[]
				{
					"UnrealEd" // FEditorDelegates
					, "Blutility" // UEditorUtilityLibrary::GetSelectionSet()
					//My modules
					, "BomberEditor" // UMyUnrealEdEngine
					, "MyEditorUtils" // UEditorUtilsLibrary
				}
			);
		}
	}
}
