// Copyright (c) Yevhenii Selivanov

#pragma once

#include "PasTurnType.generated.h"

/**
 * Specifies side of play area surrounder.
 * ══════1═════════╗
 * ╔═════1═════╗   ║
 * ║   ╔═1═╗   ║   ║
 * ║   ║   2   2   2
 * 4   4   ╨   ║   ║
 * ║   ╚═══3═══╝   ║
 * ╚═══════3═══════╝
 * 1: HorizontalRight
 * 2: VerticalDown
 * 3: HorizontalLeft
 * 4: VerticalUp.
 */
UENUM(BlueprintType)
enum class EPasTurnType : uint8
{
	None,
	HorizontalRight,
	VerticalDown,
	HorizontalLeft,
	VerticalUp
};
