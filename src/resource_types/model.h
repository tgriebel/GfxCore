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
	Model() : surfCount( 0 ), uploaded( false ) {}

	static const uint32_t		MaxSurfaces = 10;
	AABB						bounds;
	Surface						surfs[ MaxSurfaces ];
	surfaceUpload_t				upload[ MaxSurfaces ];
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
	AssetManager* assets;

	bool Load( Model& model )
	{
		return LoadRawModel( *assets, modelName + ".obj", modelPath, texturePath, model );
	}

public:
	void SetTexturePath( const std::string& path )
	{
		texturePath = path;
	}

	void SetModelPath( const std::string& path )
	{
		modelPath = path;
	}

	void SetModelName( const std::string& fileName )
	{
		modelName = fileName;
	}

	void SetAssetRef( AssetManager* assetsPtr )
	{
		assets = assetsPtr;
	}
};

using loader_t = Asset<Model>::loadHandlerPtr_t;