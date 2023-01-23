// Copyright (c) Yevhenii Selivanov

#include "MyUnrealEdEngine.h"
//---
#include "UnrealEdGlobals.h"

// Will notify on any data asset changes
UMyUnrealEdEngine::FOnAnyDataAssetChanged UMyUnrealEdEngine::GOnAnyDataAssetChanged;

// Binds to update movements of each AI controller.
UMyUnrealEdEngine::FUpdateAI UMyUnrealEdEngine::GOnAIUpdatedDelegate;

// Returns this Unreal Editor Engine object
const UMyUnrealEdEngine& UMyUnrealEdEngine::Get()
{
	const auto MyUnrealEdEngine = Cast<UMyUnrealEdEngine>(GUnrealEd);
	checkf(MyUnrealEdEngine, TEXT("The My Unread Editor Engine is not valid"));
	return *MyUnrealEdEngine;
}

// Returns singleton of the editor client by specified player index
UObject* UMyUnrealEdEngine::GetClientSingleton(int32 Index) const
{
	if (ClientSingletonsInternal.IsValidIndex(Index))
	{
		return ClientSingletonsInternal[Index];
	}

	return nullptr;
}

// Creates a new Play in Editor instance (which may be in a new process if not running under one process
void UMyUnrealEdEngine::CreateNewPlayInEditorInstance(FRequestPlaySessionParams& InRequestParams, const bool bInDedicatedInstance, const EPlayNetMode InNetMode)
{
	AddClientSingleton();

	Super::CreateNewPlayInEditorInstance(InRequestParams, bInDedicatedInstance, InNetMode);
}

// Create new singleton for the editor client if is needed
void UMyUnrealEdEngine::AddClientSingleton()
{
	if (!PlayInEditorSessionInfo.IsSet())
	{
		return;
	}

	auto GetCurrentClientIndex = [AllInstances = PlayInEditorSessionInfo->NumClientInstancesCreated]() -> int32
	{
		// All instances contain the server and all clients
		constexpr int32 AllInstancesToClientNum = 1;
		return AllInstances - AllInstancesToClientNum;
	};

	const int32 ClientIndex = GetCurrentClientIndex();
	const bool bIsClientPIE = ClientIndex >= 0;
	if (!bIsClientPIE)
	{
		return;
	}

	const bool bIsValidClassName = GameSingletonClassName.ToString().Len() > 0;
	if (!ensureMsgf(bIsValidClassName, TEXT("ASSERT: 'bIsValidClassName' is false")))
	{
		return;
	}

	const UClass* SingletonClass = LoadClass<UObject>(nullptr, *GameSingletonClassName.ToString());
	if (!ensureMsgf(SingletonClass, TEXT("ASSERT: 'SingletonClass' is not valid")))
	{
		return;
	}

	const bool bIsValidIndexInstanceIndex = ClientSingletonsInternal.IsValidIndex(ClientIndex);
	UObject* ClientSingleton = bIsValidIndexInstanceIndex ? ClientSingletonsInternal[ClientIndex] : nullptr;
	if (ClientSingleton)
	{
		// Is already created
		return;
	}

	ClientSingleton = NewObject<UObject>(this, SingletonClass, *FString::Printf(TEXT("ClientSingleton_%i"), ClientIndex));
	if (!ensureMsgf(ClientSingleton, TEXT("ASSERT: 'NewSingleton' was not created")))
	{
		return;
	}

	// Add to array
	if (bIsValidIndexInstanceIndex)
	{
		ClientSingletonsInternal[ClientIndex] = ClientSingleton;
	}
	else
	{
		ClientSingletonsInternal.Emplace(ClientSingleton);
	}
}
