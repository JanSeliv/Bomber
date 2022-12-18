﻿// Copyright (c) Yevhenii Selivanov

#include "Globals/PlayerDataAsset.h"
//---
#include "Globals/DataAssetsContainer.h"
//---
#include "Engine/Texture2DArray.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInstanceDynamic.h"
//---
#if WITH_EDITOR
#include "EditorUtilsLibrary.h"
#endif

// Returns the dynamic material instance of a player with specified skin.
UMaterialInstanceDynamic* UPlayerRow::GetMaterialInstanceDynamic(int32 SkinIndex) const
{
	if (MaterialInstancesDynamicInternal.IsValidIndex(SkinIndex))
	{
		return MaterialInstancesDynamicInternal[SkinIndex];
	}

	return nullptr;
}

#if WITH_EDITOR
// Handle adding and changing material instance to prepare dynamic materials
void UPlayerRow::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Continue only if [IsEditorNotPieWorld]
	if (!UEditorUtilsLibrary::IsEditorNotPieWorld())
	{
		return;
	}

	// If material instance was changed
	static const FName PropertyName = GET_MEMBER_NAME_CHECKED(ThisClass, MaterialInstanceInternal);
	const FProperty* Property = PropertyChangedEvent.Property;
	if (Property
	    && Property->IsA<FObjectProperty>()
	    && PropertyChangedEvent.ChangeType == EPropertyChangeType::ValueSet
	    && Property->GetFName() == PropertyName)
	{
		// Force recreation of dynamic material instances
		MaterialInstancesDynamicInternal.Empty();
		TryCreateDynamicMaterials();
	}
}

// Create dynamic material instance for each ski if is not done before.
void UPlayerRow::TryCreateDynamicMaterials()
{
	const auto PlayerDataAsset = Cast<UPlayerDataAsset>(GetOuter());
	if (!PlayerDataAsset
	    || !MaterialInstanceInternal)
	{
		return;
	}

	static const FName SkinArrayParameterName = PlayerDataAsset->GetSkinArrayParameter();
	static const FName SkinIndexParameterName = PlayerDataAsset->GetSkinIndexParameter();
	static const bool bSkinParamNamesAreValid = !(SkinArrayParameterName.IsNone() && SkinIndexParameterName.IsNone());
	if (!ensureMsgf(bSkinParamNamesAreValid, TEXT("ASSERT: TryCreateDynamicMaterials: 'bSkinParamNamesAreValid' is false")))
	{
		return;
	}

	// Find the number of skins in the texture 2D array of player material instance.
	UTexture* FoundTexture = nullptr;
	MaterialInstanceInternal->GetTextureParameterValue(SkinArrayParameterName, FoundTexture);
	const auto Texture2DArray = Cast<UTexture2DArray>(FoundTexture);
	const int32 SkinTexturesNum = Texture2DArray ? Texture2DArray->GetArraySize() : 0;
	const int32 MaterialInstancesDynamicNum = MaterialInstancesDynamicInternal.Num();
	if (SkinTexturesNum == MaterialInstancesDynamicNum)
	{
		// The same amount, so all dynamic materials are already created
		return;
	}

	// Create dynamic materials
	const int32 InstancesToCreateNum = SkinTexturesNum - MaterialInstancesDynamicNum;
	for (int32 i = 0; i < InstancesToCreateNum; ++i)
	{
		UMaterialInstanceDynamic* MaterialInstanceDynamic = UMaterialInstanceDynamic::Create(MaterialInstanceInternal, PlayerDataAsset);
		if (!ensureMsgf(MaterialInstanceDynamic, TEXT("ASSERT: Could not create 'MaterialInstanceDynamic'")))
		{
			continue;
		}

		MaterialInstanceDynamic->SetFlags(RF_Public | RF_Transactional);
		const int32 SkinPosition = MaterialInstancesDynamicInternal.Emplace(MaterialInstanceDynamic);
		MaterialInstanceDynamic->SetScalarParameterValue(SkinIndexParameterName, SkinPosition);
	}
}
#endif	//WITH_EDITOR

// Default constructor
UPlayerDataAsset::UPlayerDataAsset()
{
	ActorTypeInternal = EAT::Player;
	RowClassInternal = UPlayerRow::StaticClass();
}

// Returns the player data asset
const UPlayerDataAsset& UPlayerDataAsset::Get()
{
	const ULevelActorDataAsset* FoundDataAsset = UDataAssetsContainer::GetDataAssetByActorType(EActorType::Player);
	const auto PlayerDataAsset = Cast<UPlayerDataAsset>(FoundDataAsset);
	checkf(PlayerDataAsset, TEXT("The Player Data Asset is not valid"));
	return *PlayerDataAsset;
}

// Get nameplate material by index, is used by nameplate meshes
UMaterialInterface* UPlayerDataAsset::GetNameplateMaterial(int32 Index) const
{
	if (NameplateMaterialsInternal.IsValidIndex(Index))
	{
		return NameplateMaterialsInternal[Index];
	}

	return nullptr;
}