#include "VivSpriteParser.h"

#include "AssetRegistry/AssetRegistryModule.h"

#include "ImageUtils.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UObject/UObjectGlobals.h"

VivSpriteParser::VivSpriteParser(FString _FilePath) 
	: FilePath(_FilePath)
	, ResourceName()
	, NumSpriteSheets()
	, imageData() {
	importVivSprite();
}

VivSpriteParser::~VivSpriteParser() {


}

bool VivSpriteParser::importVivSprite() {
	//temp test shit


	//Find the file

	//Unzip the file

	//add files to unreal engine (with settings)
	//std::string testString = "test";
	FString jsonPath = FilePath + "\\" + JsonFileName;
	ParseJSONFile(jsonPath);
	UE_LOG(LogTemp, Warning, TEXT("Resource name: %s"), *ResourceName);
	UE_LOG(LogTemp, Warning, TEXT("Num Images: %d"), NumSpriteSheets);
	for (SpriteSheetData d : imageData) {
		UE_LOG(LogTemp, Warning, TEXT("%s"), *d.name);
	}
	//parser.CreateTexture(testString, filePath);

	//Set up flip books

	return false;
}

std::vector<std::string> VivSpriteParser::GetImageInitalizationData(FString* JsonData) {
	return std::vector<std::string>();
}

void VivSpriteParser::SetTextureSettings(UTexture2D* texture, TArray<TSharedPtr<FJsonValue>> JsonData) {
	return;
}


UTexture2D* VivSpriteParser::CreateTexture(FString* textureName, FString* texturePath) {
	FString FileName = *texturePath + *textureName + ".png";

	FString PackageName = TEXT("/Game/SpriteSheets/");
	PackageName += *ResourceName + *textureName;
	UPackage* Package = CreatePackage(*PackageName);
	Package->FullyLoad();

	UTexture2D* newTexture = ImportFileAsTexture2D(FileName, Package, textureName);

	newTexture->SetExternalPackage(Package);

	FString PackageFileName = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());
	bool bSaved = UPackage::SavePackage(Package, newTexture, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *PackageFileName, GError, nullptr, true, true, SAVE_NoError);

	return newTexture;
}




void VivSpriteParser::ParseJSONFile(FString filePath) {
	FString JsonBlob;
	FFileHelper::LoadFileToString(JsonBlob, *filePath);

	TSharedPtr<FJsonObject> jsonObj = MakeShareable(new FJsonObject);
	TSharedRef<TJsonReader<>> reader = TJsonReaderFactory<>::Create(JsonBlob);

	if (FJsonSerializer::Deserialize(reader, jsonObj) && jsonObj.IsValid()) {
		ResourceName = jsonObj->GetStringField("name");
		NumSpriteSheets = jsonObj->GetNumberField("numImages");
		TArray<TSharedPtr<FJsonValue>> imageSettings = jsonObj->GetArrayField("images");
		for (int32 i = 0; i < imageSettings.Num(); i++) {
			SpriteSheetData data;
			const TSharedPtr<FJsonObject> arrayObj = imageSettings[i]->AsObject();
			data.name = arrayObj->GetStringField("name");
			data.settings = arrayObj->GetArrayField("settings");
			imageData.push_back(data);
		}

	}
}

UTexture2D* VivSpriteParser::ImportFileAsTexture2D(const FString& Filename, UPackage* destination, FString* textureName) {
	IImageWrapperModule& ImageWrapperModule = FModuleManager::Get().LoadModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));

	UTexture2D* NewTexture = nullptr;
	TArray<uint8> Buffer;
	if (FFileHelper::LoadFileToArray(Buffer, *Filename))
	{
		EPixelFormat PixelFormat = PF_Unknown;

		uint8* RawData = nullptr;
		int32 BitDepth = 0;
		int32 Width = 0;
		int32 Height = 0;

		NewTexture = ImportBufferAsTexture2D(Buffer, destination, textureName);

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

UTexture2D* VivSpriteParser::ImportBufferAsTexture2D(const TArray<uint8>& Buffer, UPackage* destination, FString* textureName) {
	IImageWrapperModule& ImageWrapperModule = FModuleManager::Get().LoadModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));
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

			FName TextureName = FName(*textureName);
			NewTexture = NewObject<UTexture2D>(destination, TextureName, RF_Public | RF_Standalone | RF_MarkAsRootSet);

			NewTexture->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
			NewTexture->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
			NewTexture->SRGB = false;
			NewTexture->UpdateResource();

			FTexture2DMipMap* Mip = new(NewTexture->PlatformData->Mips) FTexture2DMipMap();
			Mip->SizeX = Width;
			Mip->SizeY = Height;

			if (NewTexture)
			{
				uint8* MipData = static_cast<uint8*>(NewTexture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE));

				// Bulk data was already allocated for the correct size when we called CreateTransient above
				FMemory::Memcpy(MipData, UncompressedData.GetData(), NewTexture->PlatformData->Mips[0].BulkData.GetBulkDataSize());
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

