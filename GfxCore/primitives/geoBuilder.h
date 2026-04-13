/*
* MIT License
*
* Copyright( c ) 2023 Thomas Griebel
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

#include "../core/common.h"

#include <vector>

// template<typename Vec>
class GeoBuilder
{
public:

	enum winding_t
	{
		WINDING_CLOCKWISE = 0,
		WINDING_COUNTER_CLOCKWISE = 1,
	};

	struct vertex_t
	{
		vec3f	pos;
		vec4f	color;
		vec3f	normal;
		vec3f	bitangent;
		vec3f	tangent;
		vec2f	texCoord;
	};

	struct planeInfo_t
	{
		vec3f				origin;
		vec4f				color;
		uint32_t			subDivisionsX;
		uint32_t			subDivisionsY;
		vec2f				gridSize;
		winding_t			winding;
		vec3f				up;
		vec3f				normal;
		vec3f				side;
		vec2f				uvOffset;
		vec2f				uvDx;
		vec2f				uvDy;
		bool				flipUv;
		bool				centerAtOrigin;

		planeInfo_t()
		{
			gridSize = vec2f( 1.0f, 1.0f );
			subDivisionsX = 1;
			subDivisionsY = 1;
			uvOffset = vec2f( 0.0f, 0.0f );
			uvDx = vec2f( 1.0f, 0.0f );
			uvDy = vec2f( 0.0f, 1.0f );
			origin = vec3f( 0.0f, 0.0f, 0.0f );
			color = vec4f( 1.0f, 1.0f, 1.0f, 1.0f );
			up = vec3f( 0.0f, 0.0f, 1.0f );
			normal = vec3f( 1.0f, 0.0f, 0.0f );
			side = vec3f( 0.0f, 0.0f, 1.0f );
			winding = WINDING_COUNTER_CLOCKWISE;
			centerAtOrigin = true;
			flipUv = false;
		}
	};

	std::vector<vertex_t>	vb;
	std::vector<uint32_t>	ib;

	GeoBuilder()
	{
	
	}

	void AddPlaneSurf( const planeInfo_t& info )
	{
		const std::pair<size_t, size_t> sizeInVertices = std::pair<size_t, size_t>( info.subDivisionsX + 1, info.subDivisionsY + 1 );

		const size_t indicesPerQuad = 6;

		const size_t firstIndex = vb.size();
		size_t indicesCnt = ib.size();
		size_t vbIx = firstIndex;
		vb.resize( vbIx + sizeInVertices.first * sizeInVertices.second );
		ib.resize( indicesCnt + indicesPerQuad * info.subDivisionsX * info.subDivisionsY );

		const vec2f gridSizeWs = vec2f( info.gridSize[ 0 ], info.gridSize[ 1 ] );
		const vec2f cellSizeWs = vec2f( gridSizeWs[ 0 ] / info.subDivisionsX, gridSizeWs[ 1 ] / info.subDivisionsY );

		const vec3f up = info.up.Normalize();
		const vec3f normal = info.normal.Normalize();
		const vec3f side = info.side.Normalize();

		// Create vertices
		for ( size_t j = 0; j < sizeInVertices.second; ++j )
		{
			for ( size_t i = 0; i < sizeInVertices.first; ++i )
			{		
				float u = ( i / static_cast<float>( info.subDivisionsX ) );
				float v = ( j / static_cast<float>( info.subDivisionsY ) );

				if( info.flipUv )
				{
					float temp = u;
					u = v;
					v = temp;
				}

				vertex_t& vert = vb[ vbIx ];

				vert.tangent = side;
				vert.bitangent = up;
				vert.normal = normal;

				vert.color = info.color;
				vert.texCoord = info.uvOffset;
				vert.texCoord += u * info.uvDx + v * info.uvDy;

				vert.pos = i * cellSizeWs[ 0 ] * side + j * cellSizeWs[ 1 ] * up;
				if( info.centerAtOrigin )
				{
					vert.pos -= 0.5f * gridSizeWs[ 0 ] * side;
					vert.pos -= 0.5f * gridSizeWs[ 1 ] * up;
				}
				vert.pos += info.origin;

				++vbIx;
			}
		}

		// Create indices
		for ( uint32_t j = 0; j < info.subDivisionsY; ++j )
		{
			for ( uint32_t i = 0; i < info.subDivisionsX; ++i )
			{
				// Generate two triangles per quad
				uint32_t vIx[ 4 ];
				vIx[ 0 ] = static_cast<uint32_t>( firstIndex + ( i + 0 ) + ( j + 0 ) * sizeInVertices.second );
				vIx[ 1 ] = static_cast<uint32_t>( firstIndex + ( i + 1 ) + ( j + 0 ) * sizeInVertices.second );
				vIx[ 2 ] = static_cast<uint32_t>( firstIndex + ( i + 0 ) + ( j + 1 ) * sizeInVertices.second );
				vIx[ 3 ] = static_cast<uint32_t>( firstIndex + ( i + 1 ) + ( j + 1 ) * sizeInVertices.second );

				if( info.winding == WINDING_CLOCKWISE )
				{
					ib[ indicesCnt++ ] = vIx[ 0 ];
					ib[ indicesCnt++ ] = vIx[ 2 ];
					ib[ indicesCnt++ ] = vIx[ 1 ];

					ib[ indicesCnt++ ] = vIx[ 1 ];
					ib[ indicesCnt++ ] = vIx[ 2 ];
					ib[ indicesCnt++ ] = vIx[ 3 ];
				}
				else
				{
					ib[ indicesCnt++ ] = vIx[ 0 ];
					ib[ indicesCnt++ ] = vIx[ 1 ];
					ib[ indicesCnt++ ] = vIx[ 2 ];

					ib[ indicesCnt++ ] = vIx[ 2 ];
					ib[ indicesCnt++ ] = vIx[ 1 ];
					ib[ indicesCnt++ ] = vIx[ 3 ];
				}
			}
		}
	}

	void AddBoxSurf( const vec3f origin, const float size )
	{
		const size_t verticesCnt = 8;
		const size_t triPerSide = 2;
		const size_t sideCnt = 6;

		size_t vbIx = vb.size();
		vb.resize( vb.size() + verticesCnt );
		ib.resize( ib.size() + 3 * triPerSide * sideCnt );

		vec3f positions[ verticesCnt ] =
		{ vec3f( 0.0f,	0.0f,	0.0f ),
			vec3f( size,	0.0f,	0.0f ),
			vec3f( 0.0f,	size,	0.0f ),
			vec3f( size,	size,	0.0f ),
			vec3f( 0.0f,	0.0f,	size ),
			vec3f( size,	0.0f,	size ),
			vec3f( 0.0f,	size,	size ),
			vec3f( size,	size,	size ),
		};

		for ( size_t i = 0; i < verticesCnt; ++i )
		{
			vertex_t& vert = vb[ vbIx ];
			vert.pos = positions[ i ];
			vert.pos -= origin;
			vert.color = vec4f( 1.0f, 1.0f, 1.0f, 1.0f );
			vert.tangent = { 1.0f, 0.0f, 0.0f };
			vert.bitangent = { 0.0f, 1.0f, 0.0f };
			vert.texCoord = { 0.0f, 0.0f };

			++vbIx;
		}

		// TODO: need to offset by starting index
		uint32_t sideIndices[ sideCnt ][ 4 ] =
		{ { 2, 0, 1, 3 },
			{ 6, 4, 5, 7 },
			{ 0, 4, 5, 1 },
			{ 1, 5, 7, 3 },
			{ 3, 7, 6, 2 },
			{ 2, 6, 4, 0 },
		};

		uint32_t indicesCnt = 0;
		for ( size_t i = 0; i < sideCnt; ++i )
		{
			uint32_t vIx[ 4 ];
			vIx[ 0 ] = static_cast<uint32_t>( sideIndices[ i ][ 0 ] );
			vIx[ 1 ] = static_cast<uint32_t>( sideIndices[ i ][ 1 ] );
			vIx[ 2 ] = static_cast<uint32_t>( sideIndices[ i ][ 2 ] );
			vIx[ 3 ] = static_cast<uint32_t>( sideIndices[ i ][ 3 ] );

			ib[ indicesCnt++ ] = vIx[ 0 ];
			ib[ indicesCnt++ ] = vIx[ 2 ];
			ib[ indicesCnt++ ] = vIx[ 1 ];

			ib[ indicesCnt++ ] = vIx[ 2 ];
			ib[ indicesCnt++ ] = vIx[ 0 ];
			ib[ indicesCnt++ ] = vIx[ 3 ];
		}
	}

};
