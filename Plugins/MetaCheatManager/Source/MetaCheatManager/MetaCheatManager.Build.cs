// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MetaCheatManager : ModuleRules
{
	public MetaCheatManager(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Latest;

		PublicDependencyModuleNames.AddRange(new[]
			{
				"Core"
			}
		);

		PrivateDependencyModuleNames.AddRange(new[]
			{
				"CoreUObject", "Engine", "Slate", "SlateCore" // Core
				, "EngineSettings" // UConsoleSettings
			}
		);
	}
}
