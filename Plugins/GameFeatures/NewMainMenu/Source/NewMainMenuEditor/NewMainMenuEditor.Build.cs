 // Copyright (c) Yevhenii Selivanov.

using UnrealBuildTool;

public class NewMainMenuEditor : ModuleRules
{
	public NewMainMenuEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		CppCompileWarningSettings.NonInlinedGenCppWarningLevel = WarningLevel.Error;

		PublicDependencyModuleNames.AddRange(new[]
			{
				"Core"
				, "GameplayTagsEditor" // Created FNmmStateTagCustomization
			}
		);

		PrivateDependencyModuleNames.AddRange(new[]
			{
				"CoreUObject", "Engine", "Slate", "SlateCore" // Core
				, "PropertyEditor" // IPropertyTypeCustomization
				// My modules
				, "NewMainMenu" // FNmmStateTag
			}
		);
	}
}
