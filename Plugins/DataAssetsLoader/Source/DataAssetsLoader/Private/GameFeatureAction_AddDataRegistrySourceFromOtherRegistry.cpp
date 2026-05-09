// Copyright (c) Yevhenii Selivanov

#include "GameFeatureAction_AddDataRegistrySourceFromOtherRegistry.h"

// DAL
#include "DalUtilsLibrary.h"

// UE
#include "DataRegistry.h"
#include "DataRegistrySubsystem.h"
#include "Engine/CurveTable.h"
#include "Engine/DataTable.h"
#include "GameFeaturesProjectPolicies.h"
#include "GameFeaturesSubsystem.h"

#if WITH_EDITOR
#include "AssetRegistry/AssetData.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Misc/DataValidation.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameFeatureAction_AddDataRegistrySourceFromOtherRegistry)

// @TODO JanSeliv: remove debug logs after fixing registry-dependency drop trace
namespace RegDepDebug
{
	static void LogImpl(ELogVerbosity::Type Verbosity, const UObject* Owner, const ANSICHAR* Callers, const TCHAR* Extra)
	{
		const UWorld* World = (Owner && GEngine) ? GEngine->GetWorldFromContextObject(Owner, EGetWorldErrorMode::ReturnNull) : nullptr;
		const FString WorldType = World ? LexToString(World->WorldType) : TEXT("None");
		static const TCHAR* NetModeNames[] = {TEXT("Standalone"), TEXT("DedicatedServer"), TEXT("ListenServer"), TEXT("Client")};
		const ENetMode NetModeValue = World ? World->GetNetMode() : NM_MAX;
		const FString NetMode = NetModeValue < NM_MAX ? NetModeNames[NetModeValue] : TEXT("None");
		const FString Message = FString::Printf(TEXT("[REGDEP]%s Owner=%s, World=%s, WorldType=%s, NetMode=%s | %hs"),
			Extra, *GetNameSafe(Owner), *GetNameSafe(World),
			*WorldType, *NetMode, Callers);
		if (Verbosity <= ELogVerbosity::Warning)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("%s"), *Message);
		}
	}

	static void LogOwner(const UObject* Owner, const ANSICHAR* Callers, const TCHAR* Extra = TEXT(""))
	{
		LogImpl(ELogVerbosity::Log, Owner, Callers, Extra);
	}

	static void WarnOwner(const UObject* Owner, const ANSICHAR* Callers, const TCHAR* Extra = TEXT(""))
	{
		LogImpl(ELogVerbosity::Warning, Owner, Callers, Extra);
	}
} // namespace RegDepDebug

#define LOCTEXT_NAMESPACE "GameFeatureAction_AddDataRegistrySourceFromOtherRegistry"

