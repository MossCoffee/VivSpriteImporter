

#pragma once

#include "CoreMinimal.h"
#include <string>
#include "Widgets/SCompoundWidget.h"
#include "Framework/SlateDelegates.h"
#include "VivSpriteParser.h"

class SVivSpriteImportWidget : public SCompoundWidget {
public:
	SVivSpriteImportWidget();
	virtual ~SVivSpriteImportWidget();

	SLATE_BEGIN_ARGS(SVivSpriteImportWidget)
	{}
	SLATE_END_ARGS();

	void Construct(const FArguments& InArgs);
	void ImportFile(std::string& filePath);

private:
};