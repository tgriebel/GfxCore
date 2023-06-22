/*
* MIT License
*
* Copyright( c ) 2021-2023 Thomas Griebel
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this softwareand associated documentation files( the "Software" ), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions :
*
* The above copyright noticeand this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#include "model.h"
#include <syscore/systemUtils.h>
#include <syscore/serializer.h>
#include "../io/serializeClasses.h"
#include "../scene/assetManager.h"

bool ModelLoader::Load( Asset<Model>& modelAsset )
{
	Model& model = modelAsset.Get();

	const std::string fileName = modelName + "." + modelExt;

	bakedAssetInfo_t modelInfo = {};
	const bool loadedBakedModel = LoadBaked( modelAsset, modelInfo, modelPath, "mdl.bin" );
	if ( loadedBakedModel )
	{
		const uint32_t surfCount = static_cast<uint32_t>( model.surfs.size() );
		for ( uint32_t surfIx = 0; surfIx < surfCount; ++surfIx ) {
			assets->materialLib.AddDeferred( model.surfs[ surfIx ].materialHdl, pMatLoader_t( new BakedMaterialLoader( assets, ".\\materials\\", "mtl.bin" ) ) );
		}
		return true;
	}

	std::cout << "Loading raw model:" << fileName << std::endl;

	if ( modelExt == "obj" ) {
		return LoadRawModel( *assets, fileName, modelPath, texturePath, model );
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