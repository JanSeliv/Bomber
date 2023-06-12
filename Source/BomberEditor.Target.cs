// Copyright (c) Yevhenii Selivanov.

using UnrealBuildTool;

public class BomberEditorTarget : TargetRules
{
    public BomberEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        bBuildAllModules = true;
        ExtraModuleNames.AddRange(new[] {"Bomber", "BomberEditor"});
    }
}
