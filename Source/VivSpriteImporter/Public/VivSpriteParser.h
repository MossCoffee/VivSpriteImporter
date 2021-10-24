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

struct SpriteSheetData {
	FString name;
	TArray<TSharedPtr<FJsonValue>> settings;
	UTexture2D* texture;
};

class VivSpriteParser {
public:
	VivSpriteParser(FString FilePath);
	virtual ~VivSpriteParser();

private:
	std::vector<std::string> GetImageInitalizationData(FString* JsonData);
	void SetTextureSettings(UTexture2D* texture, TArray<TSharedPtr<FJsonValue>> JsonData);
	UTexture2D* ImportFileAsTexture2D(const FString& Filename, UPackage* destination, FString* textureName);
	UTexture2D* ImportBufferAsTexture2D(const TArray<uint8>& Buffer, UPackage* destination, FString* textureName);
	void ParseJSONFile(FString filePath);
	bool importVivSprite();
	UTexture2D* CreateTexture(FString* textureName, FString* texturePath);

	const FString JsonFileName = "settings.json";
	FString FilePath;
	FString ResourceName;
	int NumSpriteSheets;
	std::vector<SpriteSheetData> imageData;

};