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
                , "DeveloperSettings" // Created UDataAssetsContainer
                //My modules
                , "FunctionPicker" // Used by MyInputAction.h
                , "MetaCheatManager" // Created UMyCheatManager
		    }
		);

		PrivateDependencyModuleNames.AddRange(new[]
			{
				"CoreUObject", "Engine", "InputCore", "Slate", "SlateCore" // Core
				, "RHI", "ApplicationCore" // Resolutions
				, "AIModule" // AI
				, "Niagara" // VFX
				, "GameplayTags" // FGameplayTag
                , "GameFeatures", "ModularGameplay" // Modular Game Features
				//My modules
				, "MyUtils" // UUtilsLibrary
				, "SettingsWidgetConstructor" // Generates settings
				, "PoolManager" // Used to spawn level actors
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
					, "MyEditorUtils" // FEditorUtilsLibrary
				}
			);
		}
	}
}
