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

#pragma once

#include <cinttypes>
#include <vector>
#include "../acceleration/aabb.h"
#include "../primitives/geom.h"
#include "../core/handle.h"
#include "../core/asset.h"
#include "../io/io.h"

class Surface {
public:
	hdl_t						materialHdl;
	std::vector<vertex_t>		vertices;
	std::vector<uint32_t>		indices;
	vec3f						centroid;

	void Serialize( Serializer* serializer );
};


struct surfaceUpload_t
{
	surfaceUpload_t() : vertexCount( 0 ), indexCount( 0 ), vertexOffset( 0 ), firstIndex( 0 ) {}

	uint32_t					vertexCount;
	uint32_t					indexCount;
	uint32_t					vertexOffset;
	uint32_t					firstIndex;
};


class Model
{
	static const uint32_t Version = 1;
public:
	Model() : surfCount( 0 ), uploaded( false )
	{
	}

	AABB						bounds;
	std::vector<Surface>		surfs;
	std::vector<surfaceUpload_t> upload;
	uint32_t					surfCount;
	bool						uploaded;

	void Serialize( Serializer* serializer );
};


class ModelLoader : public LoadHandler<Model>
{
private:
	std::string texturePath;
	std::string modelPath;
	std::string modelName;
	std::string modelExt;
	AssetManager* assets;

	bool Load( Model& model );

public:
	void SetTexturePath( const std::string& path );
	void SetModelPath( const std::string& path );
	void SetModelName( const std::string& fileName );
	void SetAssetRef( AssetManager* assetsPtr );
};

using loader_t = Asset<Model>::loadHandlerPtr_t;