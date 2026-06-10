// Copyright (c) Yevhenii Selivanov

#include "DalGameFeatureAction_AddDataRegistrySource.h"

// DAL
#include "DalUtilsLibrary.h"

// UE
#include "DataRegistry.h"
#include "DataRegistryId.h"
#include "DataRegistrySubsystem.h"
#include "Engine/CurveTable.h"
#include "Engine/DataTable.h"
#include "GameFeaturesProjectPolicies.h"
#include "GameFeaturesSubsystem.h"

#if WITH_EDITORONLY_DATA
#include "AssetRegistry/AssetBundleData.h"
#include "GameFeaturesSubsystemSettings.h"
#endif // WITH_EDITORONLY_DATA

#if WITH_EDITOR
#include "AssetRegistry/AssetData.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Misc/DataValidation.h"
#endif // WITH_EDITOR

#include UE_INLINE_GENERATED_CPP_BY_NAME(DalGameFeatureAction_AddDataRegistrySource)

// Called by Game Features system when owning feature is registered
void UDalGameFeatureAction_AddDataRegistrySource::OnGameFeatureRegistering()
{
	Super::OnGameFeatureRegistering();

	BeginSourcesForPhase(/*bPreloadPhase*/true);
}

// Called by Game Features system when owning feature is unregistered
void UDalGameFeatureAction_AddDataRegistrySource::OnGameFeatureUnregistering()
{
	EndSourcesForPhase(/*bPreloadPhase*/true);

	Super::OnGameFeatureUnregistering();
}

// Called by Game Features system when owning feature transitions to Active
void UDalGameFeatureAction_AddDataRegistrySource::OnGameFeatureActivating()
{
	Super::OnGameFeatureActivating();

	BeginSourcesForPhase(/*bPreloadPhase*/false);
}

// Called by Game Features system when owning feature is leaving Active state
void UDalGameFeatureAction_AddDataRegistrySource::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	EndSourcesForPhase(/*bPreloadPhase*/false);

	Super::OnGameFeatureDeactivating(Context);
}

#if WITH_EDITORONLY_DATA
// Called when owning GameFeatureData rebuilds its asset bundles on save
void UDalGameFeatureAction_AddDataRegistrySource::AddAdditionalAssetBundleData(FAssetBundleData& AssetBundleData)
{
	Super::AddAdditionalAssetBundleData(AssetBundleData);
	for (const FDalDataRegistrySource& Source : SourcesToAdd)
	{
		// Mirrors engine UGameFeatureAction_DataRegistrySource: bundle tables for client/server preload during feature loading, so apply never pays runtime load
		if (!Source.DataTableToAdd.IsNull())
		{
			const FTopLevelAssetPath DataTableSourcePath = Source.DataTableToAdd.ToSoftObjectPath().GetAssetPath();
			if (Source.bClientSource)
			{
				AssetBundleData.AddBundleAsset(UGameFeaturesSubsystemSettings::LoadStateClient, DataTableSourcePath);
			}
			if (Source.bServerSource)
			{
				AssetBundleData.AddBundleAsset(UGameFeaturesSubsystemSettings::LoadStateServer, DataTableSourcePath);
			}
		}

		if (!Source.CurveTableToAdd.IsNull())
		{
			const FTopLevelAssetPath CurveTableSourcePath = Source.CurveTableToAdd.ToSoftObjectPath().GetAssetPath();
			if (Source.bClientSource)
			{
				AssetBundleData.AddBundleAsset(UGameFeaturesSubsystemSettings::LoadStateClient, CurveTableSourcePath);
			}
			if (Source.bServerSource)
			{
				AssetBundleData.AddBundleAsset(UGameFeaturesSubsystemSettings::LoadStateServer, CurveTableSourcePath);
			}
		}
	}
}
#endif // WITH_EDITORONLY_DATA

