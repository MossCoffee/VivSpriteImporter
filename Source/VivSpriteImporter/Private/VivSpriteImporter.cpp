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
	std::string filePath = "C:\\Users\\thefa\\Downloads\\Mobility";
	VivSpriteParser parser(FString(filePath.c_str()));

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

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FVivSpriteImporterModule, VivSpriteImporter)