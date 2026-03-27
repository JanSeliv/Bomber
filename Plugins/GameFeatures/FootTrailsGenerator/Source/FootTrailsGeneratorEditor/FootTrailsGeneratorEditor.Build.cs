// Copyright (c) Yevhenii Selivanov.

using UnrealBuildTool;

public class FootTrailsGeneratorEditor : ModuleRules
{
    public FootTrailsGeneratorEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        CppCompileWarningSettings.NonInlinedGenCppWarningLevel = WarningLevel.Error;

        PublicDependencyModuleNames.AddRange(new[]
            {
                "Core"
                , "EditorSubsystem" // Created UFTGEditorSubsystem
            }
        );

        PrivateDependencyModuleNames.AddRange(new[]
            {
                "CoreUObject", "Engine" // Core
                , "GameFeatures" // UGameFeaturesSubsystem
                , "GameplayAbilities" // FGameplayEventData
                , "GameplayTags" // FGameplayTagContainer
                // My modules
                , "FootTrailsGeneratorRuntime" // UFTGComponent
                , "Bomber" // ABmrGeneratedMap
                , "MyUtils" // UUtilsLibrary
                , "InstancedStaticMeshConverter" // AInstancedStaticMeshActor
            }
        );
    }
}
