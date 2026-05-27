// Copyright (c) Yevhenii Selivanov.

using UnrealBuildTool;

public class PlayAreaSurrounderRuntime : ModuleRules
{
	public PlayAreaSurrounderRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		CppCompileWarningSettings.NonInlinedGenCppWarningLevel = WarningLevel.Error;

		PublicDependencyModuleNames.AddRange(new[]
			{
				"Core"
				// My modules
				, "Bomber"
			}
		);

		PrivateDependencyModuleNames.AddRange(new[]
			{
				"CoreUObject", "Engine", "Slate", "SlateCore" // Core
				, "GameplayTags" // FGameplayTag
				, "GameplayAbilities" // FGameplayEventData
				// My modules
				, "DataAssetsLoader" // Created UPlayAreaSurrounderData
				, "MyUtils" // UGlobalMessageSubsystem
			}
		);
	}
}
