// Copyright (c) Yevhenii Selivanov.

using UnrealBuildTool;

public class FootTrailsGeneratorRuntime : ModuleRules
{
	public FootTrailsGeneratorRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		CppCompileWarningSettings.NonInlinedGenCppWarningLevel = WarningLevel.Error;

		PublicDependencyModuleNames.AddRange(new[]
			{
				"Core", "Engine"
				, "DataAssetsLoader" // Created UFTGDataAsset
			}
		);

		PrivateDependencyModuleNames.AddRange(new[]
			{
				"CoreUObject", "Slate", "SlateCore" // Core
				// My modules
				, "Bomber"
				, "MyUtils"
				, "InstancedStaticMeshConverter",
			}
		);
	}
}
