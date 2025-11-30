// Copyright (c) Yevhenii Selivanov.

using UnrealBuildTool;

public class Bomber : ModuleRules
{
	public Bomber(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		CppCompileWarningSettings.NonInlinedGenCppWarningLevel = WarningLevel.Error;

		// Network: enable Iris replication
		const bool bAddAsPublicDependency = true; // Created FMapComponentsContainer
		SetupIrisSupport(Target, bAddAsPublicDependency);

		PublicDependencyModuleNames.AddRange(new[]
		    {
                "Core" // Core
                , "UMG" // UUserWidget creation
                , "EnhancedInput" // Created UBmrInputAction, UBmrInputMappingContext
                , "DeveloperSettings" // Created UBmrDataAssetsContainer
                , "GameFeatures" // Inherited IGameFeatureStateChangeObserver
                , "GameplayAbilities", "GameplayTags", "GameplayTasks" // Gameplay Ability System (GAS)
                , "Mover" // Created UBmrMoverComponent, UBmrMoverWalkingMode
                //My modules
                , "FunctionPicker" // Created properties in UBmrInputAction
                , "MetaCheatManager" // Created UBmrCheatManager
                , "PoolManager" // Created property in FBmrMapComponentSpec
                , "MyUtils" // Inherited from Base classes
		    }
		);

		PrivateDependencyModuleNames.AddRange(new[]
			{
				"CoreUObject", "Engine", "Slate", "SlateCore" // Core
				, "NetCore" // Network: FFastArraySerializer, PushModel, Iris
				, "OnlineSubsystem", "OnlineSubsystemUtils" // Online Sessions: create, destroy, join
				, "InputCore" // FKey
				, "AdvancedWidgets" // Widgets (URadialSlider etc)
				, "RHI", "ApplicationCore" // Resolutions
				, "AIModule" // AI
				, "Niagara" // VFX
				, "GameplayTags" // FGameplayTag
                , "ModularGameplay" // Modular Game Features (MGF)
                , "ModelViewViewModel" // MVVM UI pattern
				//My modules
				, "SettingsWidgetConstructor" // Generates settings
				, "AdvancedSessions", "AdvancedSteamSessions" // Steam
			}
		);

		if (Target.bBuildEditor)
		{
			// Include Editor modules that are used in this Runtime module
			PrivateDependencyModuleNames.AddRange(new[]
				{
					"UnrealEd" // FEditorDelegates
					//My modules
					, "BomberEditor" // UBmrUnrealEdEngine
					, "MyEditorUtils" // FEditorUtilsLibrary
				}
			);
		}
	}
}
