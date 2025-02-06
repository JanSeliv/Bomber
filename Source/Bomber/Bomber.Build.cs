// Copyright (c) Yevhenii Selivanov.

using UnrealBuildTool;

public class Bomber : ModuleRules
{
	public Bomber(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Latest;
		bEnableNonInlinedGenCppWarnings = true;

		// Network: enable Iris replication
		const bool bAddAsPublicDependency = true; // Created FMapComponentsContainer
		SetupIrisSupport(Target, bAddAsPublicDependency);

		PublicDependencyModuleNames.AddRange(new[]
		    {
                "Core" // Core
                , "UMG" // UUserWidget creation
                , "EnhancedInput" // Created UMyInputAction, UMyInputMappingContext
                , "DeveloperSettings" // Created UDataAssetsContainer
                //My modules
                , "FunctionPicker" // Created properties in UMyInputAction
                , "MetaCheatManager" // Created UMyCheatManager
                , "PoolManager" // Created property in FMapComponentSpec
                , "MyUtils" // Inherited from Base classes
		    }
		);

		PrivateDependencyModuleNames.AddRange(new[]
			{
				"CoreUObject", "Engine", "Slate", "SlateCore" // Core
				, "NetCore" // Network: FFastArraySerializer, PushModel, Iris
				, "InputCore" // FKey
				, "RHI", "ApplicationCore" // Resolutions
				, "AIModule" // AI
				, "Niagara" // VFX
				, "GameplayTags" // FGameplayTag
                , "GameFeatures", "ModularGameplay" // Modular Game Features
                , "ModelViewViewModel" // MVVM UI pattern
				//My modules
				, "SettingsWidgetConstructor" // Generates settings
			}
		);

		if (Target.bBuildEditor)
		{
			// Include Editor modules that are used in this Runtime module
			PrivateDependencyModuleNames.AddRange(new[]
				{
					"UnrealEd" // FEditorDelegates
					//My modules
					, "BomberEditor" // UMyUnrealEdEngine
					, "MyEditorUtils" // FEditorUtilsLibrary
				}
			);
		}
	}
}
