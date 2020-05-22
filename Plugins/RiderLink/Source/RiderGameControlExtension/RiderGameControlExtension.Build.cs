// Copyright 1998-2020 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class RiderGameControlExtension : ModuleRules
{
	public RiderGameControlExtension(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		bUseRTTI = true;

		PrivateDependencyModuleNames.AddRange(new []
		{
			"Core",
			"RD",
			"RiderLink",
			"HeadMountedDisplay",
			"LevelEditor",
			"UnrealEd",
			"Slate",
			"CoreUObject",
			"Engine"
		});
	}
}
