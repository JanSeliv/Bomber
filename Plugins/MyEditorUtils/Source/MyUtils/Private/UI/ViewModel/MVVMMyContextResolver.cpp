// Copyright (c) Yevhenii Selivanov

#include "UI/ViewModel/MVVMMyContextResolver.h"
//---
#include "UI/ViewModel/MVVM_MyBaseViewModel.h"
//---
#include "Blueprint/UserWidget.h"
#include "Engine/GameInstance.h"
#include "MVVMGameSubsystem.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(MVVMMyContextResolver)

// Is called to create a new instance of the ViewModel
UObject* UMVVMMyContextResolver::CreateInstance(const UClass* ExpectedType, const UUserWidget* UserWidget, const UMVVMView* View) const
{
	if (!ensureMsgf(ExpectedType, TEXT("ASSERT: [%i] %s:\n'ExpectedType' is nullptr"), __LINE__, *FString(__FUNCTION__))
		|| !ensureMsgf(ExpectedType->IsChildOf<UMVVM_MyBaseViewModel>(), TEXT("ASSERT: [%i] %s:\n'ExpectedType' is not a subclass of 'UMVVM_MyBaseViewModel'"), __LINE__, *FString(__FUNCTION__))
		|| !ensureMsgf(UserWidget, TEXT("ASSERT: [%i] %s:\n'UserWidget' is nullptr"), __LINE__, *FString(__FUNCTION__))
		|| !ensureMsgf(View, TEXT("ASSERT: [%i] %s:\n'View' is nullptr"), __LINE__, *FString(__FUNCTION__)))
	{
		return nullptr;
	}

	if (UObject* SuperInstance = Super::CreateInstance(ExpectedType, UserWidget, View))
	{
		// Is created by Super
		return SuperInstance;
	}

	const UGameInstance* GameInstance = UserWidget->GetGameInstance();
	if (!ensureMsgf(GameInstance, TEXT("ASSERT: [%i] %s:\n'GameInstance' is not valid!"), __LINE__, *FString(__FUNCTION__)))
	{
		return nullptr;
	}

	const UMVVMGameSubsystem* MVVMSubsystem = GameInstance->GetSubsystem<UMVVMGameSubsystem>();
	checkf(MVVMSubsystem, TEXT("ERROR: [%i] %s:\n'MVVMSubsystem' is null!"), __LINE__, *FString(__FUNCTION__));

	UMVVMViewModelCollectionObject* Collection = MVVMSubsystem->GetViewModelCollection();
	checkf(Collection, TEXT("ERROR: [%i] %s:\n'Collection' is null!"), __LINE__, *FString(__FUNCTION__));

	// Construct a new View Model
	UMVVM_MyBaseViewModel* NewViewModel = NewObject<UMVVM_MyBaseViewModel>(UserWidget->GetOwningPlayer(), ExpectedType);
	NewViewModel->OnViewModelConstruct(UserWidget);

	FMVVMViewModelContext Context;
	Context.ContextClass = NewViewModel->GetClass();
	Context.ContextName = NewViewModel->GetFName();

	Collection->AddViewModelInstance(MoveTemp(Context), NewViewModel);

	return NewViewModel;
}

// Is called to destroy the instance of the ViewModel
void UMVVMMyContextResolver::DestroyInstance(const UObject* ViewModel, const UMVVMView* View) const
{
	UMVVM_MyBaseViewModel* MyBaseViewModel = const_cast<UMVVM_MyBaseViewModel*>(Cast<UMVVM_MyBaseViewModel>(ViewModel));
	if (!ensureMsgf(MyBaseViewModel, TEXT("ASSERT: [%i] %s:\n'MyBaseViewModel' is nullptr"), __LINE__, *FString(__FUNCTION__))
		|| !ensureMsgf(View, TEXT("ASSERT: [%i] %s:\n'View' is nullptr"), __LINE__, *FString(__FUNCTION__)))
	{
		return;
	}

	Super::DestroyInstance(ViewModel, View);
	if (!IsValid(MyBaseViewModel))
	{
		// Is destroyed by Super
		return;
	}

	// Get Game Instance from View
	const UGameInstance* GameInstance = MyBaseViewModel->GetWorld()->GetGameInstance();
	if (!ensureMsgf(GameInstance, TEXT("ASSERT: [%i] %s:\n'GameInstance' is not valid!"), __LINE__, *FString(__FUNCTION__)))
	{
		return;
	}

	const UMVVMGameSubsystem* MVVMSubsystem = GameInstance->GetSubsystem<UMVVMGameSubsystem>();
	checkf(MVVMSubsystem, TEXT("ERROR: [%i] %s:\n'MVVMSubsystem' is null!"), __LINE__, *FString(__FUNCTION__));

	UMVVMViewModelCollectionObject* Collection = MVVMSubsystem->GetViewModelCollection();
	checkf(Collection, TEXT("ERROR: [%i] %s:\n'Collection' is null!"), __LINE__, *FString(__FUNCTION__));
	Collection->RemoveAllViewModelInstance(MyBaseViewModel);

	// Destroy the View Model
	MyBaseViewModel->OnViewModelDestruct();
	MyBaseViewModel->ConditionalBeginDestroy();
}
