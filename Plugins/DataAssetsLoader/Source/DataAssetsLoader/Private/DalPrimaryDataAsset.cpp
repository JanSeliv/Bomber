// Copyright (c) Yevhenii Selivanov

#include "DalPrimaryDataAsset.h"

// DAL
#include "DalSubsystem.h"

// UE
#include "Async/Async.h"
#include "Async/TaskGraphInterfaces.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DalPrimaryDataAsset)

// All subclasses share the same primary asset type so Asset Manager discovers them under a single type
FPrimaryAssetId UDalPrimaryDataAsset::GetPrimaryAssetId() const
{
	static const FPrimaryAssetType PrimaryAssetType(StaticClass()->GetFName());
	return FPrimaryAssetId(PrimaryAssetType, GetFName());
}

// Self-registers in UDalSubsystem regardless of how it was loaded: Asset Manager async loading, strong (hard) reference, synchronously etc
void UDalPrimaryDataAsset::PostLoad()
{
	Super::PostLoad();

	// Defers to finish loading, ProcessEvent is restricted during PostLoad
	AsyncTask(ENamedThreads::GameThread, [WeakThis = TWeakObjectPtr(this)]
	{
		if (const UDalPrimaryDataAsset* This = WeakThis.Get())
		{
			UDalSubsystem::Get().RegisterDataAsset(This->GetClass(), This);
		}
	});
}

// Unregisters this data asset from UDalSubsystem before destruction
void UDalPrimaryDataAsset::BeginDestroy()
{
	if (UDalSubsystem* Subsystem = UDalSubsystem::GetDataAssetsLoaderSubsystem())
	{
		Subsystem->UnregisterDataAsset(GetClass());
	}

	Super::BeginDestroy();
}
