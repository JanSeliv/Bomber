// Copyright 1998-2020 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class RiderLoggingExtension : ModuleRules
{
	public RiderLoggingExtension(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		bUseRTTI = true;

		PrivateDependencyModuleNames.Add("Core");
		PrivateDependencyModuleNames.Add("RD");
		PrivateDependencyModuleNames.Add("RiderLink");
	}
}
