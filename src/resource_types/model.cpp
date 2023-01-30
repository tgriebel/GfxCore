#include "model.h"
#include <systemUtils.h>

bool ModelLoader::Load( Model& model )
{
	if ( modelExt == "obj" ) {
		return LoadRawModel( *assets, modelName + "." + modelExt, modelPath, texturePath, model );
	} else if( modelExt == "mdl" ) {
	//	return LoadModel( *assets, modelName + "." + modelExt, modelPath, texturePath, model );
	} else {
		return false;
	}
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