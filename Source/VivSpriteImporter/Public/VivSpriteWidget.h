

#pragma once

#include "CoreMinimal.h"
#include <string>
#include "Widgets/SCompoundWidget.h"
#include "Framework/SlateDelegates.h"

class SVivSpriteImportWidget : public SCompoundWidget {
public:
	SVivSpriteImportWidget();
	virtual ~SVivSpriteImportWidget();

	SLATE_BEGIN_ARGS(SVivSpriteImportWidget)
	{}
	SLATE_END_ARGS();

	void Construct(const FArguments& InArgs);
	void ImportFile(std::string& filePath);
	void CreateTexture(std::string& textureName, std::string& texturePath);

private:
	UTexture2D* ImportFileAsTexture2D(const FString& Filename, UPackage* destination, std::string& textureName);
	UTexture2D* ImportBufferAsTexture2D(const TArray<uint8>& Buffer, UPackage* destination, std::string& textureName);
};