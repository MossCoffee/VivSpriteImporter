#include "VivSpriteParser.h"

#include "AssetRegistry/AssetRegistryModule.h"

#include "Engine/Texture.h"

#include "ImageUtils.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "PaperSprite.h"
//#include "PaperSpriteSheet.h"
#include "PaperFlipbookFactory.h"
#include "SpriteEditorOnlyTypes.h"
#include "VivSpriteFlipbookHelpers.h"
#include "UObject/SavePackage.h"
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
	UE_LOG(LogTemp, Error, TEXT("Importing Viv Sprite"));
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
	UE_LOG(LogTemp, Error, TEXT("past unzip"));
	//add files to unreal engine (with settings)
	FString jsonPath = FilePath + "\\" + JsonFileName;
	bool ParseSuccess = ParseJSONFile(jsonPath);
	if (!ParseSuccess) {
		UE_LOG(LogTemp, Error, TEXT("Parsing Json Data Failed, aborting import"));
		return false;
	}
	FString Sprite2DPath = FilePath + "\\" + Sprite2DFileName;
	bool ParseSprite2DSuccess = ParseSprite2D(Sprite2DPath);
	if (!ParseSprite2DSuccess) {
		UE_LOG(LogTemp, Error, TEXT("Parsing Json Data Failed, aborting import"));
		return false;
	}

	UE_LOG(LogTemp, Error, TEXT("past parse json"));
	for (SpriteSheetData& data : imageData) {
		data.texture = CreateTexture(data.name, data.settings);
		if (data.texture == nullptr) {
			UE_LOG(LogTemp, Error, TEXT("Error creating texture %s, aborting import"), *data.name);
			return false;
		}
	}
	UE_LOG(LogTemp, Error, TEXT("past create texture"));
	//Set up flip books
	bool flipbookSuccess = createFlipbooks();
	if (!flipbookSuccess) {
		UE_LOG(LogTemp, Error, TEXT("Error creating flipbooks, aborting import"));
		return false;
	}
	UE_LOG(LogTemp, Error, TEXT("Job Done!"));
	return true;
}

bool VivSpriteParser::UnzipFile() {
	//Always returns success because it's not currently implemented
	return true;
}

bool VivSpriteParser::createFlipbooks() {

	TArray<TWeakObjectPtr<UPaperSprite>> PaperSpriteArray; //Gonna keep it a buck 50. This is probably supposed to be UPaperSpriteSheet. See: FPaperSpriteSheetAssetTypeActions::GetActions
	TWeakObjectPtr<UPaperSprite> PaperSprite;
	//TWeakObjectPtr <UPaperSpriteSheet> SpriteSheet;

	FSpriteAssetInitParameters Param;
	TArray<UTexture*> AdditionalTextures;
	for (SpriteSheetData& Data : imageData)
	{
		if (Param.Texture == nullptr)
		{
			Param.SetTextureAndFill(Data.texture);
		}
		else
		{
			AdditionalTextures.Add(Data.texture);
		}
	}
	//Put the uvs in the paramters?
	Param.AdditionalTextures = AdditionalTextures;
	//This is where we cut up the texture. We're going to loop through all the uvs & create a new param based on each.\
	//Test
	Param.Offset = FIntPoint(32, 32);
	Param.Dimension = FIntPoint(32, 32);
	//
	PaperSprite = ConvertTexture2DToUPaperSprite(Param);
	if (PaperSprite.IsValid())
	{
		PaperSpriteArray.Add(PaperSprite);
		return FVivSpriteFlipbookHelpers::CreateFlipbook(PaperSpriteArray);
	}
	
	return false;
}

TWeakObjectPtr<UPaperSprite> VivSpriteParser::ConvertTexture2DToUPaperSprite(FSpriteAssetInitParameters& param)
{
	UPaperSprite* PaperSprite = NewObject<UPaperSprite>();
	PaperSprite->InitializeSprite(param);

	FString PaperSpriteName = TEXT("test");

	FString PackageName = TEXT("/Game/SpriteSheets/") + Subfolder + TEXT("/");
	FString FullTextureName = ResourceName + TEXT("_") + PaperSpriteName;
	PackageName += FullTextureName;
	UPackage* Package = CreatePackage(*PackageName);
	Package->FullyLoad();

	PaperSprite->SetExternalPackage(Package);

	FString PackageFileName = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());
	FSavePackageArgs Args(nullptr, nullptr, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, SAVE_NoError, true, true, true, FDateTime::Now(), GError);
	FSavePackageResultStruct FSaveResult = UPackage::Save(Package, PaperSprite, *PackageFileName, Args);

	return TWeakObjectPtr<UPaperSprite>(PaperSprite);
}

