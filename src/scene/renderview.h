#pragma once

#include "../math/matrix.h"
#include "../core/handle.h"
#include "scene.h"

const uint32_t	MaxViews = 6;
const uint32_t	MaxModels = 1000;
const uint32_t	MaxLights = 3;
const uint32_t	MaxVertices = 0x000FFFFF;
const uint32_t	MaxIndices = 0x000FFFFF;
const uint32_t	MaxSurfaces = MaxModels * MaxViews;

enum renderFlags_t : uint32_t
{
	NONE = 0,
	HIDDEN = ( 1 << 0 ),
	NO_SHADOWS = ( 1 << 1 ),
	WIREFRAME = ( 1 << 2 ),
	DEBUG_SOLID = ( 1 << 3 ),
	SKIP_OPAQUE = ( 1 << 4 ),
	COMMITTED = ( 1 << 5 ),
};


struct drawSurf_t
{
	uint32_t			vertexOffset;
	uint32_t			vertexCount;
	uint32_t			firstIndex;
	uint32_t			indicesCnt;
	uint32_t			objectId;
	uint32_t			materialId;
	renderFlags_t		flags;
	uint8_t				stencilBit;
	uint32_t			hash;

	hdl_t				pipelineObject[ DRAWPASS_COUNT ];
};


inline uint32_t Hash( const drawSurf_t& surf ) {
	uint64_t shaderIds[ DRAWPASS_COUNT ];
	for ( uint32_t i = 0; i < DRAWPASS_COUNT; ++i ) {
		shaderIds[ i ] = surf.pipelineObject[ i ].Get();
	}
	uint32_t shaderHash = Hash( reinterpret_cast<const uint8_t*>( &shaderIds ), sizeof( shaderIds[ 0 ] ) * DRAWPASS_COUNT );
	uint32_t stateHash = Hash( reinterpret_cast<const uint8_t*>( &surf ), offsetof( drawSurf_t, hash ) );
	return ( shaderHash ^ stateHash );
}

class RenderView
{
public:
	RenderView()
	{
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)DEFAULT_DISPLAY_WIDTH;
		viewport.height = (float)DEFAULT_DISPLAY_HEIGHT;
		viewport.near = 1.0f;
		viewport.far = 0.0f;

		committedModelCnt = 0;
		mergedModelCnt = 0;
	}

	mat4x4f										viewMatrix;
	mat4x4f										projMatrix;
	mat4x4f										viewprojMatrix;
	viewport_t									viewport;
	light_t										lights[ MaxLights ];

	uint32_t									committedModelCnt;
	uint32_t									mergedModelCnt;
	drawSurf_t									surfaces[ MaxModels ];
	drawSurf_t									merged[ MaxModels ];
	drawSurfInstance_t							instances[ MaxModels ];
	uint32_t									instanceCounts[ MaxModels ];
};