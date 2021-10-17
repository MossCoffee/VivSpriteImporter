#include "VivSpriteWidget.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SCheckBox.h"

#define LOCTEXT_NAMESPACE "VivSpriteImportWidget"

SVivSpriteImportWidget::SVivSpriteImportWidget() {}

SVivSpriteImportWidget::~SVivSpriteImportWidget() {}

void SVivSpriteImportWidget::Construct(const FArguments& InArgs) {
	//This is where all the building of the widget goes
	// Put your tab content here!
	FText WidgetText = FText::FromString("TEST");

	ChildSlot
	[
		SNew(SBox)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(WidgetText)
		]
	];
}