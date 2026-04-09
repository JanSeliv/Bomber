// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "NMMTypes.generated.h"

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
