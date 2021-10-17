

#pragma once

#include "Widgets/SCompoundWidget.h"

class SVivSpriteImportWidget : public SCompoundWidget {
public:
	SVivSpriteImportWidget();
	virtual ~SVivSpriteImportWidget();

	SLATE_BEGIN_ARGS(SVivSpriteImportWidget)
	{}
	SLATE_END_ARGS();

	void Construct(const FArguments& InArgs);
};