#if WITH_EDITOR
// Called by editor Data Validation system when this asset is validated
EDataValidationResult UDalGameFeatureAction_AddDataRegistrySource::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	// Registry-resolvability checks are only meaningful once registries are loaded, skip them during
	// cook or early init to avoid false failures when no registry is available yet
	const UDataRegistrySubsystem* DataRegistrySubsystem = UDataRegistrySubsystem::Get();
	const bool bRegistriesReady = DataRegistrySubsystem && DataRegistrySubsystem->AreRegistriesInitialized();

	if (SourcesToAdd.IsEmpty())
	{
		Result = EDataValidationResult::Invalid;
		static const FString Tmpl = TEXT("SourcesToAdd is empty, action will be a no-op");
		Context.AddError(FText::FromString(Tmpl));
	}

	int32 EntryIndex = 0;
	const IAssetRegistry* AssetRegistry = IAssetRegistry::Get();
	if (!AssetRegistry)
	{
		// Asset registry not available, cannot validate table assets
		return Result;
	}

	for (const FDalDataRegistrySource& Entry : SourcesToAdd)
	{
		if (!Entry.DataRegistry)
		{
			Result = EDataValidationResult::Invalid;
			static const FString Tmpl = TEXT("DataRegistry row struct is not set at index {0} in SourcesToAdd");
			const FString Formatted = FString::Format(*Tmpl, {EntryIndex});
			Context.AddError(FText::FromString(Formatted));
		}
		else if (bRegistriesReady && !UDalUtilsLibrary::GetRegistryForStruct(Entry.DataRegistry))
		{
			Result = EDataValidationResult::Invalid;
			static const FString Tmpl = TEXT("Row struct {0} does not map to any Data Registry at index {1} in SourcesToAdd");
			const FString Formatted = FString::Format(*Tmpl, {Entry.DataRegistry->GetName(), EntryIndex});
			Context.AddError(FText::FromString(Formatted));
		}

		if (Entry.CurveTableToAdd.IsNull() && Entry.DataTableToAdd.IsNull())
		{
			Result = EDataValidationResult::Invalid;
			static const FString Tmpl = TEXT("No valid data table or curve table specified at index {0} in SourcesToAdd");
			const FString Formatted = FString::Format(*Tmpl, {EntryIndex});
			Context.AddError(FText::FromString(Formatted));
		}

		if (!Entry.CurveTableToAdd.IsNull())
		{
			const FAssetData CurveAssetData = AssetRegistry->GetAssetByObjectPath(Entry.CurveTableToAdd.ToSoftObjectPath());
			if (!CurveAssetData.IsValid() || !CurveAssetData.AssetClassPath.GetAssetName().ToString().Contains(TEXT("CurveTable")))
			{
				Result = EDataValidationResult::Invalid;
				static const FString Tmpl = TEXT("Path {0} does not point to valid curvetable at index {1} in SourcesToAdd");
				const FString Formatted = FString::Format(*Tmpl, {Entry.CurveTableToAdd.ToString(), EntryIndex});
				Context.AddError(FText::FromString(Formatted));
			}
		}

		if (!Entry.DataTableToAdd.IsNull())
		{
			const FAssetData DataAssetData = AssetRegistry->GetAssetByObjectPath(Entry.DataTableToAdd.ToSoftObjectPath());
			if (!DataAssetData.IsValid() || !DataAssetData.AssetClassPath.GetAssetName().ToString().Contains(TEXT("DataTable")))
			{
				Result = EDataValidationResult::Invalid;
				static const FString Tmpl = TEXT("Path {0} does not point to valid datatable at index {1} in SourcesToAdd");
				const FString Formatted = FString::Format(*Tmpl, {Entry.DataTableToAdd.ToString(), EntryIndex});
				Context.AddError(FText::FromString(Formatted));
			}
			else if (Entry.DataRegistry)
			{
				// Catch data table whose row struct does not match chosen target registry struct, it would resolve to wrong registry at runtime
				static const FName RowStructureTagName = TEXT("RowStructure");
				FString RowStructPath;
				DataAssetData.GetTagValue(RowStructureTagName, RowStructPath);
				const FString ExpectedPath = Entry.DataRegistry->GetPathName();
				const bool bRowStructMatches = RowStructPath.Equals(ExpectedPath) || RowStructPath.Contains(Entry.DataRegistry->GetName());
				if (!RowStructPath.IsEmpty() && !bRowStructMatches)
				{
					Result = EDataValidationResult::Invalid;
					static const FString Tmpl = TEXT("Data table {0} row struct {1} does not match target DataRegistry {2} at index {3} in SourcesToAdd");
					const FString Formatted = FString::Format(*Tmpl, {Entry.DataTableToAdd.ToString(), RowStructPath, ExpectedPath, EntryIndex});
					Context.AddError(FText::FromString(Formatted));
				}
			}
		}

		if (!Entry.bServerSource && !Entry.bClientSource)
		{
			Result = EDataValidationResult::Invalid;
			static const FString Tmpl = TEXT("Source is not specified to load on either client or server at index {0} in SourcesToAdd");
			const FString Formatted = FString::Format(*Tmpl, {EntryIndex});
			Context.AddError(FText::FromString(Formatted));
		}

		if (bRegistriesReady && Entry.OptionalDependsOnDataRegistry && !UDalUtilsLibrary::GetRegistryForStruct(Entry.OptionalDependsOnDataRegistry))
		{
			Result = EDataValidationResult::Invalid;
			static const FString Tmpl = TEXT("Optional dependency row struct {0} does not map to any Data Registry at index {1} in SourcesToAdd");
			const FString Formatted = FString::Format(*Tmpl, {Entry.OptionalDependsOnDataRegistry->GetName(), EntryIndex});
			Context.AddError(FText::FromString(Formatted));
		}

		++EntryIndex;
	}

	return Result;
}
#endif // WITH_EDITOR

