// Copyright (c) Yevhenii Selivanov.

using UnrealBuildTool;

public class DataAssetsLoaderEditor : ModuleRules
{
	public DataAssetsLoaderEditor(ReadOnlyTargetRules Target) : base(Target)
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
				, "UnrealEd"
				, "Kismet", "KismetCompiler"
				, "DataAssetsLoader"
				, "DataRegistry" // K2Node_ListenForDataRegistryRowd
			}
		);
	}
}