// Called by the Game Features system when the owning feature transitions to Active
void UGameFeatureAction_AddDataRegistrySourceFromOtherRegistry::OnGameFeatureActivated()
{
	if (!ensureMsgf(RegistryHandles.IsEmpty() && AppliedFlags.IsEmpty() && !OnSubsystemInitHandle.IsValid(), TEXT("ASSERT: [%i] %hs:\n'RegistryHandles', 'AppliedFlags' or 'OnSubsystemInitHandle' is not empty, attempting to activate already active feature!"), __LINE__, __FUNCTION__))
	{
		ClearAllRegistrySubscriptions();
		RemoveAllApplied();
	}

	Super::OnGameFeatureActivating();

	if (!ensureMsgf(!SourcesToAdd.IsEmpty(), TEXT("ASSERT: [%i] %hs:\n'SourcesToAdd' is empty!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	AppliedFlags.Init(false, SourcesToAdd.Num());

	UDataRegistrySubsystem* DataRegistrySubsystem = UDataRegistrySubsystem::Get();
	if (!ensureMsgf(DataRegistrySubsystem, TEXT("ASSERT: [%i] %hs:\n'DataRegistrySubsystem' is null!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	RegDepDebug::LogOwner(this, __FUNCTION__, *FString::Printf(TEXT(", Entries=%d, RegistriesInit=%d, Subsystem=%s"), SourcesToAdd.Num(), DataRegistrySubsystem->AreRegistriesInitialized() ? 1 : 0, *GetNameSafe(DataRegistrySubsystem))); // @TODO JanSeliv: remove after fixing registry-dependency drop trace

	if (DataRegistrySubsystem->AreRegistriesInitialized())
	{
		RegDepDebug::LogOwner(this, __FUNCTION__, TEXT(", Path=Init, calling ResolveAndSubscribeAll directly")); // @TODO JanSeliv: remove after fixing registry-dependency drop trace
		ResolveAndSubscribeAll();
		return;
	}

	RegDepDebug::LogOwner(this, __FUNCTION__, TEXT(", Path=Deferred, subscribing OnSubsystemInitialized")); // @TODO JanSeliv: remove after fixing registry-dependency drop trace
	OnSubsystemInitHandle = DataRegistrySubsystem->OnSubsystemInitialized().AddUObject(this, &ThisClass::OnDataRegistrySubsystemInitialized);
}

// Called by the Game Features system when the owning feature is leaving the Active state
void UGameFeatureAction_AddDataRegistrySourceFromOtherRegistry::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	RegDepDebug::LogOwner(this, __FUNCTION__, *FString::Printf(TEXT(", Entries=%d, Handles=%d, AppliedFlags=%d"), SourcesToAdd.Num(), RegistryHandles.Num(), AppliedFlags.Num())); // @TODO JanSeliv: remove after fixing registry-dependency drop trace

	if (UDataRegistrySubsystem* DataRegistrySubsystem = UDataRegistrySubsystem::Get())
	{
		DataRegistrySubsystem->OnSubsystemInitialized().Remove(OnSubsystemInitHandle);
	}
	OnSubsystemInitHandle.Reset();

	ClearAllRegistrySubscriptions();
	RemoveAllApplied();
	AppliedFlags.Empty();

	Super::OnGameFeatureDeactivating(Context);
}

#if WITH_EDITOR
// Reports configuration errors to the editor's Data Validation system
EDataValidationResult UGameFeatureAction_AddDataRegistrySourceFromOtherRegistry::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (SourcesToAdd.IsEmpty())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(LOCTEXT("EmptySourcesToAdd", "SourcesToAdd is empty, action will be a no-op"));
	}

	int32 EntryIndex = 0;
	for (const FDalDataRegistrySourceWithDependency& Entry : SourcesToAdd)
	{
		if (!Entry.DependsOnRowStruct)
		{
			Result = EDataValidationResult::Invalid;
			Context.AddError(FText::Format(LOCTEXT("MissingDepStruct", "DependsOnRowStruct is not set at index {0} in SourcesToAdd"), FText::AsNumber(EntryIndex)));
		}

		if (Entry.Source.RegistryToAddTo.IsNone())
		{
			Result = EDataValidationResult::Invalid;
			Context.AddError(FText::Format(LOCTEXT("InvalidRegistry", "Source specifies an invalid name (NONE) as the target registry at index {0} in SourcesToAdd"), FText::AsNumber(EntryIndex)));
		}

		if (Entry.Source.CurveTableToAdd.IsNull() && Entry.Source.DataTableToAdd.IsNull())
		{
			Result = EDataValidationResult::Invalid;
			Context.AddError(FText::Format(LOCTEXT("MissingSource", "No valid data table or curve table specified at index {0} in SourcesToAdd"), FText::AsNumber(EntryIndex)));
		}

		if (!Entry.Source.CurveTableToAdd.IsNull())
		{
			const FAssetData CurveAssetData = IAssetRegistry::Get()->GetAssetByObjectPath(Entry.Source.CurveTableToAdd.ToSoftObjectPath());
			if (!CurveAssetData.IsValid() || !CurveAssetData.AssetClassPath.GetAssetName().ToString().Contains(TEXT("CurveTable")))
			{
				Result = EDataValidationResult::Invalid;
				Context.AddError(FText::Format(LOCTEXT("MissingCurveTable", "Path {0} does not point to valid curvetable at index {1} in SourcesToAdd"), FText::FromString(Entry.Source.CurveTableToAdd.ToString()), FText::AsNumber(EntryIndex)));
			}
		}

		if (!Entry.Source.DataTableToAdd.IsNull())
		{
			const FAssetData DataAssetData = IAssetRegistry::Get()->GetAssetByObjectPath(Entry.Source.DataTableToAdd.ToSoftObjectPath());
			if (!DataAssetData.IsValid() || !DataAssetData.AssetClassPath.GetAssetName().ToString().Contains(TEXT("DataTable")))
			{
				Result = EDataValidationResult::Invalid;
				Context.AddError(FText::Format(LOCTEXT("MissingDataTable", "Path {0} does not point to valid datatable at index {1} in SourcesToAdd"), FText::FromString(Entry.Source.DataTableToAdd.ToString()), FText::AsNumber(EntryIndex)));
			}
		}

		if (!Entry.Source.bServerSource && !Entry.Source.bClientSource)
		{
			Result = EDataValidationResult::Invalid;
			Context.AddError(FText::Format(LOCTEXT("NeverUsed", "Source is not specified to load on either client or server at index {0} in SourcesToAdd"), FText::AsNumber(EntryIndex)));
		}

		++EntryIndex;
	}

	return Result;
}
#endif

// Hook bound to UDataRegistrySubsystem::OnSubsystemInitialized when activation occurs before registries are ready
void UGameFeatureAction_AddDataRegistrySourceFromOtherRegistry::OnDataRegistrySubsystemInitialized()
{
	RegDepDebug::LogOwner(this, __FUNCTION__, TEXT(", DRSubsystem now initialized, proceeding to ResolveAndSubscribeAll")); // @TODO JanSeliv: remove after fixing registry-dependency drop trace

	if (UDataRegistrySubsystem* DataRegistrySubsystem = UDataRegistrySubsystem::Get())
	{
		DataRegistrySubsystem->OnSubsystemInitialized().Remove(OnSubsystemInitHandle);
	}
	OnSubsystemInitHandle.Reset();

	ResolveAndSubscribeAll();
}

// Walks unique upstream row structs, resolves each to its registry via DAL and subscribes to its cache invalidation
void UGameFeatureAction_AddDataRegistrySourceFromOtherRegistry::ResolveAndSubscribeAll()
{
	TSet<TObjectPtr<UScriptStruct>> SeenStructs;
	for (const FDalDataRegistrySourceWithDependency& Entry : SourcesToAdd)
	{
		if (!Entry.DependsOnRowStruct)
		{
			continue;
		}

		bool bAlreadySeen = false;
		SeenStructs.Add(Entry.DependsOnRowStruct, &bAlreadySeen);
		if (bAlreadySeen)
		{
			continue;
		}

		UDataRegistry* Registry = UDalUtilsLibrary::GetRegistryForStruct(Entry.DependsOnRowStruct);
		RegDepDebug::LogOwner(this, __FUNCTION__, *FString::Printf(TEXT(", DepStruct=%s, Registry=%s"), *GetNameSafe(Entry.DependsOnRowStruct), *GetNameSafe(Registry))); // @TODO JanSeliv: remove after fixing registry-dependency drop trace
		if (!Registry)
		{
			continue;
		}

		const FDelegateHandle Handle = Registry->OnCacheVersionInvalidated().AddUObject(this, &ThisClass::OnRegistryCacheInvalidated);
		RegistryHandles.Emplace(Registry, Handle);
	}

	RegDepDebug::LogOwner(this, __FUNCTION__, *FString::Printf(TEXT(", Subscribed=%d, runningSnapshotEvaluate"), RegistryHandles.Num())); // @TODO JanSeliv: remove after fixing registry-dependency drop trace
	EvaluateAllEntries();
}

// Hook bound to each upstream UDataRegistry::OnCacheVersionInvalidated
void UGameFeatureAction_AddDataRegistrySourceFromOtherRegistry::OnRegistryCacheInvalidated(UDataRegistry* /*InRegistry*/)
{
	RegDepDebug::LogOwner(this, __FUNCTION__, TEXT(", cache version invalidated, running EvaluateAllEntries (per-entry RowsNum will reveal which dep changed)")); // @TODO JanSeliv: remove after fixing registry-dependency drop trace
	EvaluateAllEntries();
}

// Re-checks every entry, edge-triggering Apply on rising edge and Remove on falling edge
void UGameFeatureAction_AddDataRegistrySourceFromOtherRegistry::EvaluateAllEntries()
{
	for (int32 Index = 0; Index < SourcesToAdd.Num(); ++Index)
	{
		const FDalDataRegistrySourceWithDependency& Entry = SourcesToAdd[Index];
		if (!Entry.DependsOnRowStruct)
		{
			continue;
		}

		const int32 RowsNum = UDalUtilsLibrary::GetRegistryRowsNum(Entry.DependsOnRowStruct);
		const bool bShouldBeApplied = RowsNum > 0;
		const bool bIsApplied = AppliedFlags[Index];
		RegDepDebug::LogOwner(this, __FUNCTION__, *FString::Printf(TEXT(", Index=%d, DepStruct=%s, RowsNum=%d, ShouldBeApplied=%d, IsApplied=%d"), Index, *GetNameSafe(Entry.DependsOnRowStruct), RowsNum, bShouldBeApplied ? 1 : 0, bIsApplied ? 1 : 0)); // @TODO JanSeliv: remove after fixing registry-dependency drop trace

		if (bShouldBeApplied && !bIsApplied)
		{
			ApplyEntry(Index);
			AppliedFlags[Index] = true;
		}
		else if (!bShouldBeApplied && bIsApplied)
		{
			RemoveEntry(Index);
			AppliedFlags[Index] = false;
		}
	}
}

// Applies the entry's Source to the Data Registry honoring per-entry client/server flags
void UGameFeatureAction_AddDataRegistrySourceFromOtherRegistry::ApplyEntry(int32 EntryIndex)
{
	RegDepDebug::LogOwner(this, __FUNCTION__, *FString::Printf(TEXT(", Index=%d, Target=%s, DataTable=%s, CurveTable=%s"), EntryIndex, *SourcesToAdd[EntryIndex].Source.RegistryToAddTo.ToString(), *SourcesToAdd[EntryIndex].Source.DataTableToAdd.ToString(), *SourcesToAdd[EntryIndex].Source.CurveTableToAdd.ToString())); // @TODO JanSeliv: remove after fixing registry-dependency drop trace

	UDataRegistrySubsystem* DataRegistrySubsystem = UDataRegistrySubsystem::Get();
	if (!ensureMsgf(DataRegistrySubsystem, TEXT("ASSERT: [%i] %hs:\n'DataRegistrySubsystem' is null!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	bool bIsClient = false;
	bool bIsServer = false;
	UGameFeaturesProjectPolicies& Policy = UGameFeaturesSubsystem::Get().GetPolicy<UGameFeaturesProjectPolicies>();
	Policy.GetGameFeatureLoadingMode(bIsClient, bIsServer);

	const FDataRegistrySourceToAdd& Source = SourcesToAdd[EntryIndex].Source;
	const bool bShouldAdd = (bIsServer && Source.bServerSource) || (bIsClient && Source.bClientSource);
	if (!bShouldAdd)
	{
		return;
	}

	TMap<FDataRegistryType, TArray<FSoftObjectPath>> AssetMap;
	TArray<FSoftObjectPath>& AssetList = AssetMap.Add(Source.RegistryToAddTo);

	if (!Source.DataTableToAdd.IsNull())
	{
		AssetList.Add(Source.DataTableToAdd.ToSoftObjectPath());
	}

	if (!Source.CurveTableToAdd.IsNull())
	{
		AssetList.Add(Source.CurveTableToAdd.ToSoftObjectPath());
	}

#if !UE_BUILD_SHIPPING
	// Mirrors engine UGameFeatureAction_DataRegistrySource: late application pays a load hitch unless asset was preloaded
	if (DataRegistrySubsystem->AreRegistriesInitialized())
	{
		if (!Source.DataTableToAdd.IsNull() && !Source.DataTableToAdd.IsValid())
		{
			UE_LOG(LogTemp, Log, TEXT("ApplyEntry %s: DataRegistry source asset %s was not loaded before activation, this may cause a long hitch"), *GetPathName(), *Source.DataTableToAdd.ToString());
		}

		if (!Source.CurveTableToAdd.IsNull() && !Source.CurveTableToAdd.IsValid())
		{
			UE_LOG(LogTemp, Log, TEXT("ApplyEntry %s: DataRegistry source asset %s was not loaded before activation, this may cause a long hitch"), *GetPathName(), *Source.CurveTableToAdd.ToString());
		}
	}
#endif

	DataRegistrySubsystem->PreregisterSpecificAssets(AssetMap, Source.AssetPriority);
}

// Removes the entry's Source from the Data Registry
void UGameFeatureAction_AddDataRegistrySourceFromOtherRegistry::RemoveEntry(int32 EntryIndex)
{
	RegDepDebug::LogOwner(this, __FUNCTION__, *FString::Printf(TEXT(", Index=%d, Target=%s, DataTable=%s, CurveTable=%s"), EntryIndex, *SourcesToAdd[EntryIndex].Source.RegistryToAddTo.ToString(), *SourcesToAdd[EntryIndex].Source.DataTableToAdd.ToString(), *SourcesToAdd[EntryIndex].Source.CurveTableToAdd.ToString())); // @TODO JanSeliv: remove after fixing registry-dependency drop trace

	UDataRegistrySubsystem* DataRegistrySubsystem = UDataRegistrySubsystem::Get();
	if (!DataRegistrySubsystem)
	{
		// Subsystem already torn down (PIE shutdown); applied state already invalid
		return;
	}

	const FDataRegistrySourceToAdd& Source = SourcesToAdd[EntryIndex].Source;

	if (!Source.DataTableToAdd.IsNull())
	{
		if (!DataRegistrySubsystem->UnregisterSpecificAsset(Source.RegistryToAddTo, Source.DataTableToAdd.ToSoftObjectPath()))
		{
			UE_LOG(LogTemp, Log, TEXT("RemoveEntry %s: DataRegistry data table %s failed to unregister"), *GetPathName(), *Source.DataTableToAdd.ToString());
		}
	}

	if (!Source.CurveTableToAdd.IsNull())
	{
		if (!DataRegistrySubsystem->UnregisterSpecificAsset(Source.RegistryToAddTo, Source.CurveTableToAdd.ToSoftObjectPath()))
		{
			UE_LOG(LogTemp, Log, TEXT("RemoveEntry %s: DataRegistry curve table %s failed to unregister"), *GetPathName(), *Source.CurveTableToAdd.ToString());
		}
	}
}

// Removes every entry whose flag is set
void UGameFeatureAction_AddDataRegistrySourceFromOtherRegistry::RemoveAllApplied()
{
	for (int32 Index = 0; Index < AppliedFlags.Num(); ++Index)
	{
		if (AppliedFlags[Index])
		{
			RemoveEntry(Index);
			AppliedFlags[Index] = false;
		}
	}
}

// Drops every per-registry cache subscription held by this action
void UGameFeatureAction_AddDataRegistrySourceFromOtherRegistry::ClearAllRegistrySubscriptions()
{
	for (auto It = RegistryHandles.CreateIterator(); It; ++It)
	{
		UDataRegistry* Registry = It->Key.Get();
		if (Registry)
		{
			Registry->OnCacheVersionInvalidated().Remove(It->Value);
		}
		It.RemoveCurrent();
	}
}

#undef LOCTEXT_NAMESPACE