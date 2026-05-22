// Copyright (c) Yevhenii Selivanov

#include "GameFeatureActions/GfpmAction_RegisterGameFeaturePluginActivation.h"

// GFPM
#include "Subsystems/GfpmLoaderSubsystem.h"

// UE
#include "Engine/World.h"
#include "GameFeatureData.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif // WITH_EDITOR

#include UE_INLINE_GENERATED_CPP_BY_NAME(GfpmAction_RegisterGameFeaturePluginActivation)

// Called by Game Features system when owning plugin is registered
void UGfpmAction_RegisterGameFeaturePluginActivation::OnGameFeatureRegistering()
{
	Super::OnGameFeatureRegistering();

	// Registration fires at engine init before any world exists
	if (!OnPostWorldInitHandle.IsValid())
	{
		OnPostWorldInitHandle = FWorldDelegates::OnPostWorldInitialization.AddWeakLambda(this, [this](UWorld* World, const UWorld::InitializationValues /*IVS*/)
		{
			OnPostWorldInitialized(World);
		});
	}

	// Feed any world already live at registration (editor world, or single game world on dynamic registration)
	if (UGfpmLoaderSubsystem* Loader = UGfpmLoaderSubsystem::GetLoaderSubsystem())
	{
		Loader->RegisterGameFeaturePluginActivation(GetTypedOuter<UGameFeatureData>(), ActivationTags);
	}
}

// Called by Game Features system when owning plugin is unregistered
void UGfpmAction_RegisterGameFeaturePluginActivation::OnGameFeatureUnregistering()
{
	Super::OnGameFeatureUnregistering();

	if (OnPostWorldInitHandle.IsValid())
	{
		FWorldDelegates::OnPostWorldInitialization.Remove(OnPostWorldInitHandle);
		OnPostWorldInitHandle.Reset();
	}

	if (UGfpmLoaderSubsystem* Loader = UGfpmLoaderSubsystem::GetLoaderSubsystem())
	{
		Loader->UnregisterGameFeaturePluginActivation(GetTypedOuter<UGameFeatureData>());
	}
}

// When any world finished initializing
void UGfpmAction_RegisterGameFeaturePluginActivation::OnPostWorldInitialized(UWorld* World)
{
	UGfpmLoaderSubsystem* Loader = World ? World->GetSubsystem<UGfpmLoaderSubsystem>() : nullptr;
	if (!Loader)
	{
		// World does not host the loader (editor, preview, inactive type), nothing to seed
		return;
	}

	Loader->RegisterGameFeaturePluginActivation(GetTypedOuter<UGameFeatureData>(), ActivationTags);
}

#if WITH_EDITOR
// Called by editor's Data Validation system when validation runs on this object
EDataValidationResult UGfpmAction_RegisterGameFeaturePluginActivation::IsDataValid(FDataValidationContext& Context) const
{
	const EDataValidationResult Result = Super::IsDataValid(Context);

	if (ActivationTags.IsEmpty())
	{
		static const FString Tmpl = TEXT("ActivationTags is empty, action will never activate plugin");
		const FString Formatted = FString::Format(*Tmpl, FStringFormatOrderedArguments{});
		Context.AddWarning(FText::FromString(Formatted));
	}

	return Result;
}

// Called by editor when any property on this object is changed in details panel
void UGfpmAction_RegisterGameFeaturePluginActivation::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Register replaces any stale binding and internally schedules apply against any tracked ASC, so property panel commits before deferred transition
	if (UGfpmLoaderSubsystem* Loader = UGfpmLoaderSubsystem::GetLoaderSubsystem())
	{
		Loader->RegisterGameFeaturePluginActivation(GetTypedOuter<UGameFeatureData>(), ActivationTags);
	}
}
#endif // WITH_EDITOR
