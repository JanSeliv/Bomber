// Copyright 2021 Yevhenii Selivanov.

using UnrealBuildTool;

public class BomberEditor : ModuleRules
{
	public BomberEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core", "CoreUObject", "Engine", "InputCore", // Default
			"UnrealEd", // FEditorDelegates::EndPIE
			"Slate", "SlateCore", "PropertyEditor", "EditorStyle" // Property types customizations
		});

		PublicDependencyModuleNames.AddRange(new[] {"Bomber"});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		//Include Public and Private folder paths
		PublicIncludePaths.AddRange(
			new[]
			{
				"BomberEditor/Public"
			});

		PrivateIncludePaths.AddRange(
			new[]
			{
				"BomberEditor/Private"
			});
	}
}
