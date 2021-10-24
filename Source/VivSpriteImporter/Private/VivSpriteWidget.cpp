#include "VivSpriteWidget.h"

#include "Framework/SlateDelegates.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SButton.h"

#define LOCTEXT_NAMESPACE "VivSpriteImportWidget"

SVivSpriteImportWidget::SVivSpriteImportWidget() {}

SVivSpriteImportWidget::~SVivSpriteImportWidget() {}

void SVivSpriteImportWidget::Construct(const FArguments& InArgs) {
	//This is where all the building of the widget goes
	// Put your tab content here!
	FText WidgetText = FText::FromString("TEST");
	UE_LOG(LogTemp, Warning, TEXT("Contruct Function"));
	FOnClicked clickFunction;
	clickFunction.BindLambda([&]() {

		std::string filePath = "C:\\Users\\thefa\\Downloads\\Mobility";
		ImportFile(filePath);
		return FReply::Handled();
	});

	ChildSlot
	[
		SNew(SBox)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SButton)
			.OnClicked(clickFunction)
			.Text(WidgetText)
		]
	];
}

void SVivSpriteImportWidget::ImportFile(std::string& filePath) {
	VivSpriteParser parser(FString(filePath.c_str()));
}