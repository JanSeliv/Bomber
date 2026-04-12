// Copyright (c) Yevhenii Selivanov

#include "MyUtilsLibraries/AsyncLoadUtilsLibrary.h"

// My Utils
#include "MyUtilsLibraries/UtilsLibrary.h"

// UE
#include "Engine/AssetManager.h"
#include "Engine/Engine.h"
#include "Engine/StreamableManager.h"
#include "Engine/World.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AsyncLoadUtilsLibrary)

/*********************************************************************************************
 * UAsyncLoadUtilsLibrary
 ********************************************************************************************* */

// Async loads multiple assets and fires the delegate on completion with PIE-safe world context
TSharedPtr<FStreamableHandle> UAsyncLoadUtilsLibrary::RequestAsyncLoad(const UObject* WorldContextObject, TArray<FSoftObjectPath> PathsToLoad, TDelegate<void()> OnComplete)
{
	if (PathsToLoad.IsEmpty())
	{
		return nullptr;
	}

	FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();
	return StreamableManager.RequestAsyncLoad(MoveTemp(PathsToLoad), PIESafeAsync::MakePIESafeCallback(WorldContextObject, [OnComplete = MoveTemp(OnComplete)]()
	{
		OnComplete.ExecuteIfBound();
	}));
}

void AsyncTaskGameThread(const UObject* WorldContextObject, TFunction<void()> Function)
{
	if (!ensureMsgf(Function, TEXT("ASSERT: [%i] %hs:\n'Function' is not bound!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	AsyncTask(ENamedThreads::GameThread, PIESafeAsync::MakePIESafeCallback(WorldContextObject, MoveTemp(Function)));
}

/*********************************************************************************************
 * PIE-Safe Callback Wrapper
 ********************************************************************************************* */

TFunction<void()> PIESafeAsync::MakePIESafeCallback(const UObject* WorldContextObject, TFunction<void()> Callback)
{
	const TWeakObjectPtr WeakContext(WorldContextObject);
	const TSharedRef<TFunction<void()>> SharedCallback = MakeShared<TFunction<void()>>(MoveTemp(Callback));

	return [WeakContext, SharedCallback]()
	{
		ExecutePIESafe(WeakContext.Get(), *SharedCallback);
	};
}

void PIESafeAsync::ExecutePIESafe(const UObject* ContextObject, TFunction<void()> Callback)
{
	if (!Callback || !ContextObject)
	{
		return;
	}

#if WITH_EDITOR
	if (GIsEditor)
	{
		const UWorld* World = Cast<UWorld>(GetWorldObject(ContextObject));
		const bool bWorldUsable = World && !World->bIsTearingDown && !IsGarbageCollecting();
		if (!bWorldUsable)
		{
			return;
		}

		World->GetTimerManager().SetTimerForNextTick([WeakContext = TWeakObjectPtr(ContextObject), Callback = MoveTemp(Callback)]()
		{
			if (WeakContext.IsValid())
			{
				Callback();
			}
		});
		return;
	}
#endif // WITH_EDITOR

	Callback();
}

UObject* PIESafeAsync::GetWorldObject(const UObject* WorldContextObject)
{
	return UUtilsLibrary::GetPlayWorld(WorldContextObject);
}
