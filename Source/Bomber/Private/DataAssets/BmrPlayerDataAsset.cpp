// Copyright (c) Yevhenii Selivanov

#include "DataAssets/BmrPlayerDataAsset.h"

// Bomber
#include "DalSubsystem.h"

#if WITH_EDITOR
#include "MyEditorUtilsLibraries/EditorUtilsLibrary.h"
#endif

// UE
#include "Engine/SkeletalMesh.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Engine/Texture2DArray.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInstanceDynamic.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrPlayerDataAsset)

// Returns the num of skin textures in the array of diffuse maps specified a player material instance
int32 UBmrPlayerRow::GetSkinTexturesNum() const
{
	const UBmrPlayerDataAsset* PlayerDataAsset = Cast<UBmrPlayerDataAsset>(GetOuter());
	if (!PlayerDataAsset
	    || !MaterialInstance)
	{
		return INDEX_NONE;
	}

	const FName SkinArrayParameterName = PlayerDataAsset->GetSkinArrayParameter();
	if (!ensureMsgf(!SkinArrayParameterName.IsNone(), TEXT("ASSERT: [%i] %hs:\nNo skin array parameter name specified in the player data asset"), __LINE__, __FUNCTION__))
	{
		return INDEX_NONE;
	}

	UTexture* FoundTexture = nullptr;
	MaterialInstance->GetTextureParameterValue(SkinArrayParameterName, /*out*/ FoundTexture);
	const UTexture2DArray* Texture2DArray = Cast<UTexture2DArray>(FoundTexture);
	return Texture2DArray ? Texture2DArray->GetArraySize() : INDEX_NONE;
}

// Returns the dynamic material instance of a player with specified skin.
UMaterialInstanceDynamic* UBmrPlayerRow::GetMaterialInstanceDynamic(int32 SkinIndex) const
{
	if (MaterialInstancesDynamic.IsValidIndex(SkinIndex))
	{
		return MaterialInstancesDynamic[SkinIndex];
	}

	return nullptr;
}

#if WITH_EDITOR
// Handle adding and changing material instance to prepare dynamic materials
void UBmrPlayerRow::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Continue only if [IsEditorNotPieWorld]
	if (!FEditorUtilsLibrary::IsEditorNotPieWorld())
	{
		return;
	}

	// If material instance was changed
	static const FName PropertyName = GET_MEMBER_NAME_CHECKED(ThisClass, MaterialInstance);
	const FProperty* Property = PropertyChangedEvent.Property;
	if (Property
	    && Property->IsA<FObjectProperty>()
	    && PropertyChangedEvent.ChangeType == EPropertyChangeType::ValueSet
	    && Property->GetFName() == PropertyName)
	{
		// Force recreation of dynamic material instances
		MaterialInstancesDynamic.Empty();
		UpdateSkinTextures();
	}
}

// Retrieves bones and socket names from skeletal mesh of this row to be displayed as dropdown in editor
TArray<FString> UBmrPlayerRow::GetRowMeshSockets() const
{
	TArray<FString> SocketNames;

	const USkeletalMesh* SkeletalMesh = Cast<USkeletalMesh>(Mesh);
	if (!SkeletalMesh)
	{
		return SocketNames;
	}

	const TArray<USkeletalMeshSocket*>& Sockets = SkeletalMesh->GetActiveSocketList();
	for (const USkeletalMeshSocket* SocketIt : Sockets)
	{
		SocketNames.AddUnique(SocketIt->SocketName.ToString());
	}

	const FReferenceSkeleton& RefSkeleton = SkeletalMesh->GetRefSkeleton();
	for (int32 BoneIndex = 0; BoneIndex < RefSkeleton.GetNum(); ++BoneIndex)
	{
		SocketNames.AddUnique(RefSkeleton.GetBoneName(BoneIndex).ToString());
	}

	return SocketNames;
}
#endif // WITH_EDITOR

// Create dynamic material instance for each ski if is not done before.
void UBmrPlayerRow::UpdateSkinTextures()
{
	UBmrPlayerDataAsset* PlayerDataAsset = Cast<UBmrPlayerDataAsset>(GetOuter());
	if (!PlayerDataAsset
	    || !MaterialInstance)
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

	const int32 MaterialInstancesDynamicNum = MaterialInstancesDynamic.Num();
	if (SkinTexturesNum == MaterialInstancesDynamicNum)
	{
		// The same amount, so all dynamic materials are already created
		return;
	}

	// Create dynamic materials
	const int32 InstancesToCreateNum = SkinTexturesNum - MaterialInstancesDynamicNum;
	for (int32 Index = 0; Index < InstancesToCreateNum; ++Index)
	{
		UMaterialInstanceDynamic* MaterialInstanceDynamic = UMaterialInstanceDynamic::Create(MaterialInstance, PlayerDataAsset);
		if (!ensureMsgf(MaterialInstanceDynamic, TEXT("ASSERT: Could not create 'MaterialInstanceDynamic'")))
		{
			continue;
		}

		MaterialInstanceDynamic->SetFlags(RF_Public | RF_Transactional);
		const int32 SkinPosition = MaterialInstancesDynamic.Emplace(MaterialInstanceDynamic);
		MaterialInstanceDynamic->SetScalarParameterValue(SkinIndexParameterName, SkinPosition);
	}
}

// Default constructor
UBmrPlayerDataAsset::UBmrPlayerDataAsset()
{
	ActorType = EAT::Player;
	RowClass = UBmrPlayerRow::StaticClass();
}

// Returns the player data asset
const UBmrPlayerDataAsset& UBmrPlayerDataAsset::Get()
{
	return UDalSubsystem::GetDataAssetChecked<ThisClass>();
}

// Get nameplate material by index, is used by nameplate meshes
UMaterialInterface* UBmrPlayerDataAsset::GetNameplateMaterial(int32 Index) const
{
	if (NameplateMaterials.IsValidIndex(Index))
	{
		return NameplateMaterials[Index];
	}

	return nullptr;
}

// Return first found row by specified player tag
const UBmrPlayerRow* UBmrPlayerDataAsset::GetRowByPlayerTag(const FBmrPlayerTag& PlayerTag) const
{
	for (const UBmrLevelActorRow* RowIt : Rows)
	{
		const UBmrPlayerRow* PlayerRow = Cast<UBmrPlayerRow>(RowIt);
		if (PlayerRow && PlayerRow->PlayerTag == PlayerTag)
		{
			return PlayerRow;
		}
	}
	return nullptr;
}