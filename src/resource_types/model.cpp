#include "model.h"
#include <systemUtils.h>

bool ModelLoader::Load( Model& model )
{
	return LoadRawModel( *assets, modelName + "." + modelExt, modelPath, texturePath, model );
}


void ModelLoader::SetTexturePath( const std::string& path )
{
	texturePath = path;
}


void ModelLoader::SetModelPath( const std::string& path )
{
	modelPath = path;
}


void ModelLoader::SetModelName( const std::string& fileName )
{
	SplitFileName( fileName, modelName, modelExt );
}


void ModelLoader::SetAssetRef( AssetManager* assetsPtr )
{
	assets = assetsPtr;
}