// Copyright (c) Yevhenii Selivanov.

using UnrealBuildTool;

public class NewAI : ModuleRules
{
	public NewAI(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Latest;
		bEnableNonInlinedGenCppWarnings = true;

		PublicDependencyModuleNames.AddRange(new[]
			{
				"Core"
				, "AIModule" // is AI system, can implements AI classes
				// My modules
				, "Bomber"
			}
		);

		PrivateDependencyModuleNames.AddRange(new[]
			{
				"CoreUObject", "Engine", "Slate", "SlateCore" // Core
				// My modules
				, "MyUtils"
			}
		);
	}
}
