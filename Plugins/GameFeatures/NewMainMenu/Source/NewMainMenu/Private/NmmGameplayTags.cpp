// Copyright (c) Yevhenii Selivanov.

#include "NmmGameplayTags.h"

namespace NmmGameplayTags
{
	namespace UI
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Widget_Menu, "UI.Widget.NewMainMenu.Menu", "Widget tag for the Main Menu idle state widget");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Widget_Cinematic, "UI.Widget.NewMainMenu.Cinematic", "Widget tag for the Main Menu cinematic state widget");
	} // namespace UI

	namespace Menu
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Spot, "NMM.Spot", "Skeletal mesh actor should own this tag, used to prevent initializing menu spots on other skeletal mesh actors, like from cinematics");
	} // namespace Menu

	namespace Event
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(MenuReady, "Event.NewMainMenu.MenuReady", "Event that fires when Main Menu is ready to display basic menu, is called on local player ASC");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(MenuStateChanged, "Event.NewMainMenu.MenuStateChanged", "Event that fires when the New Main Menu state was changed, InstigatorTags contains the new FNmmStateTag");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(PlayButtonPressed, "Event.NewMainMenu.PlayButtonPressed", "Event that fires when user clicked the Play button in Main Menu, is called on local player ASC");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(CinematicSkipped, "Event.NewMainMenu.CinematicSkipped", "Event that fires when user skipped the pre-game cinematic manually, is called on local player ASC");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(CinematicPlaybackFinished, "Event.NewMainMenu.CinematicPlaybackFinished", "Event that fires when pre-game cinematic Level Sequence finished playing naturally, is called on local player ASC");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(MenuUnloaded, "Event.NewMainMenu.Unloaded", "Event that fires when New Main Menu game feature is being unloaded, is called on local player ASC");
	} // namespace Event
} // namespace NmmGameplayTags
