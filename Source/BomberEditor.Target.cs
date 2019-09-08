// Copyright 2019 Yevhenii Selivanov.

using UnrealBuildTool;
using System.Collections.Generic;

public class BomberEditorTarget : TargetRules
{
	public BomberEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;

		ExtraModuleNames.AddRange( new string[] { "Bomber" } );
	}
}
