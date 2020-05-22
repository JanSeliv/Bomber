// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class RiderLink : ModuleRules
{
	public RiderLink(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		bUseRTTI = true;

		PublicDependencyModuleNames.Add("Core");
		PublicDependencyModuleNames.Add("RD");
	}
}