void VivSpriteParser::SetTextureSettings(UTexture2D* texture, TSharedPtr<FJsonObject>& JsonData) {
	
	//MipGenSettings
	// It's always no mips
	//Texture Group
	//texture->LODGroup = TextureGroup::TEXTUREGROUP_Character;
	FString TextureGroupString = JsonData->HasField("TextureGroup") ? JsonData->GetStringField("TextureGroup") : TEXT("World");
	if (TextureGroupString == TEXT("World")) {texture->LODGroup = TextureGroup::TEXTUREGROUP_World;}
	else if (TextureGroupString == TEXT("WorldNormalMap")) {texture->LODGroup = TextureGroup::TEXTUREGROUP_WorldNormalMap;}
	else if (TextureGroupString == TEXT("WorldSpecular")) {texture->LODGroup = TextureGroup::TEXTUREGROUP_WorldSpecular;}
	else if (TextureGroupString == TEXT("Character")) {texture->LODGroup = TextureGroup::TEXTUREGROUP_Character;}
	else if (TextureGroupString == TEXT("CharacterNormalMap")) {texture->LODGroup = TextureGroup::TEXTUREGROUP_CharacterNormalMap;}
	else if (TextureGroupString == TEXT("CharacterSpecular")) {texture->LODGroup = TextureGroup::TEXTUREGROUP_CharacterSpecular;}
	else if (TextureGroupString == TEXT("Weapon")) {texture->LODGroup = TextureGroup::TEXTUREGROUP_Weapon;}
	else if (TextureGroupString == TEXT("WeaponNormalMap")) {texture->LODGroup = TextureGroup::TEXTUREGROUP_WeaponNormalMap;}
	else if (TextureGroupString == TEXT("WeaponSpecular")) {texture->LODGroup = TextureGroup::TEXTUREGROUP_WeaponSpecular;}
	else if (TextureGroupString == TEXT("Vehicle")) {texture->LODGroup = TextureGroup::TEXTUREGROUP_Vehicle;}
	else if (TextureGroupString == TEXT("VehicleNormalMap")) {texture->LODGroup = TextureGroup::TEXTUREGROUP_VehicleNormalMap;}
	else if (TextureGroupString == TEXT("VehicleSpecular")) {texture->LODGroup = TextureGroup::TEXTUREGROUP_VehicleSpecular;}
	else if (TextureGroupString == TEXT("Cinematic")) {texture->LODGroup = TextureGroup::TEXTUREGROUP_Cinematic;}
	else if (TextureGroupString == TEXT("Effects")) {texture->LODGroup = TextureGroup::TEXTUREGROUP_Effects;}
	else if (TextureGroupString == TEXT("EffectsNotFiltered")) {texture->LODGroup = TextureGroup::TEXTUREGROUP_EffectsNotFiltered;}
	else if (TextureGroupString == TEXT("SkyBox")) {texture->LODGroup = TextureGroup::TEXTUREGROUP_Skybox;}
	else if (TextureGroupString == TEXT("UI")) {texture->LODGroup = TextureGroup::TEXTUREGROUP_UI;}
	else if (TextureGroupString == TEXT("Lightmap")) {texture->LODGroup = TextureGroup::TEXTUREGROUP_Lightmap;}
	else if (TextureGroupString == TEXT("RenderTarget")) {texture->LODGroup = TextureGroup::TEXTUREGROUP_RenderTarget;}
	else if (TextureGroupString == TEXT("MobileFlattened")) {texture->LODGroup = TextureGroup::TEXTUREGROUP_MobileFlattened;}
	else if (TextureGroupString == TEXT("ProcBuilding_Face")) {texture->LODGroup = TextureGroup::TEXTUREGROUP_ProcBuilding_Face;}
	else if (TextureGroupString == TEXT("ProcBuilding_LightMap")) {texture->LODGroup = TextureGroup::TEXTUREGROUP_ProcBuilding_LightMap;}
	else if (TextureGroupString == TEXT("Shadowmap")) {texture->LODGroup = TextureGroup::TEXTUREGROUP_Shadowmap;}
	else if (TextureGroupString == TEXT("ColorLookupTable")) {texture->LODGroup = TextureGroup::TEXTUREGROUP_ColorLookupTable;}
	else if (TextureGroupString == TEXT("Terrain_Heightmap")) {texture->LODGroup = TextureGroup::TEXTUREGROUP_Terrain_Heightmap;}
	else if (TextureGroupString == TEXT("Terrain_Weightmap")) {texture->LODGroup = TextureGroup::TEXTUREGROUP_Terrain_Weightmap;}
	else if (TextureGroupString == TEXT("Bokeh")) {texture->LODGroup = TextureGroup::TEXTUREGROUP_Bokeh;}
	else if (TextureGroupString == TEXT("IESLightProfile")) {texture->LODGroup = TextureGroup::TEXTUREGROUP_IESLightProfile;}
	else if (TextureGroupString == TEXT("Pixels2D")) {texture->LODGroup = TextureGroup::TEXTUREGROUP_Pixels2D;}
	else if (TextureGroupString == TEXT("HierarchicalLOD")) {texture->LODGroup = TextureGroup::TEXTUREGROUP_HierarchicalLOD;}
	else if (TextureGroupString == TEXT("Impostor")) {texture->LODGroup = TextureGroup::TEXTUREGROUP_Impostor;}
	else if (TextureGroupString == TEXT("ImpostorNormalDepth")) {texture->LODGroup = TextureGroup::TEXTUREGROUP_ImpostorNormalDepth;}
	else if (TextureGroupString == TEXT("8BitData")) {texture->LODGroup = TextureGroup::TEXTUREGROUP_8BitData;}
	else if (TextureGroupString == TEXT("16BitData")) {texture->LODGroup = TextureGroup::TEXTUREGROUP_16BitData;}
	/*else if (TextureGroupString == TEXT("")) {texture->LODGroup = TextureGroup::TEXTUREGROUP_Project01;}
	else if (TextureGroupString == TEXT("")) {texture->LODGroup = TextureGroup::TEXTUREGROUP_Project02;}
	else if (TextureGroupString == TEXT("")) {texture->LODGroup = TextureGroup::TEXTUREGROUP_Project03;}
	else if (TextureGroupString == TEXT("")) {texture->LODGroup = TextureGroup::TEXTUREGROUP_Project04;}
	else if (TextureGroupString == TEXT("")) {texture->LODGroup = TextureGroup::TEXTUREGROUP_Project05;}
	else if (TextureGroupString == TEXT("")) {texture->LODGroup = TextureGroup::TEXTUREGROUP_Project06;}
	else if (TextureGroupString == TEXT("")) {texture->LODGroup = TextureGroup::TEXTUREGROUP_Project07;}
	else if (TextureGroupString == TEXT("")) {texture->LODGroup = TextureGroup::TEXTUREGROUP_Project08;}
	else if (TextureGroupString == TEXT("")) {texture->LODGroup = TextureGroup::TEXTUREGROUP_Project09 ;}
	else if (TextureGroupString == TEXT("")) {texture->LODGroup = TextureGroup::TEXTUREGROUP_Project10 ;}
	else if (TextureGroupString == TEXT("")) {texture->LODGroup = TextureGroup::TEXTUREGROUP_Project11 ;}
	else if (TextureGroupString == TEXT("")) {texture->LODGroup = TextureGroup::TEXTUREGROUP_Project12 ;}
	else if (TextureGroupString == TEXT("")) {texture->LODGroup = TextureGroup::TEXTUREGROUP_Project13;}
	else if (TextureGroupString == TEXT("")) {texture->LODGroup = TextureGroup::TEXTUREGROUP_Project14;}
	else if (TextureGroupString == TEXT("")) {texture->LODGroup = TextureGroup::TEXTUREGROUP_Project15;} will add these if needed*/
	else {texture->LODGroup = TextureGroup::TEXTUREGROUP_MAX;}
	//Downscale
	texture->Downscale = JsonData->HasField("Downscale") ? JsonData->GetNumberField("Downscale") : 1;
	//Compression Settings
	//texture->CompressionSettings = TextureCompressionSettings::TC_Grayscale;
	FString CompressionString = JsonData->HasField("CompressionSettings") ? JsonData->GetStringField("CompressionSettings") : TEXT("Default");
	if(CompressionString == TEXT("Default")) {texture->CompressionSettings = TextureCompressionSettings::TC_Default				;}	
	else if(CompressionString == TEXT("NormalMap")) {texture->CompressionSettings = TextureCompressionSettings::TC_Normalmap				;}
	else if(CompressionString == TEXT("Masks")) {texture->CompressionSettings = TextureCompressionSettings::TC_Masks					;}
	else if(CompressionString == TEXT("Grayscale") || CompressionString == TEXT("Greyscale")) {texture->CompressionSettings = TextureCompressionSettings::TC_Grayscale				;}
	else if(CompressionString == TEXT("Displacementmap")) {texture->CompressionSettings = TextureCompressionSettings::TC_Displacementmap		;}	
	else if(CompressionString == TEXT("VectorDisplacementmap")) {texture->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap	;}
	else if(CompressionString == TEXT("HDR")) {texture->CompressionSettings = TextureCompressionSettings::TC_HDR					;}	
	else if(CompressionString == TEXT("EditorIcon")) {texture->CompressionSettings = TextureCompressionSettings::TC_EditorIcon			;}	
	else if(CompressionString == TEXT("Alpha")) {texture->CompressionSettings = TextureCompressionSettings::TC_Alpha					;}
	else if(CompressionString == TEXT("DistanceFieldFont")) {texture->CompressionSettings = TextureCompressionSettings::TC_DistanceFieldFont		;} 
	else if(CompressionString == TEXT("HDRCompressed")) {texture->CompressionSettings = TextureCompressionSettings::TC_HDR_Compressed		;}	
	else if(CompressionString == TEXT("BC7")) {texture->CompressionSettings = TextureCompressionSettings::TC_BC7					;}	
	else if(CompressionString == TEXT("HalfFloat")) {texture->CompressionSettings = TextureCompressionSettings::TC_HalfFloat				;}
	//else if(CompressionString == TEXT("ReflectionCapture")) {texture->CompressionSettings = TextureCompressionSettings::TC_EncodedReflectionCapture		;}
	else {texture->CompressionSettings = TextureCompressionSettings::TC_MAX;}


	//Compress w/o alpha
	texture->CompressionNoAlpha = JsonData->HasField("CompressionNoAlpha") ? JsonData->GetBoolField("CompressionNoAlpha") : false;
	//sRGB
	texture->SRGB = JsonData->HasField("SRGB") ? JsonData->GetBoolField("SRGB") : false;
	//Filter
	FString TextureFilterString = JsonData->HasField("Filter") ? JsonData->GetStringField("Filter") : TEXT("Nearest");
	if(TextureFilterString == TEXT("Nearest")) { texture->Filter = TextureFilter::TF_Nearest; }
	else if(TextureFilterString == TEXT("Bi-linear") || TextureFilterString == TEXT("Bilinear")) { texture->Filter = TextureFilter::TF_Bilinear; }
	else if (TextureFilterString == TEXT("Tri-linear") || TextureFilterString == TEXT("Trilinear")) { texture->Filter = TextureFilter::TF_Trilinear; }
	else if (TextureFilterString == TEXT("Max")) { texture->Filter = TextureFilter::TF_MAX; }
	else { texture->Filter = TextureFilter::TF_Default; }
	return;
}

