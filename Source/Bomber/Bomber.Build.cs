// Copyright 2019 Yevhenii Selivanov.

using UnrealBuildTool;

public class Bomber : ModuleRules
{
    public Bomber(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core", "CoreUObject", "Engine", "InputCore", // Default
            "HeadMountedDisplay", "UMG", "Slate", "SlateCore", // UMG
            "AIModule" // AI
            });

        PrivateDependencyModuleNames.AddRange(new string[] { });

        if (Target.Type == TargetType.Editor)
        {
            PrivateDependencyModuleNames.AddRange(
                new string[]
                {
                    "UnrealEd" // FEditorDelegates::EndPIE
                });
        }

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
