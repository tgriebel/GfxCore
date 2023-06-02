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
#include <systemUtils.h>

bool ModelLoader::Load( Model& model )
{
	if ( modelExt == "obj" ) {
		return LoadRawModel( *assets, modelName + "." + modelExt, modelPath, texturePath, model );
	} else if( modelExt == "mdl" ) {
		return false;
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