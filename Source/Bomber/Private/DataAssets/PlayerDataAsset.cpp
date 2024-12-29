﻿// Copyright (c) Yevhenii Selivanov

#include "DataAssets/PlayerDataAsset.h"
//---
#include "DataAssets/DataAssetsContainer.h"
//---
#include "GameFramework/Actor.h"
#include "Engine/Texture2DArray.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInstanceDynamic.h"
//---
#if WITH_EDITOR
#include "MyEditorUtilsLibraries/EditorUtilsLibrary.h"
#endif
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(PlayerDataAsset)

// Returns the num of skin textures in the array of diffuse maps specified a player material instance
int32 UPlayerRow::GetSkinTexturesNum() const
{
	const UPlayerDataAsset* PlayerDataAsset = Cast<UPlayerDataAsset>(GetOuter());
	if (!PlayerDataAsset
	    || !MaterialInstanceInternal)
	{
		return INDEX_NONE;
	}

	const FName SkinArrayParameterName = PlayerDataAsset->GetSkinArrayParameter();
	if (!ensureMsgf(!SkinArrayParameterName.IsNone(), TEXT("ASSERT: [%i] %hs:\nNo skin array parameter name specified in the player data asset"), __LINE__, __FUNCTION__))
	{
		return INDEX_NONE;
	}

	UTexture* FoundTexture = nullptr;
	MaterialInstanceInternal->GetTextureParameterValue(SkinArrayParameterName, /*out*/FoundTexture);
	const UTexture2DArray* Texture2DArray = Cast<UTexture2DArray>(FoundTexture);
	return Texture2DArray ? Texture2DArray->GetArraySize() : INDEX_NONE;
}

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
	if (!FEditorUtilsLibrary::IsEditorNotPieWorld())
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
		UpdateSkinTextures();
	}
}
#endif	//WITH_EDITOR

// Create dynamic material instance for each ski if is not done before.
void UPlayerRow::UpdateSkinTextures()
{
	UPlayerDataAsset* PlayerDataAsset = Cast<UPlayerDataAsset>(GetOuter());
	if (!PlayerDataAsset
	    || !MaterialInstanceInternal)
	{
		return;
	}

	const FName SkinIndexParameterName = PlayerDataAsset->GetSkinIndexParameter();
	const int32 SkinTexturesNum = GetSkinTexturesNum();
	if (!ensureMsgf(SkinTexturesNum != INDEX_NONE, TEXT("ASSERT: [%i] %hs:\nNo skin textures found in the texture 2D array of player material instance"), __LINE__, __FUNCTION__)
	    || !ensureMsgf(!SkinIndexParameterName.IsNone(), TEXT("ASSERT: [%i] %hs:\nNo skin index parameter name specified in the player data asset"), __LINE__, __FUNCTION__))
	{
		return;
	}

	const int32 MaterialInstancesDynamicNum = MaterialInstancesDynamicInternal.Num();
	if (SkinTexturesNum == MaterialInstancesDynamicNum)
	{
		// The same amount, so all dynamic materials are already created
		return;
	}

	// Create dynamic materials
	const int32 InstancesToCreateNum = SkinTexturesNum - MaterialInstancesDynamicNum;
	for (int32 Index = 0; Index < InstancesToCreateNum; ++Index)
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

// Default constructor
UPlayerDataAsset::UPlayerDataAsset()
{
	ActorTypeInternal = EAT::Player;
	RowClassInternal = UPlayerRow::StaticClass();
}

// Returns the player data asset
const UPlayerDataAsset& UPlayerDataAsset::Get()
{
	return UDataAssetsContainer::GetLevelActorDataAssetChecked<ThisClass>();
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

// Return first found row by specified player tag
const UPlayerRow* UPlayerDataAsset::GetRowByPlayerTag(const FPlayerTag& PlayerTag) const
{
	for (const ULevelActorRow* RowIt : RowsInternal)
	{
		const UPlayerRow* PlayerRow = Cast<UPlayerRow>(RowIt);
		if (PlayerRow && PlayerRow->PlayerTag == PlayerTag)
		{
			return PlayerRow;
		}
	}
	return nullptr;
}