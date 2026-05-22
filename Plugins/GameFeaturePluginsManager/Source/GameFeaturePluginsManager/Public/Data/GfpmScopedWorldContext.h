// Copyright (c) Yevhenii Selivanov

#pragma once

#if WITH_EDITOR
#include "Templates/UniquePtr.h"
#endif // WITH_EDITOR

/**
 * RAII scope guard restoring GWorld and (editor-only) PIE ID to point at given world for current scope.
 * Required when control enters from engine callsites that iterate world contexts without per-world scope switch, so synchronous work running afterward inside this scope resolves to correct world via global context.
 */
struct GAMEFEATUREPLUGINSMANAGER_API FGfpmScopedWorldContext
{
	/** When entering scope, saves current globals and switches to given world. */
	explicit FGfpmScopedWorldContext(class UWorld* InWorld);

	/** When exiting scope, restores prior globals. */
	~FGfpmScopedWorldContext();

private:
	class UWorld* OldGWorld = nullptr;
#if WITH_EDITOR
	TUniquePtr<struct FTemporaryPlayInEditorIDOverride> PlayInEditorIDOverride;
#endif // WITH_EDITOR
};
