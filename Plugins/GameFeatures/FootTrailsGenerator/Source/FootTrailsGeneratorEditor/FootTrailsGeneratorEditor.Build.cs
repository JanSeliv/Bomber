// Copyright (c) Yevhenii Selivanov.

using UnrealBuildTool;

public class FootTrailsGeneratorEditor : ModuleRules
{
    public FootTrailsGeneratorEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        CppStandard = CppStandardVersion.Latest;
        bEnableNonInlinedGenCppWarnings = true;

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
                // My modules
                , "FootTrailsGeneratorRuntime" // UFTGComponent
                , "Bomber" // AGeneratedMap
                , "MyEditorUtils" // FEditorUtilsLibrary
            }
        );
    }
}
