// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Engine/DataTable.h"

// DAL
#include "DalRegistryRow.h"

// UE
#include "GameplayTagContainer.h"

#include "BmrWidgetRow.generated.h"

/**
 * Row struct for UI widget data registered via Data Registry.
 * Mods or maps register their own data tables with widget rows for auto-creation.
 */
USTRUCT(BlueprintType)
struct BOMBER_API FBmrWidgetRow : public FTableRowBase
#if CPP
    , public TDalRegistryRow<FBmrWidgetRow>
#endif
{
	GENERATED_BODY()

	/** The tag associated with this widget, is used for obtaining its widget data or widget instance. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Categories = "UI"))
	FGameplayTag WidgetTag = FGameplayTag::EmptyTag;

	/** The class of the widget to create. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftClassPtr<class UUserWidget> WidgetClass = nullptr;

	/** If true, adds the widget to the viewport, so it will be registered in slate and ready to use. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAddToViewport = true;

	/** The Z-order of the widget, higher order will be drawn on top of lower order. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ZOrder = 0;

	/** Returns widget row data by tag from Data Registry, or nullptr */
	static const FBmrWidgetRow* GetRowByTag(FGameplayTag WidgetTag);
};
