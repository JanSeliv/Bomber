// Copyright 2019 Yevhenii Selivanov.

using UnrealBuildTool;
using System.Collections.Generic;

public class BomberTarget : TargetRules
{
    public BomberTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;

        ExtraModuleNames.AddRange(new string[] { "Bomber" });
    }
}
