// Copyright (c) Yevhenii Selivanov

#include "Data/GfpmScopedWorldContext.h"

// UE
#include "Engine/World.h"
#include "UObject/Package.h"

#if WITH_EDITOR
#include "UnrealEngine.h" // FTemporaryPlayInEditorIDOverride
#endif // WITH_EDITOR

// When entering scope, saves current globals and switches to given world
FGfpmScopedWorldContext::FGfpmScopedWorldContext(UWorld* InWorld)
{
	OldGWorld = GWorld;
	if (InWorld)
	{
		GWorld = InWorld;
#if WITH_EDITOR
		const int32 NewPieID = InWorld->GetOutermost()->GetPIEInstanceID();
		// FTemporaryPlayInEditorIDOverride also keeps GPlayInEditorContextString in sync via engine's UpdatePlayInEditorWorldDebugString
		PlayInEditorIDOverride = MakeUnique<FTemporaryPlayInEditorIDOverride>(NewPieID);
#endif // WITH_EDITOR
	}
}

// When exiting scope, restores prior globals
FGfpmScopedWorldContext::~FGfpmScopedWorldContext()
{
#if WITH_EDITOR
	PlayInEditorIDOverride.Reset();
#endif // WITH_EDITOR
	GWorld = OldGWorld;
}
