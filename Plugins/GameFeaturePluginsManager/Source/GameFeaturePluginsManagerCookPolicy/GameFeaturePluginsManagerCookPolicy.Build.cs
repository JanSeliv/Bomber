// Copyright (c) Yevhenii Selivanov.

using UnrealBuildTool;

public class GameFeaturePluginsManagerCookPolicy : ModuleRules
{
	public GameFeaturePluginsManagerCookPolicy(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		CppCompileWarningSettings.NonInlinedGenCppWarningLevel = WarningLevel.Error;

		// Minimal core-only dependencies so this module loads at an early phase, before the External Data Layer engine subsystem resolves its injection policy
		PrivateDependencyModuleNames.AddRange(new[]
			{
				"Core", "CoreUObject", "Engine"
			}
		);
	}
}
