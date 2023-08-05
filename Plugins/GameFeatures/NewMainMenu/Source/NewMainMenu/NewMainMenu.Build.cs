// Copyright (c) Yevhenii Selivanov.

using UnrealBuildTool;

public class NewMainMenu : ModuleRules
{
	public NewMainMenu(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Latest;

		PublicDependencyModuleNames.AddRange(new[]
			{
				"Core", "Engine"
				, "UMG" // UUserWidget creation
			}
		);

		PrivateDependencyModuleNames.AddRange(new[]
			{
				"CoreUObject", "Slate", "SlateCore" // Core
				, "MovieScene", "LevelSequence" // Cinematics
				// My modules
				, "Bomber"
				, "MyUtils"
				, "SettingsWidgetConstructor"
			}
		);
	}
}