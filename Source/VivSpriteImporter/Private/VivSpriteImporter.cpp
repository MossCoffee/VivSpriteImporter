// Copyright Epic Games, Inc. All Rights Reserved.

#include "VivSpriteImporter.h"
#include "VivSpriteImporterStyle.h"
#include "VivSpriteImporterCommands.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"
#include "VivSpriteParser.h"
#include "IDesktopPlatform.h"
#include "DesktopPlatformModule.h"

static const FName VivSpriteImporterTabName("VivSpriteImporter");

#define LOCTEXT_NAMESPACE "FVivSpriteImporterModule"

void FVivSpriteImporterModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FVivSpriteImporterStyle::Initialize();
	FVivSpriteImporterStyle::ReloadTextures();

	FVivSpriteImporterCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FVivSpriteImporterCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FVivSpriteImporterModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FVivSpriteImporterModule::RegisterMenus));
}

void FVivSpriteImporterModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FVivSpriteImporterStyle::Shutdown();

	FVivSpriteImporterCommands::Unregister();
}


void FVivSpriteImporterModule::PluginButtonClicked()
{
	FString LastFilePath = FString("C:\\");
	FString FilePath;
	if(OpenFile(FString("Select Asset"), FString(".vivsprite"), LastFilePath, FilePath)){
		VivSpriteParser parser(FilePath);
	}

}


void FVivSpriteImporterModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FVivSpriteImporterCommands::Get().PluginAction, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FVivSpriteImporterCommands::Get().PluginAction));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

bool FVivSpriteImporterModule::OpenFile(const FString& Title, const FString& FileTypes, FString& InOutLastPath, FString& OutOpenFilenames)
{
	TSharedPtr<SWidget> empty;
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	const void* ParentWindow = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(empty);
	bool bOpened = false;
	TArray<FString> output;

	bOpened = DesktopPlatform->OpenFileDialog(
		ParentWindow,
		Title,
		InOutLastPath,
		TEXT(""),
		FileTypes,
		EFileDialogFlags::Type::None,
		output
	);

	if (output.Num() > 0) {
		OutOpenFilenames = output[0];
	}

	if (bOpened)
	{
		// User successfully chose a file; remember the path for the next time the dialog opens.
		InOutLastPath = OutOpenFilenames;
	}

	return bOpened;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FVivSpriteImporterModule, VivSpriteImporter)