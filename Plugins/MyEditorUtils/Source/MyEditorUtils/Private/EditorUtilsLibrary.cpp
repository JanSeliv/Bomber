// Copyright (c) Yevhenii Selivanov

#include "EditorUtilsLibrary.h"
//---
#include "Editor.h"
#include "LevelEditor.h"
#include "SLevelViewport.h"
#include "UnrealEdGlobals.h"
#include "CookOnTheSide/CookOnTheFlyServer.h"
#include "Editor/EditorEngine.h"
#include "Editor/UnrealEdEngine.h"
#include "EditorFramework/AssetImportData.h"
#include "Misc/FileHelper.h"

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

// Returns true if currently is cooking the package
bool UEditorUtilsLibrary::IsCooking()
{
	if (IsEditorNotPieWorld())
	{
		const UCookOnTheFlyServer* CookServer = GUnrealEd ? GUnrealEd->CookServer : nullptr;
		return CookServer ? CookServer->IsCookByTheBookMode() : true;
	}
	return false;
}

// Returns current editor viewport
FViewport* UEditorUtilsLibrary::GetEditorViewport()
{
	const FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
	const TSharedPtr<ILevelEditor> LevelEditor = LevelEditorModule.GetFirstLevelEditor();
	if (!LevelEditor)
	{
		return nullptr;
	}

	for (const TSharedPtr<SLevelViewport>& LevelViewportIt : LevelEditor->GetViewports())
	{
		const FEditorViewportClient* EditorViewport = LevelViewportIt ? LevelViewportIt->GetViewportClient().Get() : nullptr;
		FViewport* Viewport = EditorViewport ? EditorViewport->Viewport : nullptr;
		if (Viewport && Viewport->GetSizeXY() != FIntPoint::ZeroValue)
		{
			return Viewport;
		}
	}

	return nullptr;
}

// Exports specified data table to already its .json
void UEditorUtilsLibrary::ReExportTableAsJSON(const UDataTable* DataTable)
{
	const UAssetImportData* AssetImportData = DataTable ? DataTable->AssetImportData : nullptr;
	if (!AssetImportData)
	{
		return;
	}

	const FString CurrentFilename = AssetImportData->GetFirstFilename();
	if (!CurrentFilename.IsEmpty())
	{
		const FString TableAsJSON = DataTable->GetTableAsJSON(EDataTableExportFlags::UseJsonObjectsForStructs);
		FFileHelper::SaveStringToFile(TableAsJSON, *CurrentFilename);
	}
}

// Removes all custom assets from context menu
void UEditorUtilsLibrary::UnregisterAssets(TArray<TSharedPtr<FAssetTypeActions_Base>>& RegisteredAssets)
{
	static const FName AssetToolsModuleName = TEXT("AssetTools");
	const FAssetToolsModule* AssetToolsPtr = FModuleManager::GetModulePtr<FAssetToolsModule>(AssetToolsModuleName);
	if (!AssetToolsPtr)
	{
		return;
	}

	IAssetTools& AssetTools = AssetToolsPtr->Get();
	for (TSharedPtr<FAssetTypeActions_Base>& AssetTypeActionIt : RegisteredAssets)
	{
		if (AssetTypeActionIt.IsValid())
		{
			AssetTools.UnregisterAssetTypeActions(AssetTypeActionIt.ToSharedRef());
		}
	}
}
