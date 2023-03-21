#pragma once

#include "CoreMinimal.h"
#include "CoreTypes.h"

#include "Toolkits/IToolkitHost.h"
#include "AssetTypeActions_Base.h"

class UPaperSprite;

class FVivSpriteFlipbookHelpers
{
public:
	static bool CreateFlipbook(TArray<TWeakObjectPtr<UPaperSprite>> Objects, const FString& FlipbookName);
};
