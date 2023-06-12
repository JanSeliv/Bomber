// Copyright (c) Yevhenii Selivanov.

using UnrealBuildTool;

public class FootTrailsGeneratorRuntime : ModuleRules
{
	public FootTrailsGeneratorRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Latest;

		PublicDependencyModuleNames.AddRange(new[]
			{
				"Core", "Engine"
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
