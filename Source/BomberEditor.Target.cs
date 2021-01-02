// Copyright 2021 Yevhenii Selivanov.

using UnrealBuildTool;

public class BomberEditorTarget : TargetRules
{
    public BomberEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V2;
        ExtraModuleNames.AddRange(new[] {"Bomber", "BomberEditor"});
    }
}
