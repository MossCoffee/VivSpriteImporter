// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "VivSpriteImporterStyle.h"

class FVivSpriteImporterCommands : public TCommands<FVivSpriteImporterCommands>
{
public:

	FVivSpriteImporterCommands()
		: TCommands<FVivSpriteImporterCommands>(TEXT("VivSpritePlugin"), NSLOCTEXT("Contexts", "VivSpriteImporter", "VivSpriteImporter Plugin"), NAME_None, FVivSpriteImporterStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};