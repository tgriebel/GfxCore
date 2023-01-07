#pragma once

#include <cinttypes>
#include <vector>
#include "../acceleration/aabb.h"
#include "../primitives/geom.h"
#include "../core/handle.h"

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