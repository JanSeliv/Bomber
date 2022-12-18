// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class NewAIRuntime : ModuleRules
{
	public NewAIRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Latest;

		PublicDependencyModuleNames.AddRange(new[]
			{
				"Core"
				, "Bomber" // Is included in header files
				, "AIModule" // is AI system, can implements AI classes
			}
		);

		PrivateDependencyModuleNames.AddRange(new[]
			{
				"CoreUObject", "Engine", "Slate", "SlateCore" // Core
			}
		);
	}
}
