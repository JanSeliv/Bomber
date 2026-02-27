// Copyright (c) Yevhenii Selivanov

#include "MyUtilsLibraries/AsyncLoadUtilsLibrary.h"

// My Utils
#include "MyUtilsLibraries/UtilsLibrary.h"

// UE
#include "Engine/Engine.h"
#include "Engine/StreamableManager.h"
#include "Engine/World.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AsyncLoadUtilsLibrary)

/*********************************************************************************************
 * UAsyncLoadUtilsLibrary
 ********************************************************************************************* */

// Async loads a soft object by path with PIE-safe world context recovery
void UAsyncLoadUtilsLibrary::AsyncLoadAssetByPath(const UObject* WorldContextObject, const FSoftObjectPath& SoftObjectPath, TFunction<void(UObject*)> Callback, TAsyncLoadPriority Priority)
{
	if (!ensureMsgf(SoftObjectPath.IsValid(), TEXT("ASSERT: [%i] %hs:\n'SoftObjectPath' is not valid!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	FLoadSoftObjectPathAsyncDelegate Delegate;
	Delegate.BindWeakLambda(WorldContextObject, [SafeCallback = PIESafeAsync::MakePIESafeCallback<UObject>(WorldContextObject, MoveTemp(Callback))](const FSoftObjectPath&, UObject* LoadedObject)
	{
		SafeCallback(LoadedObject);
	});

	FLoadAssetAsyncOptionalParams Params;
	Params.PackagePriority = Priority;
	SoftObjectPath.LoadAsync(MoveTemp(Delegate), MoveTemp(Params));
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
	TWeakObjectPtr<UObject> WeakWorld(GetWorldObject(WorldContextObject));
	auto SharedCallback = MakeShared<TFunction<void()>>(MoveTemp(Callback));

	return [WeakWorld, SharedCallback]()
	{
		ExecutePIESafe(WeakWorld.Get(), *SharedCallback);
	};
}

void PIESafeAsync::ExecutePIESafe(UObject* WorldObject, TFunction<void()> Callback)
{
	if (!Callback)
	{
		return;
	}

#if WITH_EDITOR
	UWorld* World = Cast<UWorld>(WorldObject);
	if (UUtilsLibrary::IsEditor() && World)
	{
		World->GetTimerManager().SetTimerForNextTick(MoveTemp(Callback));
		return;
	}
#endif // WITH_EDITOR

	Callback();
}

UObject* PIESafeAsync::GetWorldObject(const UObject* WorldContextObject)
{
	return UUtilsLibrary::GetPlayWorld(WorldContextObject);
}