UTexture2D* VivSpriteParser::CreateTexture(FString textureName, TSharedPtr<FJsonObject>& textureSettings) {
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
	FSavePackageArgs Args(nullptr, nullptr, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, SAVE_NoError, true, true, true, FDateTime::Now(), GError);
	FSavePackageResultStruct FSaveResult = UPackage::Save(Package, newTexture, *PackageFileName, Args);

	//Error Handling goes here

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
		Sprite2DFileName = jsonObj->HasField("paper2dsprite") ? jsonObj->GetStringField("paper2dSprite") : TEXT("default_path.paper2dSp");
		if (Subfolder.Len() == 0) {
			Subfolder = ResourceName;
		}
		TArray<TSharedPtr<FJsonValue>> imageSettings = jsonObj->GetArrayField("images");
		for (int32 i = 0; i < imageSettings.Num(); i++) {
			SpriteSheetData data;
			const TSharedPtr<FJsonObject> arrayObj = imageSettings[i]->AsObject();
			data.name = arrayObj->GetStringField("name");
			data.settings = arrayObj->GetObjectField("settings");
			imageData.push_back(data);
			NumSpriteSheets++;
		}

	}
	return true;
}

bool VivSpriteParser::ParseSprite2D(FString filePath) {
	if (filePath.Len()) {}
	return true;
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

			TArray64<uint8> Data;
			bool result = ImageWrapper->GetRaw(RGBFormat, BitDepth, Data);
			   
			FName TextureName = FName(textureName);
			NewTexture = NewObject<UTexture2D>(destination, TextureName, RF_Public | RF_Standalone | RF_MarkAsRootSet);

			NewTexture->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
			//Note: using this instead of "TMGS_NoMipmaps" because the texture won't render properly without 
			//a mipmap of some kind. If "TMGS_NoMipmaps" is required to keep clarity of the art, then 
			//you have to add the Mip manually
			NewTexture->MipGenSettings = TextureMipGenSettings::TMGS_Unfiltered;
			NewTexture->MipLoadOptions = ETextureMipLoadOptions::OnlyFirstMip;
			NewTexture->SRGB = false;
			NewTexture->UpdateResource();

			FTexturePlatformData* TextureData = NewTexture->GetPlatformData();

			if (NewTexture)
			{
				NewTexture->Source.Init(Width, Height, 1, 1, ETextureSourceFormat::TSF_BGRA8, Data.GetData());
				NewTexture->UpdateResource();
				destination->MarkPackageDirty();
				FAssetRegistryModule::AssetCreated(NewTexture);

				//NewTexture->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
				//FAssetRegistryModule::AssetSaved(*NewTexture);

			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Error creating texture. Couldn't determine the file format"));
	}

	return NewTexture;
}

