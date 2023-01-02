// Copyright (c) Yevhenii Selivanov.

using UnrealBuildTool;

public class CustomShapeButton : ModuleRules
{
	public CustomShapeButton(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Latest;

		PublicDependencyModuleNames.AddRange(new[]
			{
				"Core"
				, "UMG" // Created UCustomShapeButton
				, "Slate" // Created SCustomShapeButton
			}
		);

		PrivateDependencyModuleNames.AddRange(new[]
			{
				"CoreUObject", "Engine", "Slate", "SlateCore" // Core
				, "RHI" // FRHITexture2D
				, "RenderCore" // Render threads
			}
		);
	}
}
