// Copyright MossCoffee All Rights Reserved.

#include "VivSpriteFlipbookHelpers.h"

//#include "SpriteAssetTypeActions.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "ToolMenus.h"
#include "Misc/PackageName.h"
#include "Misc/FeedbackContext.h"
#include "Styling/AppStyle.h"

//#include "SpriteEditor/SpriteEditor.h"
#include "IContentBrowserSingleton.h"
#include "ContentBrowserModule.h"
#include "PaperFlipbookHelpers.h"
#include "PaperFlipbook.h"
#include "PaperFlipbookFactory.h"
#include "PaperSprite.h"
//#include "PaperSpriteSheet.h"

//Pulled directly from SpriteAssetTypeActions in Paper2D. May need some trimming.
bool FVivSpriteFlipbookHelpers::CreateFlipbook(TArray<TWeakObjectPtr<UPaperSprite>> Objects, const FString& FlipbookName)
{
	FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	TArray<UPaperSprite*> AllSprites;

	for (auto ObjIt = Objects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		UPaperSprite* Object = (*ObjIt).Get();
		if (Object && Object->IsValidLowLevel())
		{
			AllSprites.Add(Object);
		}
	}

	TMap<FString, TArray<UPaperSprite*> > SpriteFlipbookMap;
	FPaperFlipbookHelpers::ExtractFlipbooksFromSprites(/*out*/SpriteFlipbookMap, AllSprites, TArray<FString>());
	TArray<UObject*> ObjectsToSync;

	if (SpriteFlipbookMap.Num() > 0)
	{
		GWarn->BeginSlowTask(NSLOCTEXT("Paper2D", "Paper2D_CreateFlipbooks", "Creating flipbooks from selection"), true, true);

		int Progress = 0;
		int TotalProgress = SpriteFlipbookMap.Num();

		// Create the flipbook
		bool bOneFlipbookCreated = SpriteFlipbookMap.Num() == 1;
		for (auto Iter : SpriteFlipbookMap)
		{
			GWarn->UpdateProgress(Progress++, TotalProgress);

			TArray<UPaperSprite*> Sprites = Iter.Value;

			const FString SpritePathName = AllSprites[0]->GetOutermost()->GetPathName();
			const FString LongPackagePath = FPackageName::GetLongPackagePath(AllSprites[0]->GetOutermost()->GetPathName());

			const FString NewFlipBookDefaultPath = LongPackagePath + TEXT("/") + FlipbookName;
			FString DefaultSuffix;
			FString AssetName = TEXT("test");
			FString PackageName;

			UPaperFlipbookFactory* FlipbookFactory = NewObject<UPaperFlipbookFactory>();
			for (int32 SpriteIndex = 0; SpriteIndex < Sprites.Num(); ++SpriteIndex)
			{
				UPaperSprite* Sprite = Sprites[SpriteIndex];
				FPaperFlipbookKeyFrame* KeyFrame = new (FlipbookFactory->KeyFrames) FPaperFlipbookKeyFrame();
				KeyFrame->Sprite = Sprite;
				KeyFrame->FrameRun = 1;
			}

			AssetToolsModule.Get().CreateUniqueAssetName(NewFlipBookDefaultPath, DefaultSuffix, /*out*/ PackageName, /*out*/ AssetName);
			const FString PackagePath = FPackageName::GetLongPackagePath(PackageName);

			if (UObject* NewAsset = AssetToolsModule.Get().CreateAsset(AssetName, PackagePath, UPaperFlipbook::StaticClass(), FlipbookFactory))
			{
				ObjectsToSync.Add(NewAsset);
			}


			if (GWarn->ReceivedUserCancel())
			{
				break;
			}
		}

		GWarn->EndSlowTask();

		if (ObjectsToSync.Num() > 0)
		{
			ContentBrowserModule.Get().SyncBrowserToAssets(ObjectsToSync);
		}
	}
	return true;
}