// Copyright (c) Yevhenii Selivanov.

using UnrealBuildTool;

public class PlayAreaSurrounderRuntime : ModuleRules
{
	public PlayAreaSurrounderRuntime(ReadOnlyTargetRules Target) : base(Target)
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
				, "Bomber"
			}
		);
	}
}
