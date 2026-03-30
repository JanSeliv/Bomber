// Copyright (c) Yevhenii Selivanov.

using UnrealBuildTool;

public class MyUtilsNodes : ModuleRules
{
	public MyUtilsNodes(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		CppCompileWarningSettings.NonInlinedGenCppWarningLevel = WarningLevel.Error;

		PublicDependencyModuleNames.AddRange(new[]
			{
				"Core"
				, "BlueprintGraph" // UK2Node
			}
		);

		PrivateDependencyModuleNames.AddRange(new[]
			{
				"CoreUObject", "Engine", "Slate", "SlateCore" // Core
				, "UnrealEd", "Kismet", "KismetCompiler"
				, "GameplayTags", "GameplayAbilities", "AsyncMessageSystem" // UK2Node_CallOrListenForGlobalMessage
				// My modules
				, "MyUtils" // UGlobalMessageSubsystem
			}
		);
	}
}