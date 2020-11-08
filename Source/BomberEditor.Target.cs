// Copyright 2020 Yevhenii Selivanov.

using UnrealBuildTool;
using System.Collections.Generic;

public class BomberEditorTarget : TargetRules
{
	public BomberEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.AddRange( new string[] { "Bomber" } );
	}
}
