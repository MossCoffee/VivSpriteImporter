// Copyright Epic Games, Inc. All Rights Reserved.

#include "VivSpriteImporterCommands.h"

#define LOCTEXT_NAMESPACE "FVivSpriteImporterModule"

void FVivSpriteImporterCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "Import VivSprite", "Import a VivSprite!", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
