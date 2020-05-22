// Copyright 1998-2020 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class RiderBlueprintExtension : ModuleRules
{
	public RiderBlueprintExtension(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		bUseRTTI = true;

		PrivateDependencyModuleNames.AddRange(new []
		{
			"Core",
			"SlateCore",
			"RD",
			"RiderLink",
			"Slate",
			"AssetRegistry",
			"MessagingCommon",
			"UnrealEd",
			"UnrealEdMessages",
			"Engine",
			"CoreUObject"
		});
	}
}
