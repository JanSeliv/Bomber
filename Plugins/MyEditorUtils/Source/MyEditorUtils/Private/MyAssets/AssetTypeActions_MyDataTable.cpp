// Copyright (c) Yevhenii Selivanov.

#include "MyAssets/AssetTypeActions_MyDataTable.h"
//---
#include "DataTableEditorModule.h"
#include "DesktopPlatformModule.h"
#include "ToolMenus.h"
#include "EditorFramework/AssetImportData.h"
#include "Engine/DataTable.h"
#include "Framework/Application/SlateApplication.h"
#include "Misc/FileHelper.h"
#include "Misc/MessageDialog.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(AssetTypeActions_MyDataTable)

#define LOCTEXT_NAMESPACE "AssetTypeActions"

UMyDataTableFactory::UMyDataTableFactory()
{
	bCreateNew = true;
	bEditAfterNew = true;
	bEditorImport = true;

	// Has to be set in child factory
	SupportedClass = UDataTable::StaticClass();
}

FText UMyDataTableFactory::GetDisplayName() const
{
	return LOCTEXT("MyDataTableFactory", "My Data Table");
}

UObject* UMyDataTableFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	check(DoesSupportClass(InClass));
	return NewObject<UObject>(InParent, InClass, InName, Flags);
}

bool UMyDataTableFactory::DoesSupportClass(UClass* Class)
{
	return Class && Class->IsChildOf(GetSupportedClass());
}

FText FAssetTypeActions_MyDataTable::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_MyDataTable", "My Data Table");
}

UClass* FAssetTypeActions_MyDataTable::GetSupportedClass() const
{
	return UDataTable::StaticClass();
}

// Shows 'Export as JSON' option in the context menu
void FAssetTypeActions_MyDataTable::GetActions(const TArray<UObject*>& InObjects, FToolMenuSection& Section)
{
	const TArray<TWeakObjectPtr<UObject>> Tables = GetTypedWeakObjectPtrs<UObject>(InObjects);

	Section.AddMenuEntry(
		"DataTable_ExportAsJSON",
		LOCTEXT("DataTable_ExportAsJSON", "Export as JSON"),
		LOCTEXT("DataTable_ExportAsJSONTooltip", "Export the data table as a file containing JSON data."),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateSP(this, &FAssetTypeActions_MyDataTable::ExecuteExportAsJSON, Tables),
			FCanExecuteAction()
		)
	);
}

// Is overridden to show 'reimport' options in the contexts menu
void FAssetTypeActions_MyDataTable::GetResolvedSourceFilePaths(const TArray<UObject*>& TypeAssets, TArray<FString>& OutSourceFilePaths) const
{
	for (UObject* Asset : TypeAssets)
	{
		const UDataTable* DataTable = CastChecked<UDataTable>(Asset);
		if (const UAssetImportData* AssetImportData = DataTable->AssetImportData)
		{
			AssetImportData->ExtractFilenames(OutSourceFilePaths);
		}
	}
}

// Opens data within the data table editor
void FAssetTypeActions_MyDataTable::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	TArray<UDataTable*> DataTablesToOpen;
	TArray<UDataTable*> InvalidDataTables;

	for (UObject* Obj : InObjects)
	{
		UDataTable* Table = Cast<UDataTable>(Obj);
		if (Table)
		{
			if (Table->GetRowStruct())
			{
				DataTablesToOpen.Add(Table);
			}
			else
			{
				InvalidDataTables.Add(Table);
			}
		}
	}

	if (InvalidDataTables.Num() > 0)
	{
		FTextBuilder DataTablesListText;
		DataTablesListText.Indent();
		for (const UDataTable* Table : InvalidDataTables)
		{
			const FTopLevelAssetPath ResolvedRowStructName = Table->GetRowStructPathName();
			DataTablesListText.AppendLineFormat(LOCTEXT("DataTable_MissingRowStructListEntry", "* {0} (Row Structure: {1})"), FText::FromString(Table->GetName()), FText::FromString(ResolvedRowStructName.ToString()));
		}

		const EAppReturnType::Type DlgResult = FMessageDialog::Open(
			EAppMsgType::YesNoCancel,
			FText::Format(LOCTEXT("DataTable_MissingRowStructMsg", "The following Data Tables are missing their row structure and will not be editable.\n\n{0}\n\nDo you want to open these data tables?"), DataTablesListText.ToText()),
			LOCTEXT("DataTable_MissingRowStructTitle", "Continue?")
		);

		switch (DlgResult)
		{
		case EAppReturnType::Yes:
			DataTablesToOpen.Append(InvalidDataTables);
			break;
		case EAppReturnType::Cancel:
			return;
		default:
			break;
		}
	}

	FDataTableEditorModule& DataTableEditorModule = FModuleManager::LoadModuleChecked<FDataTableEditorModule>("DataTableEditor");
	for (UDataTable* Table : DataTablesToOpen)
	{
		DataTableEditorModule.CreateDataTableEditor(EToolkitMode::Standalone, EditWithinLevelEditor, Table);
	}
}

// Low-level .json exporter, is called when 'Export as JSON' actions was pressed
void FAssetTypeActions_MyDataTable::ExecuteExportAsJSON(TArray<TWeakObjectPtr<UObject>> Objects)
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();

	const void* ParentWindowWindowHandle = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);

	for (auto ObjIt = Objects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		auto DataTable = Cast<UDataTable>((*ObjIt).Get());
		if (DataTable)
		{
			const FText Title = FText::Format(LOCTEXT("DataTable_ExportJSONDialogTitle", "Export '{0}' as JSON..."), FText::FromString(*DataTable->GetName()));
			const FString CurrentFilename = DataTable->AssetImportData->GetFirstFilename();
			const FString FileTypes = TEXT("Data Table JSON (*.json)|*.json");

			TArray<FString> OutFilenames;
			DesktopPlatform->SaveFileDialog(
				ParentWindowWindowHandle,
				Title.ToString(),
				(CurrentFilename.IsEmpty()) ? TEXT("") : FPaths::GetPath(CurrentFilename),
				(CurrentFilename.IsEmpty()) ? TEXT("") : FPaths::GetBaseFilename(CurrentFilename) + TEXT(".json"),
				FileTypes,
				EFileDialogFlags::None,
				OutFilenames
			);

			if (OutFilenames.Num() > 0)
			{
				FFileHelper::SaveStringToFile(DataTable->GetTableAsJSON(EDataTableExportFlags::UseJsonObjectsForStructs), *OutFilenames[0]);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
