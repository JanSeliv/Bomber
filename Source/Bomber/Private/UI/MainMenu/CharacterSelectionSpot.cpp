// Copyright (c) Yevhenii Selivanov

#include "UI/MainMenu/CharacterSelectionSpot.h"
//---
#include "Controllers/MyPlayerController.h"
#include "UI/MyHUD.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
//---
#include "Camera/CameraActor.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(CharacterSelectionSpot)

void ACharacterSelectionSpot::SetCameraViewOnSpot(bool bBlend)
{
	AMyPlayerController* PC = UMyBlueprintFunctionLibrary::GetLocalPlayerController();
	if (!PC || !CameraActorInternal)
	{
		return;
	}

	const float BlendTime = bBlend ? 0.5f : 0.0f;
	PC->SetViewTargetWithBlend(CameraActorInternal, BlendTime);
}

// Overridable native event for when play begins for this actor.
void ACharacterSelectionSpot::BeginPlay()
{
	Super::BeginPlay();

	if (AMyHUD* MyHUD = UMyBlueprintFunctionLibrary::GetMyHUD())
	{
		MyHUD->AddCharacterSelectionSpot(this);
	}
}

// Sets camera view to this spot if current level type is equal to the spot's player
void ACharacterSelectionSpot::TrySetCameraViewByDefault()
{
	const ELevelType CurrentLevelType = UMyBlueprintFunctionLibrary::GetLevelType();
	const ELevelType PlayerByLevelType = GetMeshChecked().GetAssociatedLevelType();
	const bool bCanReadLevelType = CurrentLevelType != ELT::None && PlayerByLevelType != ELT::None;
	if (ensureMsgf(bCanReadLevelType, TEXT("'bCanReadLevelType' condition is FALSE, can not determine the level type for '%s' spot."), *GetNameSafe(this))
	    && CurrentLevelType == PlayerByLevelType)
	{
		constexpr bool bBlend = false;
		SetCameraViewOnSpot(bBlend);
	}
}
