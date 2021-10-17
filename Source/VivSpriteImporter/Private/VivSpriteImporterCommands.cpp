// Copyright Epic Games, Inc. All Rights Reserved.

#include "VivSpriteImporterCommands.h"

#define LOCTEXT_NAMESPACE "FVivSpriteImporterModule"

void FVivSpriteImporterCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "VivSpriteImporter", "Bring up VivSpriteImporter window", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
