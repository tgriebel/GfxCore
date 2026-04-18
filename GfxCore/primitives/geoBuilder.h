#pragma once

#include "../core/common.h"

#include <cmath>
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
				vIx[ 0 ] = static_cast<uint32_t>( firstIndex + ( i + 0 ) + ( j + 0 ) * sizeInVertices.first );
				vIx[ 1 ] = static_cast<uint32_t>( firstIndex + ( i + 1 ) + ( j + 0 ) * sizeInVertices.first );
				vIx[ 2 ] = static_cast<uint32_t>( firstIndex + ( i + 0 ) + ( j + 1 ) * sizeInVertices.first );
				vIx[ 3 ] = static_cast<uint32_t>( firstIndex + ( i + 1 ) + ( j + 1 ) * sizeInVertices.first );

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

	struct sphereInfo_t
	{
		vec3f		origin;
		vec4f		color;
		float		radius;
		uint32_t	latitudeDivisions;		// rings pole-to-pole
		uint32_t	longitudeDivisions;		// sectors around the equator
		winding_t	winding;

		sphereInfo_t()
		{
			origin				= vec3f( 0.0f, 0.0f, 0.0f );
			color				= vec4f( 1.0f, 1.0f, 1.0f, 1.0f );
			radius				= 1.0f;
			latitudeDivisions	= 16;
			longitudeDivisions	= 32;
			winding				= WINDING_COUNTER_CLOCKWISE;
		}
	};

	// "UV Sphere" Generation Algorithm
	void AddSphereSurf( const sphereInfo_t& info )
	{
		const uint32_t ringCount	= info.latitudeDivisions  + 1;
		const uint32_t sectorCount	= info.longitudeDivisions + 1;

		const size_t firstIndex = vb.size();
		size_t indicesCnt = ib.size();

		vb.resize( firstIndex + ringCount   * sectorCount );
		ib.resize( indicesCnt + 6 * info.latitudeDivisions * info.longitudeDivisions );

		size_t vbIx = firstIndex;

		for ( uint32_t j = 0; j < ringCount; ++j )
		{
			const float theta    = PI * ( j / static_cast<float>( info.latitudeDivisions ) );
			const float sinTheta = std::sin( theta );
			const float cosTheta = std::cos( theta );

			for ( uint32_t i = 0; i < sectorCount; ++i )
			{
				const float phi    = 2.0f * PI * ( i / static_cast<float>( info.longitudeDivisions ) );
				const float sinPhi = std::sin( phi );
				const float cosPhi = std::cos( phi );

				vertex_t& vert = vb[ vbIx ];

				vert.normal   = vec3f( sinTheta * cosPhi, sinTheta * sinPhi, cosTheta );
				vert.pos      = info.origin + info.radius * vert.normal;
				vert.color    = info.color;
				vert.tangent   = vec3f( -sinPhi, cosPhi, 0.0f );
				vert.bitangent = vec3f( cosTheta * cosPhi, cosTheta * sinPhi, -sinTheta );

				vert.texCoord  = vec2f(
					i / static_cast<float>( info.longitudeDivisions ),
					j / static_cast<float>( info.latitudeDivisions )
				);

				++vbIx;
			}
		}

		for ( uint32_t j = 0; j < info.latitudeDivisions; ++j )
		{
			for ( uint32_t i = 0; i < info.longitudeDivisions; ++i )
			{
				const uint32_t vIx0 = static_cast<uint32_t>( firstIndex + ( j + 0 ) * sectorCount + ( i + 0 ) );
				const uint32_t vIx1 = static_cast<uint32_t>( firstIndex + ( j + 0 ) * sectorCount + ( i + 1 ) );
				const uint32_t vIx2 = static_cast<uint32_t>( firstIndex + ( j + 1 ) * sectorCount + ( i + 0 ) );
				const uint32_t vIx3 = static_cast<uint32_t>( firstIndex + ( j + 1 ) * sectorCount + ( i + 1 ) );

				// NOTE: the sphere grid axes are T_phi (longitude) and T_theta
				// (latitude, increasing toward south pole). T_phi x T_theta = -N,
				// so the grid is left-handed w.r.t. the outward normal. The index
				// cases below are therefore swapped relative to AddPlaneSurf so
				// that WINDING_COUNTER_CLOCKWISE consistently means outward-facing.
				if ( info.winding == WINDING_COUNTER_CLOCKWISE )
				{
					ib[ indicesCnt++ ] = vIx0;
					ib[ indicesCnt++ ] = vIx2;
					ib[ indicesCnt++ ] = vIx1;

					ib[ indicesCnt++ ] = vIx1;
					ib[ indicesCnt++ ] = vIx2;
					ib[ indicesCnt++ ] = vIx3;
				}
				else
				{
					ib[ indicesCnt++ ] = vIx0;
					ib[ indicesCnt++ ] = vIx1;
					ib[ indicesCnt++ ] = vIx2;

					ib[ indicesCnt++ ] = vIx2;
					ib[ indicesCnt++ ] = vIx1;
					ib[ indicesCnt++ ] = vIx3;
				}
			}
		}
	}


	void AddBoxSurf( const vec3f origin, const float size )
	{
		struct boxFaceFrame_t
		{
			vec3f	normal;
			vec3f	side;
			vec3f	up;
		};

		const boxFaceFrame_t faces[ 6 ] =
		{
			{ vec3f( 1.0f,  0.0f,  0.0f ), vec3f( 0.0f, 1.0f, 0.0f ), vec3f( 0.0f, 0.0f, 1.0f ) }, // +X
			{ vec3f( -1.0f,  0.0f,  0.0f ), vec3f( 0.0f, 0.0f, 1.0f ), vec3f( 0.0f, 1.0f, 0.0f ) }, // -X
			{ vec3f( 0.0f,  1.0f,  0.0f ), vec3f( 0.0f, 0.0f, 1.0f ), vec3f( 1.0f, 0.0f, 0.0f ) }, // +Y
			{ vec3f( 0.0f, -1.0f,  0.0f ), vec3f( 1.0f, 0.0f, 0.0f ), vec3f( 0.0f, 0.0f, 1.0f ) }, // -Y
			{ vec3f( 0.0f,  0.0f,  1.0f ), vec3f( 1.0f, 0.0f, 0.0f ), vec3f( 0.0f, 1.0f, 0.0f ) }, // +Z
			{ vec3f( 0.0f,  0.0f, -1.0f ), vec3f( 0.0f, 1.0f, 0.0f ), vec3f( 1.0f, 0.0f, 0.0f ) }, // -Z
		};

		const float halfSize = 0.5f * size;

		for( size_t i = 0; i < 6; ++i )
		{
			planeInfo_t info;
			info.origin = origin + halfSize * faces[ i ].normal;
			info.normal = faces[ i ].normal;
			info.side = faces[ i ].side;
			info.up = faces[ i ].up;
			info.gridSize = vec2f( size, size );

			AddPlaneSurf( info );
		}
	}
};
