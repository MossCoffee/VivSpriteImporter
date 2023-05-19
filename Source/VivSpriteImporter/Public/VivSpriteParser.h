#pragma once

#include "CoreMinimal.h"
#include "CoreTypes.h"
#include <string>
#include <vector>

#include "Containers/UnrealString.h"
#include "Engine/Texture2D.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"

class UPaperSprite;
struct FSpriteAssetInitParameters;

//MetaData
struct SpriteSheetData {
	FString name;
	TSharedPtr<FJsonObject> settings;
	UTexture2D* texture;
};

struct SpriteSheetUV {
	FString name;
	FIntPoint offset;
	FIntPoint size;
};

class VivSpriteParser {
public:
	VivSpriteParser(FString FilePath);
	virtual ~VivSpriteParser();

private:
	bool UnzipFile();
	void SetTextureSettings(UTexture2D* texture, TSharedPtr<FJsonObject>& JsonData);
	UTexture2D* ImportFileAsTexture2D(const FString& Filename, UPackage* destination, FString& textureName);
	UTexture2D* ImportBufferAsTexture2D(const TArray<uint8>& Buffer, UPackage* destination, FString& textureName);
	bool ParseJSONFile(FString filePath);
	bool ParseSprite2D(FString filePath);
	TWeakObjectPtr<UPaperSprite> ConvertTexture2DToUPaperSprite(FSpriteAssetInitParameters& Param, const FString& name);

	bool importVivSprite();
	UTexture2D* CreateTexture(FString textureName, TSharedPtr<FJsonObject>& textureSettings);
	bool createFlipbooks();

	const TArray<FString> JsonFileNames = { TEXT("import.json"), TEXT("settings.json") };
	FString FilePath;
	FString ResourceName;
	FString Subfolder;
	FString Sprite2DFileName;
	int NumSpriteSheets;
	std::vector<SpriteSheetUV> uvData;
	std::vector<SpriteSheetData> imageData;
};