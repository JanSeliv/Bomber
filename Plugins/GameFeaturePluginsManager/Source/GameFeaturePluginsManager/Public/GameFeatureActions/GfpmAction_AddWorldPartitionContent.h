// Copyright (c) Yevhenii Selivanov

#pragma once

#include "GameFeatureAction_AddWorldPartitionContent.h"

#include "GfpmAction_AddWorldPartitionContent.generated.h"

/**
 * Game Feature action that replaces engine Add World Partition Content action for runtime map switching of External Data Layer content.
 */
UCLASS(DisplayName = "GFPM Add World Partition Content")
class GAMEFEATUREPLUGINSMANAGER_API UGfpmAction_AddWorldPartitionContent final : public UGameFeatureAction_AddWorldPartitionContent
{
	GENERATED_BODY()

protected:
	/** Called by the Game Features system when the owning feature is registered. */
	virtual void OnGameFeatureRegistering() override;
};
