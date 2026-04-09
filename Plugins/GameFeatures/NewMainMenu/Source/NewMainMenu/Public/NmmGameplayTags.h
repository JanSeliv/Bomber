// Copyright (c) Yevhenii Selivanov.

#pragma once

// UE
#include "NativeGameplayTags.h" // UE_DECLARE_GAMEPLAY_TAG_EXTERN

namespace NmmGameplayTags
{
	namespace UI
	{
		/** Widget tag for the Main Menu idle state widget */
		NEWMAINMENU_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Widget_Menu);

		/** Widget tag for the Main Menu cinematic state widget */
		NEWMAINMENU_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Widget_Cinematic);
	} // namespace UI

	namespace Menu
	{
		/** Skeletal mesh actor should own this tag, used to prevent initializing menu spots on other skeletal mesh actors, like from cinematics */
		NEWMAINMENU_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Spot);
	} // namespace Menu

	namespace Event
	{
		/** Event that fires when Main Menu is ready to display basic menu, is called on local player ASC */
		NEWMAINMENU_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MenuReady);

		/** Event that fires when the New Main Menu state was changed, InstigatorTags contains the new FNmmStateTag */
		NEWMAINMENU_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MenuStateChanged);

		/** Event that fires when user clicked the Play button in Main Menu, is called on local player ASC */
		NEWMAINMENU_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(PlayButtonPressed);

		/** Event that fires when user skipped the pre-game cinematic manually, is called on local player ASC */
		NEWMAINMENU_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(CinematicSkipped);

		/** Event that fires when pre-game cinematic Level Sequence finished playing naturally, is called on local player ASC */
		NEWMAINMENU_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(CinematicPlaybackFinished);

		/** Event that fires when New Main Menu game feature is being unloaded, is called on local player ASC */
		NEWMAINMENU_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MenuUnloaded);
	} // namespace Event
} // namespace NmmGameplayTags
