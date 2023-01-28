// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ProgressionSystemRuntime : ModuleRules
{
	public ProgressionSystemRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Latest;

		PublicDependencyModuleNames.AddRange(new[]
			{
				"Core"
				, "Bomber" // Is included in header files
			}
		);

		PrivateDependencyModuleNames.AddRange(new[]
			{
				"CoreUObject", "Engine", "Slate", "SlateCore" // Core
			}
		);
	}
}
