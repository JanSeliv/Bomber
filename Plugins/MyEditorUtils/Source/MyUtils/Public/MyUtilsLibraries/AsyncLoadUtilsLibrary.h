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
	/** Async loads multiple assets, calls Listener member function when done.
	 * Uses weak UObject binding - callback is skipped if Listener is destroyed.
	 * @return Streamable handle for lifetime management, nullptr if PathsToLoad is empty */
	template <typename T>
	static FORCEINLINE TSharedPtr<struct FStreamableHandle> RequestAsyncLoad(const UObject* WorldContextObject, TArray<FSoftObjectPath> PathsToLoad, T* Listener, void (T::*OnComplete)()) { return RequestAsyncLoad(WorldContextObject, MoveTemp(PathsToLoad), TDelegate<void()>::CreateUObject(Listener, OnComplete)); }

	/** Async loads multiple assets and fires the delegate on completion.
	 * Defers callback to correct world tick in PIE, preventing cross-world interference.
	 * @param WorldContextObject - UObject for world context, weak-captured for lifetime safety
	 * @param PathsToLoad - All soft paths to load
	 * @return Streamable handle for lifetime management, nullptr if PathsToLoad is empty */
	static TSharedPtr<struct FStreamableHandle> RequestAsyncLoad(const UObject* WorldContextObject, TArray<FSoftObjectPath> PathsToLoad, TDelegate<void()> OnComplete);
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

	/** Executes callback with PIE-safe dispatch: defers in editor with weak context safety, direct call otherwise */
	MYUTILS_API void ExecutePIESafe(const UObject* ContextObject, TFunction<void()> Callback);

	/** Returns the world from context object as UObject for weak pointer storage, nullptr if invalid. */
	MYUTILS_API UObject* GetWorldObject(const UObject* WorldContextObject);

	/** Returns PIE-safe wrapped callback that protects UObject from GC during frame delay. */
	template <typename T>
	TFunction<void(T*)> MakePIESafeCallback(const UObject* WorldContextObject, TFunction<void(T*)> Callback)
	{
		const TWeakObjectPtr WeakContext(WorldContextObject);
		auto SharedCallback = MakeShared<TFunction<void(T*)>>(MoveTemp(Callback));

		return [WeakContext, SharedCallback](T* Object)
		{
			ExecutePIESafe(WeakContext.Get(), [SharedCallback, StrongObject = TStrongObjectPtr<T>(Object)]()
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
