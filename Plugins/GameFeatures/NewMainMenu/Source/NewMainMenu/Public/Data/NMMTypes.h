// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "NMMTypes.generated.h"

/**
 * Represents the state of the Main Menu cinematics.
 */
UENUM(BlueprintType, DisplayName = "New Main Menu State")
enum class ENMMState : uint8
{
	///< Uninitialized
	None,
	///< Basic menu: gameplay camera, Play/Settings/Quit visible, no cinematics loaded
	BasicMenu,
	///< Camera moving between spots on rail
	Transition,
	///< Cinematic lobby: spot camera, character selection buttons, Part 0 loop
	Idle,
	///< Part 1+ playing after Play pressed
	Cinematic
};

/**
 * Represents the state of the camera rail state.
 */
UENUM(BlueprintType, DisplayName = "New Main Menu Camera Rail State")
enum class ENMMCameraRailTransitionState : uint8
{
	///< Transition is not started
	None,
	///< Camera moving between Spots started
	BeginTransition,
	///< Camera moving between Spots reached it's halfway
	HalfwayTransition,
	///< Camera moving between Spots finished transition
	EndTransition
};
