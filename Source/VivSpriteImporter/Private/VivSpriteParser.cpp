#include "VivSpriteParser.h"

#include "AssetRegistry/AssetRegistryModule.h"

#include "Modules/ModuleManager.h"
#include "TextureCompressorModule.h"
#include "Engine/Texture.h"

#include "ImageCore.h"
#include "ImageUtils.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UObject/UObjectGlobals.h"

VivSpriteParser::VivSpriteParser(FString _FilePath) 
	: FilePath(_FilePath)
	, ResourceName()
	, Subfolder()
	, NumSpriteSheets(0)
	, imageData() {
	importVivSprite();
}

VivSpriteParser::~VivSpriteParser() {


}

bool VivSpriteParser::importVivSprite() {
	//Find the file - disabled until we switch to zip files
	//if (!FPaths::FileExists(FilePath)) {
	//	UE_LOG(LogTemp, Error, TEXT("No file found at: %s, aborting import"), *FilePath);
	//	return false;
	//}
	//Unzip the file
	bool unzipSuccess = UnzipFile();
	if (!unzipSuccess) {
		UE_LOG(LogTemp, Error, TEXT("Error unzipping, aborting import"), *FilePath);
		return false;
	}

	//add files to unreal engine (with settings)
	FString jsonPath = FilePath + "\\" + JsonFileName;
	bool ParseSuccess = ParseJSONFile(jsonPath);
	if (!ParseSuccess) {
		return false;
	}
	
	for (SpriteSheetData& data : imageData) {
		data.texture = CreateTexture(data.name, data.settings);
		if (data.texture == nullptr) {
			UE_LOG(LogTemp, Error, TEXT("Error creating texture %s, aborting import"), *data.name);
			return false;
		}
	}

	//Set up flip books
	bool flipbookSuccess = createFlipbooks();
	if (!flipbookSuccess) {
		UE_LOG(LogTemp, Error, TEXT("Error creating flipbooks, aborting import"));
		return false;
	}

	return true;
}

bool VivSpriteParser::UnzipFile() {
	//Always returns success because it's not currently implemented
	return true;
}

bool VivSpriteParser::createFlipbooks() {
	//Always returns success because they aren't currently implemented
	return true;
}

void VivSpriteParser::SetTextureSettings(UTexture2D* texture, TArray<TSharedPtr<FJsonValue>>& JsonData) {
	//MipGenSettings
	//Texture Group
	//Downscale
	//Compression Settings
	texture->CompressionSettings = TextureCompressionSettings::TC_Grayscale;
	//Compress w/o alpha
	texture->CompressionNoAlpha = 1;
	//sRGB
	texture->SRGB = 0;
	//Filter
	texture->Filter = TextureFilter::TF_Default;

	return;
}


UTexture2D* VivSpriteParser::CreateTexture(FString textureName, TArray<TSharedPtr<FJsonValue>>& textureSettings) {
	FString FileName = FilePath + "\\" + textureName + ".png";
	if (!FPaths::FileExists(FileName)) {
		UE_LOG(LogTemp, Error, TEXT("No file found at: %s"), *FileName);
		return nullptr;
	}
	FString PackageName = TEXT("/Game/SpriteSheets/") + Subfolder + TEXT("/");
	FString FullTextureName = ResourceName + TEXT("_") + textureName;
	PackageName += FullTextureName;
	UPackage* Package = CreatePackage(*PackageName);
	Package->FullyLoad();

	UTexture2D* newTexture = ImportFileAsTexture2D(FileName, Package, FullTextureName);

	SetTextureSettings(newTexture, textureSettings);

	newTexture->SetExternalPackage(Package);

	FString PackageFileName = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());
	bool bSaved = UPackage::SavePackage(Package, newTexture, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *PackageFileName, GError, nullptr, true, true, SAVE_NoError);

	return newTexture;
}


bool VivSpriteParser::ParseJSONFile(FString filePath) {
	if (!FPaths::FileExists(filePath)) {
		UE_LOG(LogTemp, Error, TEXT("No file found at: %s"), *filePath);
		return false;
	}
	FString JsonBlob;
	FFileHelper::LoadFileToString(JsonBlob, *filePath);

	TSharedPtr<FJsonObject> jsonObj = MakeShareable(new FJsonObject);
	TSharedRef<TJsonReader<>> reader = TJsonReaderFactory<>::Create(JsonBlob);

	if (FJsonSerializer::Deserialize(reader, jsonObj) && jsonObj.IsValid()) {
		ResourceName = jsonObj->HasField("name") ? jsonObj->GetStringField("name") : TEXT("default_path");
		
		Subfolder = jsonObj->HasField("subfolder") ? jsonObj->GetStringField("subfolder") : ResourceName;
		if (Subfolder.Len() == 0) {
			Subfolder = ResourceName;
		}
		TArray<TSharedPtr<FJsonValue>> imageSettings = jsonObj->GetArrayField("images");
		for (int32 i = 0; i < imageSettings.Num(); i++) {
			SpriteSheetData data;
			const TSharedPtr<FJsonObject> arrayObj = imageSettings[i]->AsObject();
			data.name = arrayObj->GetStringField("name");
			data.settings = arrayObj->GetArrayField("settings");
			imageData.push_back(data);
			NumSpriteSheets++;
		}

	}
	return true;
}

FCompressedImage2D VivSpriteParser::RunImagePreprocessor(const TArray64<uint8>& Buffer, int32 Width, int32 Height) {
	//Create an FImage
	TArray<FImage> source = {FImage(Width, Height, ERawImageFormat::Type::BGRA8, EGammaSpace::Linear)};
	TArray<FImage> normals;
	

	ITextureCompressorModule* compressor = &FModuleManager::LoadModuleChecked<ITextureCompressorModule>(TEXTURE_COMPRESSOR_MODULENAME);
	//Create & set Build Settings
	FTextureBuildSettings settings;
	//Create an out array
	TArray<FCompressedImage2D> output;
	//call build texture
	uint32 mipsInTail = 0;
	uint32 ExtData = 0;
	compressor->BuildTexture(source, normals, settings, output, mipsInTail, ExtData);

	return output[0]; 

}


UTexture2D* VivSpriteParser::ImportFileAsTexture2D(const FString& Filename, UPackage* destination, FString& textureName) {
	IImageWrapperModule& ImageWrapperModule = FModuleManager::Get().LoadModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));

	UTexture2D* NewTexture = nullptr;
	TArray<uint8> Buffer;
	if (FFileHelper::LoadFileToArray(Buffer, *Filename))
	{
		//Run Compression Settings here
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

UTexture2D* VivSpriteParser::ImportBufferAsTexture2D(const TArray<uint8>& Buffer, UPackage* destination, FString& textureName) {
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

			if (BitDepth == 8)
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

			FCompressedImage2D ProcessedTexture = RunImagePreprocessor(UncompressedData, Width, Height);

			FName TextureName = FName(textureName);
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

				FMemory::Memcpy(MipData, ProcessedTexture.RawData.GetData(), NewTexture->PlatformData->Mips[0].BulkData.GetBulkDataSize());
				NewTexture->PlatformData->Mips[0].BulkData.Unlock();

				NewTexture->Source.Init(Width, Height, 1, 1, ETextureSourceFormat::TSF_BGRA8, ProcessedTexture.RawData.GetData());
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