// Begins subset of sources matching given phase: applies ungated ones and subscribes gated ones
void UDalGameFeatureAction_AddDataRegistrySource::BeginSourcesForPhase(bool bPreloadPhase)
{
	if (!ensureMsgf(!SourcesToAdd.IsEmpty(), TEXT("ASSERT: [%i] %hs:\n'SourcesToAdd' is empty!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	// Size lifecycle bitsets once, both phases share them since registration always precedes activation
	if (BegunFlags.Num() != SourcesToAdd.Num())
	{
		BegunFlags.Init(false, SourcesToAdd.Num());
		AppliedFlags.Init(false, SourcesToAdd.Num());
	}

	bool bAnyBegun = false;
	for (int32 Index = 0; Index < SourcesToAdd.Num(); ++Index)
	{
		const FDalDataRegistrySource& Source = SourcesToAdd[Index];
		if (Source.bPreloadOnRegistered != bPreloadPhase
		    || BegunFlags[Index])
		{
			// Belongs to other lifecycle phase, or already begun
			continue;
		}

		BegunFlags[Index] = true;
		bAnyBegun = true;
	}

	if (!bAnyBegun)
	{
		// No source began this phase, nothing to resolve
		return;
	}

	// Defer apply (ungated and gated alike) until registries are initialized: on clients registry
	// objects may not be loaded yet at activation, so resolving target registry from row struct
	// would fail, engine action sidesteps this by storing registry name and scheduling instead
	UDataRegistrySubsystem* DataRegistrySubsystem = UDataRegistrySubsystem::Get();
	if (!ensureMsgf(DataRegistrySubsystem, TEXT("ASSERT: [%i] %hs:\n'DataRegistrySubsystem' is null!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	if (DataRegistrySubsystem->AreRegistriesInitialized())
	{
		ResolveAndSubscribeAll();
		return;
	}

	if (!OnSubsystemInitHandle.IsValid())
	{
		OnSubsystemInitHandle = DataRegistrySubsystem->OnSubsystemInitialized().AddUObject(this, &ThisClass::OnDataRegistrySubsystemInitialized);
	}
}

// Ends subset of sources matching given phase: removes applied ones and drops gate subscriptions when no gated source remains
void UDalGameFeatureAction_AddDataRegistrySource::EndSourcesForPhase(bool bPreloadPhase)
{
	for (int32 Index = 0; Index < SourcesToAdd.Num(); ++Index)
	{
		if (!BegunFlags.IsValidIndex(Index)
		    || !BegunFlags[Index]
		    || SourcesToAdd[Index].bPreloadOnRegistered != bPreloadPhase)
		{
			// Not begun, or belongs to other lifecycle phase
			continue;
		}

		BegunFlags[Index] = false;

		// Clear applied state for gated and ungated alike, stale flag would skip re-apply on next activation
		if (AppliedFlags[Index])
		{
			RemoveEntry(Index);
			AppliedFlags[Index] = false;
		}
	}

	if (HasBegunGatedSources())
	{
		// Other phase still keeps gated sources alive, leave shared subscriptions in place
		return;
	}

	ClearAllRegistrySubscriptions();

	if (UDataRegistrySubsystem* DataRegistrySubsystem = UDataRegistrySubsystem::Get())
	{
		DataRegistrySubsystem->OnSubsystemInitialized().Remove(OnSubsystemInitHandle);
	}
	OnSubsystemInitHandle.Reset();
}

// Hook bound to UDataRegistrySubsystem::OnSubsystemInitialized when phase begins before registries are ready
void UDalGameFeatureAction_AddDataRegistrySource::OnDataRegistrySubsystemInitialized()
{
	if (UDataRegistrySubsystem* DataRegistrySubsystem = UDataRegistrySubsystem::Get())
	{
		DataRegistrySubsystem->OnSubsystemInitialized().Remove(OnSubsystemInitHandle);
	}
	OnSubsystemInitHandle.Reset();

	ResolveAndSubscribeAll();
}

// Applies begun ungated sources and subscribes begun gated sources to their upstream registry cache invalidation, then runs immediate evaluation
void UDalGameFeatureAction_AddDataRegistrySource::ResolveAndSubscribeAll()
{
	TSet<const UScriptStruct*> SeenStructs;
	for (int32 Index = 0; Index < SourcesToAdd.Num(); ++Index)
	{
		if (!BegunFlags[Index])
		{
			// Source not yet begun in any phase, skip
			continue;
		}

		const FDalDataRegistrySource& Source = SourcesToAdd[Index];
		if (!Source.OptionalDependsOnDataRegistry)
		{
			// Ungated source: apply once now that registries are initialized, idempotent
			if (!AppliedFlags[Index])
			{
				AppliedFlags[Index] = ApplyEntry(Index);
			}
			continue;
		}

		const UScriptStruct* DependsStruct = Source.OptionalDependsOnDataRegistry;
		bool bAlreadySeen = false;
		SeenStructs.Add(DependsStruct, &bAlreadySeen);
		if (bAlreadySeen)
		{
			// Already subscribed this upstream registry in current pass, skip duplicate
			continue;
		}

		UDataRegistry* Registry = UDalUtilsLibrary::GetRegistryForStruct(DependsStruct);
		if (!Registry
		    || RegistryHandles.Contains(Registry))
		{
			// Registry unresolvable or already subscribed, skip
			continue;
		}

		const FDelegateHandle Handle = Registry->OnCacheVersionInvalidated().AddUObject(this, &ThisClass::OnRegistryCacheInvalidated);
		RegistryHandles.Emplace(Registry, Handle);
	}

	EvaluateAllEntries();
}

// Hook bound to each upstream UDataRegistry::OnCacheVersionInvalidated, defers per-entry evaluation on cache change
void UDalGameFeatureAction_AddDataRegistrySource::OnRegistryCacheInvalidated(UDataRegistry* /*InRegistry*/)
{
	EvaluateAllEntries();
}

// Re-checks every begun gated entry: applies on rising edge (rows become non-empty), removes on falling edge
void UDalGameFeatureAction_AddDataRegistrySource::EvaluateAllEntries()
{
	for (int32 Index = 0; Index < SourcesToAdd.Num(); ++Index)
	{
		if (!BegunFlags[Index])
		{
			// Source not yet begun in any phase, skip
			continue;
		}

		const FDalDataRegistrySource& Source = SourcesToAdd[Index];
		if (!Source.OptionalDependsOnDataRegistry)
		{
			// Ungated sources are applied or removed at phase begin/end, not on cache changes
			continue;
		}

		const int32 RowsNum = UDalUtilsLibrary::GetRegistryRowsNum(Source.OptionalDependsOnDataRegistry);
		const bool bShouldBeApplied = RowsNum > 0;
		const bool bIsApplied = AppliedFlags[Index];

		if (bShouldBeApplied && !bIsApplied)
		{
			AppliedFlags[Index] = ApplyEntry(Index);
		}
		else if (!bShouldBeApplied && bIsApplied)
		{
			RemoveEntry(Index);
			AppliedFlags[Index] = false;
		}
	}
}

// Adds source to its target registry, registry resolved from its row struct, honoring client/server flags
bool UDalGameFeatureAction_AddDataRegistrySource::ApplyEntry(int32 EntryIndex)
{
	UDataRegistrySubsystem* DataRegistrySubsystem = UDataRegistrySubsystem::Get();
	if (!ensureMsgf(DataRegistrySubsystem, TEXT("ASSERT: [%i] %hs:\n'DataRegistrySubsystem' is null!"), __LINE__, __FUNCTION__))
	{
		return false;
	}

	bool bIsClient = false;
	bool bIsServer = false;
	const UGameFeaturesProjectPolicies& Policy = UGameFeaturesSubsystem::Get().GetPolicy<UGameFeaturesProjectPolicies>();
	Policy.GetGameFeatureLoadingMode(bIsClient, bIsServer);

	const FDalDataRegistrySource& Source = SourcesToAdd[EntryIndex];
	const bool bShouldAdd = (bIsServer && Source.bServerSource) || (bIsClient && Source.bClientSource);
	if (!bShouldAdd)
	{
		// Excluded by active client/server loading mode
		return false;
	}

	const FDataRegistryType RegistryType = GetRegistryTypeForStruct(Source.DataRegistry);
	if (!ensureMsgf(RegistryType.IsValid(), TEXT("ASSERT: [%i] %hs:\n'RegistryType' is none, row struct does not map to any Data Registry!"), __LINE__, __FUNCTION__))
	{
		return false;
	}

	TMap<FDataRegistryType, TArray<FSoftObjectPath>> AssetMap;
	TArray<FSoftObjectPath>& AssetList = AssetMap.Add(RegistryType);

	if (!Source.DataTableToAdd.IsNull())
	{
		AssetList.Add(Source.DataTableToAdd.ToSoftObjectPath());
	}

	if (!Source.CurveTableToAdd.IsNull())
	{
		AssetList.Add(Source.CurveTableToAdd.ToSoftObjectPath());
	}

#if !UE_BUILD_SHIPPING
	// Mirrors engine UGameFeatureAction_DataRegistrySource: late application pays load hitch unless asset was preloaded
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
#endif // !UE_BUILD_SHIPPING

	DataRegistrySubsystem->PreregisterSpecificAssets(AssetMap, Source.AssetPriority);
	return true;
}

// Removes source from its target registry, mirroring ApplyEntry
void UDalGameFeatureAction_AddDataRegistrySource::RemoveEntry(int32 EntryIndex)
{
	UDataRegistrySubsystem* DataRegistrySubsystem = UDataRegistrySubsystem::Get();
	if (!DataRegistrySubsystem)
	{
		// Subsystem already torn down (PIE shutdown), applied state already invalid
		return;
	}

	const FDalDataRegistrySource& Source = SourcesToAdd[EntryIndex];
	const FDataRegistryType RegistryType = GetRegistryTypeForStruct(Source.DataRegistry);
	if (!RegistryType.IsValid())
	{
		// Row struct no longer maps to any Data Registry, nothing to unregister
		return;
	}

	if (!Source.DataTableToAdd.IsNull())
	{
		if (!DataRegistrySubsystem->UnregisterSpecificAsset(RegistryType, Source.DataTableToAdd.ToSoftObjectPath()))
		{
			UE_LOG(LogTemp, Log, TEXT("RemoveEntry %s: DataRegistry data table %s failed to unregister"), *GetPathName(), *Source.DataTableToAdd.ToString());
		}
	}

	if (!Source.CurveTableToAdd.IsNull())
	{
		if (!DataRegistrySubsystem->UnregisterSpecificAsset(RegistryType, Source.CurveTableToAdd.ToSoftObjectPath()))
		{
			UE_LOG(LogTemp, Log, TEXT("RemoveEntry %s: DataRegistry curve table %s failed to unregister"), *GetPathName(), *Source.CurveTableToAdd.ToString());
		}
	}
}

// Drops every per-registry cache subscription held by this action
void UDalGameFeatureAction_AddDataRegistrySource::ClearAllRegistrySubscriptions()
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

// Returns true when at least one gated source is currently begun, so its registry subscriptions must stay alive
bool UDalGameFeatureAction_AddDataRegistrySource::HasBegunGatedSources() const
{
	for (int32 Index = 0; Index < SourcesToAdd.Num(); ++Index)
	{
		if (BegunFlags.IsValidIndex(Index)
		    && BegunFlags[Index]
		    && SourcesToAdd[Index].OptionalDependsOnDataRegistry)
		{
			// Found active gated source, subscriptions must stay alive
			return true;
		}
	}

	return false;
}

// Resolves target registry type for source row struct via DAL, or none type when unresolved
FDataRegistryType UDalGameFeatureAction_AddDataRegistrySource::GetRegistryTypeForStruct(const UScriptStruct* InStruct)
{
	const UDataRegistry* Registry = UDalUtilsLibrary::GetRegistryForStruct(InStruct);
	return Registry ? FDataRegistryType(Registry->GetRegistryType()) : FDataRegistryType();
}
