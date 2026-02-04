// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

// UE
#include "Templates/Function.h"
#include "UObject/StrongObjectPtr.h"

#include "AsyncLoadUtilsLibrary.generated.h"

/**
 * Function library with PIE-safe async loading helpers.
 * Ensures callbacks are executed in the correct world context, even in PIE multiplayer.
 */
UCLASS()
class MYUTILS_API UAsyncLoadUtilsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Async loads a soft object with PIE-safe world context recovery.
	 * Uses weak binding, callback won't fire if object is destroyed. */
	template <typename T, typename U>
	static void AsyncLoadAsset(U* Object, const TSoftObjectPtr<T>& SoftObjectPtr, void (U::*Callback)(T*), TAsyncLoadPriority Priority = 0)
	{
		AsyncLoadAssetByPath(Object, SoftObjectPtr.ToSoftObjectPath(), [Object, Callback](UObject* LoadedObject)
		{
			(Object->*Callback)(Cast<T>(LoadedObject));
		}, Priority);
	}

	/** Async loads a soft object by path with PIE-safe world context recovery.
	 * Uses weak binding to WorldContextObject, callback won't fire if object is destroyed. */
	static void AsyncLoadAssetByPath(const UObject* WorldContextObject, const FSoftObjectPath& SoftObjectPath, TFunction<void(UObject*)> Callback, TAsyncLoadPriority Priority = 0);
};

/** Wrapper for AsyncTask to return to the game thread with PIE-safe world context recovery.
 * Is in global scope to mimic AsyncTask function signature. */
MYUTILS_API void AsyncTaskGameThread(const UObject* WorldContextObject, TFunction<void()> Function);

/*********************************************************************************************
 * PIE-Safe Callback Wrapper
 * Wraps any callback to ensure it executes in the correct world context in PIE multiplayer.
 * In editor: defers one frame via SetTimerForNextTick, uses TStrongObjectPtr for UObject* args.
 * In non-editor: executes callback directly.
 ********************************************************************************************* */

namespace PIESafeAsync
{
	/** Returns PIE-safe wrapped callback for void functions. */
	MYUTILS_API TFunction<void()> MakePIESafeCallback(const UObject* WorldContextObject, TFunction<void()> Callback);

	/** Executes callback with PIE-safe dispatch: defers in editor, direct call otherwise. */
	MYUTILS_API void ExecutePIESafe(UObject* WorldObject, TFunction<void()> Callback);

	/** Returns the world from context object as UObject for weak pointer storage, nullptr if invalid. */
	MYUTILS_API UObject* GetWorldObject(const UObject* WorldContextObject);

	/** Returns PIE-safe wrapped callback that protects UObject from GC during frame delay. */
	template <typename T>
	TFunction<void(T*)> MakePIESafeCallback(const UObject* WorldContextObject, TFunction<void(T*)> Callback)
	{
		TWeakObjectPtr<UObject> WeakWorld(GetWorldObject(WorldContextObject));
		auto SharedCallback = MakeShared<TFunction<void(T*)>>(MoveTemp(Callback));

		return [WeakWorld, SharedCallback](T* Object)
		{
			ExecutePIESafe(WeakWorld.Get(), [SharedCallback, StrongObject = TStrongObjectPtr<T>(Object)]()
			{
				(*SharedCallback)(StrongObject.Get());
			});
		};
	}

	/** Returns PIE-safe wrapped callback from delegate. */
	template <typename T, typename DelegateType>
	TFunction<void(T*)> MakePIESafeCallback(const UObject* WorldContextObject, const DelegateType& Delegate)
	{
		return MakePIESafeCallback<T>(WorldContextObject, TFunction<void(T*)>([Delegate](T* Obj)
		{
			Delegate.Execute(Obj);
		}));
	}

} // namespace PIESafeAsync
