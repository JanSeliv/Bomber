// Copyright (c) Yevhenii Selivanov.

using UnrealBuildTool;

public class Bomber : ModuleRules
{
	public Bomber(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Cpp17; // Fails on CppStandardVersion.Latest by Niagara

		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core", "CoreUObject", "Engine", "InputCore", // Default
			"HeadMountedDisplay", "UMG", "Slate", "SlateCore", // UMG
			"AIModule", // AI
			"GameplayTags", // Tags
			"RHI", "ApplicationCore", // Resolutions
			"EnhancedInput", // Enhanced Input plugin
			"Niagara", // VFX
			"RenderCore", // Render threads,
			"MyUtils" // FFunctionPicker etc.
		});

		if (Target.bBuildEditor)
		{
			// Include Editor modules that are used in this Runtime module
			PrivateDependencyModuleNames.AddRange(new[]
				{
					"UnrealEd", // FEditorDelegates
					"BomberEditor", // UMyUnrealEdEngine
					"Blutility", // UEditorUtilityLibrary::GetSelectionSet()
					"MyEditorUtils" // UEditorUtilsLibrary
				}
			);
		}
	}
}
