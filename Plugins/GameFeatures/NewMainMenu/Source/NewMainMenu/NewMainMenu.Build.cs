// Copyright (c) Yevhenii Selivanov.

using UnrealBuildTool;

public class NewMainMenu : ModuleRules
{
	public NewMainMenu(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		CppCompileWarningSettings.NonInlinedGenCppWarningLevel = WarningLevel.Error;

		PublicDependencyModuleNames.AddRange(new[]
			{
				"Core", "Engine"
				, "UMG" // UUserWidget creation
				// Bomber modules
				, "Bomber"
				, "DataAssetsLoader" // Created UNMMDataAsset
				, "MetaCheatManager" // UNMMCheatExtension
			}
		);

		PrivateDependencyModuleNames.AddRange(new[]
			{
				"CoreUObject", "Slate", "SlateCore" // Core
				, "MovieScene", "LevelSequence" // Cinematics
				, "AdvancedWidgets" // URadialSlider
				, "CineCameraRigs" // Camera rails
				, "GameplayTags", "GameplayAbilities" // Tags
				, "ModelViewViewModel" // UNmmMVVM_MenuViewModel
				// Bomber modules
				, "MyUtils"
				, "SettingsWidgetConstructor"
			}
		);
	}
}