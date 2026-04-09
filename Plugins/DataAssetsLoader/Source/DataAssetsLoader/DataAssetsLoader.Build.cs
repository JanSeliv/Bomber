// Copyright (c) Yevhenii Selivanov.

using UnrealBuildTool;

public class DataAssetsLoader : ModuleRules
{
	public DataAssetsLoader(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		CppCompileWarningSettings.NonInlinedGenCppWarningLevel = WarningLevel.Error;

		PublicDependencyModuleNames.AddRange(new[]
			{
				"Core"
				, "GameFeatures" // Inherited IGameFeatureStateChangeObserver
			}
		);

		PrivateDependencyModuleNames.AddRange(new[]
			{
				"CoreUObject", "Engine" // Core
				, "AssetRegistry" // FARFilter, FAssetData
				, "DataRegistry" // UDalRegistrySubsystem
			}
		);

		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.Add("UnrealEd"); // GEditor
		}
	}
}