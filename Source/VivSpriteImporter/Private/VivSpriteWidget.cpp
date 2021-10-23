#include "VivSpriteWidget.h"

#include "AssetRegistry/AssetRegistryModule.h"

#include "Framework/SlateDelegates.h"
#include "ImageUtils.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UObject/UObjectGlobals.h"

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

		std::string filePath = "c:/test.png";
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
	//temp test shit
	

	//Find the file

	//Unzip the file

	//add files to unreal engine (with settings)
	std::string testString = "washingmachine";
	CreateTexture(testString, filePath);

	//Set up flip books
	
}


void SVivSpriteImportWidget::CreateTexture(std::string& textureName, std::string& texturePath) {
	FString FileName = FString(texturePath.c_str());
	UE_LOG(LogTemp, Warning, TEXT("The file was found: %s"), (FPaths::FileExists(FileName) ? TEXT("true") : TEXT("false")));

	FString PackageName = TEXT("/Game/SpriteSheets/");
	PackageName += FString(textureName.c_str());
	UPackage* Package = CreatePackage(*PackageName);
	Package->FullyLoad();

	UTexture2D* newTexture = ImportFileAsTexture2D(FileName, Package, textureName);

	newTexture->SetExternalPackage(Package);

	UE_LOG(LogTemp, Warning, TEXT("The integer value is: %d"), newTexture->PlatformData->SizeX);

	FString PackageFileName = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());
	bool bSaved = UPackage::SavePackage(Package, newTexture, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *PackageFileName, GError, nullptr, true, true, SAVE_NoError);

	UE_LOG(LogTemp, Warning, TEXT("Created Texture"));
	
}



UTexture2D* SVivSpriteImportWidget::ImportFileAsTexture2D(const FString& Filename, UPackage* destination, std::string& textureName)
{
	IImageWrapperModule& ImageWrapperModule = FModuleManager::Get().LoadModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));

	UTexture2D* NewTexture = nullptr;
	TArray<uint8> Buffer;
	if (FFileHelper::LoadFileToArray(Buffer, *Filename))
	{
		UE_LOG(LogTemp, Warning, TEXT("Created Buffer of size %d"), Buffer.Num());
		EPixelFormat PixelFormat = PF_Unknown;

		uint8* RawData = nullptr;
		int32 BitDepth = 0;
		int32 Width = 0;
		int32 Height = 0;

		NewTexture = SVivSpriteImportWidget::ImportBufferAsTexture2D(Buffer, destination, textureName);
		UE_LOG(LogTemp, Warning, TEXT("ImportBufferAsTexture2D result size: %d"), NewTexture->PlatformData->SizeX);

		if (!NewTexture)
		{
			UE_LOG(LogTemp, Warning, TEXT("Error creating texture. %s is not a supported file format"), *Filename)
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Error creating texture. %s could not be found"), *Filename)
	}

	return NewTexture;
}

UTexture2D* SVivSpriteImportWidget::ImportBufferAsTexture2D(const TArray<uint8>& Buffer, UPackage* destination, std::string& textureName)
{
	IImageWrapperModule& ImageWrapperModule = FModuleManager::Get().LoadModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));
	UE_LOG(LogTemp, Warning, TEXT("Buffer.GetData %d"), Buffer.GetAllocatedSize());
	EImageFormat Format = ImageWrapperModule.DetectImageFormat(Buffer.GetData(), Buffer.GetAllocatedSize());

	UTexture2D* NewTexture = nullptr;
	EPixelFormat PixelFormat = PF_Unknown;

	if (Format != EImageFormat::Invalid)
	{
		TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(Format);

		int32 BitDepth = 0;
		int32 Width = 0;
		int32 Height = 0;

		if (ImageWrapper->SetCompressed((void*)Buffer.GetData(), Buffer.GetAllocatedSize()))
		{
			PixelFormat = PF_Unknown;

			ERGBFormat RGBFormat = ERGBFormat::Invalid;

			BitDepth = ImageWrapper->GetBitDepth();

			Width = ImageWrapper->GetWidth();
			Height = ImageWrapper->GetHeight();
			
			UE_LOG(LogTemp, Warning, TEXT("ImageWrapperWidth %d, ImageWrapperHeight %d"), Width, Height);

			if (BitDepth == 16)
			{
				PixelFormat = PF_FloatRGBA;
				RGBFormat = ERGBFormat::BGRA;
			}
			else if (BitDepth == 8)
			{
				PixelFormat = PF_B8G8R8A8;
				RGBFormat = ERGBFormat::BGRA;
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Error creating texture. Bit depth is unsupported. (%d)"), BitDepth);
				return nullptr;
			}

			TArray64<uint8> UncompressedData;
			bool result = ImageWrapper->GetRaw(RGBFormat, BitDepth, UncompressedData);
			UE_LOG(LogTemp, Warning, TEXT("get raw result %s"), result ? TEXT("true") : TEXT("false"));

			FName TextureName = FName(textureName.c_str());
			NewTexture = NewObject<UTexture2D>(destination, TextureName, RF_Public | RF_Standalone | RF_MarkAsRootSet);
			
			NewTexture->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
			NewTexture->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
			NewTexture->SRGB = false;
			NewTexture->UpdateResource();

			UE_LOG(LogTemp, Warning, TEXT("NewTexture size: %d"), NewTexture->PlatformData->SizeX);

			FTexture2DMipMap* Mip = new(NewTexture->PlatformData->Mips) FTexture2DMipMap();
			Mip->SizeX = Width;
			Mip->SizeY = Height;
			UE_LOG(LogTemp, Warning, TEXT("MipSizeX %d, MipSizeY %d"), Mip->SizeX, Mip->SizeY);

			if (NewTexture)
			{
				uint8* MipData = static_cast<uint8*>(NewTexture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE));

				// Bulk data was already allocated for the correct size when we called CreateTransient above
				FMemory::Memcpy(MipData, UncompressedData.GetData(), NewTexture->PlatformData->Mips[0].BulkData.GetBulkDataSize());
				UE_LOG(LogTemp, Warning, TEXT("NewTexture size: %d"), NewTexture->PlatformData->SizeX);
				NewTexture->PlatformData->Mips[0].BulkData.Unlock();

				NewTexture->Source.Init(Width, Height, 1, 1, ETextureSourceFormat::TSF_BGRA8, UncompressedData.GetData());
				NewTexture->UpdateResource();
				destination->MarkPackageDirty();
				FAssetRegistryModule::AssetCreated(NewTexture);

			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Error creating texture. Couldn't determine the file format"));
	}

	return NewTexture;
}