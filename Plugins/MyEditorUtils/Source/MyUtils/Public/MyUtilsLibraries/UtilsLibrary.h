// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
//---
#include "UtilsLibrary.generated.h"

enum EAspectRatioAxisConstraint : int;

/**
 * The common functions library
 */
UCLASS()
class MYUTILS_API UUtilsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/*********************************************************************************************
	 * Play In Editor
	 ********************************************************************************************* */
public:
	/** Returns the current play world.
	 * Will attempt to get world from World Context Object if specified. */
	static UWorld* GetPlayWorld(const UObject* OptionalWorldContext = nullptr);

	/** Checks, is the current world placed in the editor. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (DevelopmentOnly))
	static bool IsEditor();

	/** Checks is the current world placed in the editor and the game not started yet. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (DevelopmentOnly))
	static bool IsEditorNotPieWorld();

	/** Returns true if game is started in the Editor. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (DevelopmentOnly))
	static bool IsPIE();

	/** Returns true if is started multiplayer game (server + client(s)) right in the Editor. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (DevelopmentOnly))
	static bool IsEditorMultiplayer();

	/** Returns the index of current player during editor multiplayer.
	 * 0 is server.
	 * 1 (or higher) is client.
	 * -1 in the standalone game. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (DevelopmentOnly))
	static int32 GetEditorPlayerIndex();

	/** Returns true if game was started. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static bool HasWorldBegunPlay();

	/** Returns true if this instance is server. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static bool IsServer();

	/*********************************************************************************************
	 * Viewport
	 ********************************************************************************************* */
public:
	/** Returns true if viewport is initialized, is always true in PIE, but takes a while in builds. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static bool IsViewportInitialized();

	/** Returns the actual screen resolution.
	 * Is most reliable in comparisons with other ways get resolution like settings. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static FIntPoint GetViewportResolution();

	/** Returns 'MaintainYFOV' if Horizontal FOV is currently used while 'MaintainXFOV' for the Vertical one.*/
	UFUNCTION(BlueprintPure, Category = "C++")
	static TEnumAsByte<EAspectRatioAxisConstraint> GetViewportAspectRatioAxisConstraint();

	/*********************************************************************************************
	 * Gameplay
	 ********************************************************************************************* */
public:
	/** Abstract method that allows set both static and skeletal meshes to the specified mesh component.
	 * @param MeshComponent The mesh component to set mesh.
	 * @param MeshAsset The mesh asset to set to the mesh component. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	static void SetMesh(class UMeshComponent* MeshComponent, class UStreamableRenderAsset* MeshAsset);
};
