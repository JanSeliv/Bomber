// Copyright (c) Yevhenii Selivanov

#include "EditorUtilsLibrary.h"
//---
#include "Editor.h"
#include "Editor/EditorEngine.h"

// Checks, is the current world placed in the editor
bool UEditorUtilsLibrary::IsEditor()
{
	return GIsEditor && GEditor && GWorld && GWorld->IsEditorWorld();
}

// Checks, that this actor placed in the editor world and the game is not started yet
bool UEditorUtilsLibrary::IsEditorNotPieWorld()
{
	return IsEditor() && !GEditor->IsPlaySessionInProgress();
}

// Returns true if game is started in the Editor
bool UEditorUtilsLibrary::IsPIE()
{
	return IsEditor() && GEditor->IsPlaySessionInProgress();
}

// Returns true if is started multiplayer game (server + client(s)) right in the Editor
bool UEditorUtilsLibrary::IsEditorMultiplayer()
{
	if (IsPIE())
	{
		const TOptional<FPlayInEditorSessionInfo>& PIEInfo = GEditor->GetPlayInEditorSessionInfo();
		return PIEInfo.IsSet() && PIEInfo->PIEInstanceCount > 0;
	}
	return false;
}

// Returns the index of current player during editor multiplayer
int32 UEditorUtilsLibrary::GetEditorPlayerIndex()
{
	if (!IsEditorMultiplayer())
	{
		return INDEX_NONE;
	}

	const UWorld* CurrentEditorWorld = GEditor->GetCurrentPlayWorld();
	if (!CurrentEditorWorld)
	{
		return INDEX_NONE;
	}

	int32 FoundAtIndex = INDEX_NONE;
	const TIndirectArray<FWorldContext>& WorldContexts = GEditor->GetWorldContexts();
	for (const FWorldContext& WorldContextIt : WorldContexts)
	{
		if (WorldContextIt.PIEInstance == INDEX_NONE)
		{
			continue;
		}

		++FoundAtIndex;

		const UWorld* WorldIt = WorldContextIt.World();
		if (WorldIt
			&& WorldIt == CurrentEditorWorld)
		{
			return FoundAtIndex;
		}
	}
	return INDEX_NONE;
}
