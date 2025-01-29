// Copyright (c) Yevhenii Selivanov.

using UnrealBuildTool;

public class BomberTarget : TargetRules
{
    public BomberTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        ExtraModuleNames.AddRange(new[] {"Bomber"});

		// Network: enable the Iris replication
		bUseIris = true;
    }
}
