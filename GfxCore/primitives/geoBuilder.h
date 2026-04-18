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

	// -----------------------------------------------------------------------
	// Plane
	// -----------------------------------------------------------------------

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

	// -----------------------------------------------------------------------
	// Torus
	// -----------------------------------------------------------------------

	struct torusInfo_t
	{
		vec3f		origin;
		vec4f		color;
		float		innerRadius;
		float		outerRadius;
		uint32_t	ringDivisions;		// segments around the hole (azimuth)
		uint32_t	crossDivisions;		// segments around the tube (cross-section)
		winding_t	winding;

		torusInfo_t()
		{
			origin			= vec3f( 0.0f, 0.0f, 0.0f );
			color			= vec4f( 1.0f, 1.0f, 1.0f, 1.0f );
			innerRadius		= 0.5f;
			outerRadius		= 1.0f;
			ringDivisions	= 32;
			crossDivisions	= 16;
			winding			= WINDING_COUNTER_CLOCKWISE;
		}
	};

	// "UV torus" 
	void AddTorusSurf( const torusInfo_t& info )
	{
		static constexpr float Pi = 3.14159265358979f;

		const float R = 0.5f * ( info.outerRadius + info.innerRadius );
		const float r = 0.5f * ( info.outerRadius - info.innerRadius );

		const uint32_t ringCount  = info.ringDivisions  + 1;
		const uint32_t crossCount = info.crossDivisions + 1;

		const size_t firstIndex = vb.size();
		size_t indicesCnt = ib.size();

		vb.resize( firstIndex + ringCount * crossCount );
		ib.resize( indicesCnt + 6 * info.ringDivisions * info.crossDivisions );

		// Vertices
		{
			size_t vbIx = firstIndex;

			for ( uint32_t j = 0; j < ringCount; ++j )
			{
				const float phi    = 2.0f * Pi * ( j / static_cast<float>( info.ringDivisions ) );
				const float cosPhi = std::cos( phi );
				const float sinPhi = std::sin( phi );

				const vec3f radial = vec3f( cosPhi, sinPhi, 0.0f );

				for ( uint32_t i = 0; i < crossCount; ++i )
				{
					const float theta    = 2.0f * Pi * ( i / static_cast<float>( info.crossDivisions ) );
					const float cosTheta = std::cos( theta );
					const float sinTheta = std::sin( theta );

					vertex_t& vert = vb[ vbIx ];
					vert.color  = info.color;

					vert.normal = vec3f( cosTheta * cosPhi, cosTheta * sinPhi, sinTheta );
					vert.pos    = info.origin + R * radial + r * vert.normal;

					++vbIx;
				}
			}
		}

		// Indices
		{
			for ( uint32_t j = 0; j < ringCount; ++j )
			{
				const float phi    = 2.0f * Pi * ( j / static_cast<float>( info.ringDivisions ) );
				const float cosPhi = std::cos( phi );
				const float sinPhi = std::sin( phi );

				for ( uint32_t i = 0; i < crossCount; ++i )
				{
					const float theta    = 2.0f * Pi * ( i / static_cast<float>( info.crossDivisions ) );
					const float cosTheta = std::cos( theta );
					const float sinTheta = std::sin( theta );

					vertex_t& vert = vb[ firstIndex + j * crossCount + i ];

					vert.tangent   = vec3f( -sinPhi,              cosPhi,             0.0f     );
					vert.bitangent = vec3f( -sinTheta * cosPhi,  -sinTheta * sinPhi,  cosTheta );
					vert.texCoord  = vec2f(
						j / static_cast<float>( info.ringDivisions  ),
						i / static_cast<float>( info.crossDivisions )
					);

					if ( j >= info.ringDivisions || i >= info.crossDivisions ) {
						continue;
					}

					const uint32_t vIx0 = static_cast<uint32_t>( firstIndex + ( j + 0 ) * crossCount + ( i + 0 ) );
					const uint32_t vIx1 = static_cast<uint32_t>( firstIndex + ( j + 0 ) * crossCount + ( i + 1 ) );
					const uint32_t vIx2 = static_cast<uint32_t>( firstIndex + ( j + 1 ) * crossCount + ( i + 0 ) );
					const uint32_t vIx3 = static_cast<uint32_t>( firstIndex + ( j + 1 ) * crossCount + ( i + 1 ) );

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
	}


	// -----------------------------------------------------------------------
	// Box
	// -----------------------------------------------------------------------

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


	// -----------------------------------------------------------------------
	// Capsule
	// -----------------------------------------------------------------------

	struct capsuleInfo_t
	{
		vec3f		origin;
		vec4f		color;
		float		radius;				// hemisphere radius
		float		cylinderHeight;		// length of cylindrical body (excluding caps)
		uint32_t	ringDivisions;		// azimuth segments
		uint32_t	capDivisions;		// latitude rings per hemisphere (>= 1)
		uint32_t	stackDivisions;		// interior rings in cylindrical body
		winding_t	winding;

		capsuleInfo_t()
		{
			origin			= vec3f( 0.0f, 0.0f, 0.0f );
			color			= vec4f( 1.0f, 1.0f, 1.0f, 1.0f );
			radius			= 0.5f;
			cylinderHeight	= 1.0f;
			ringDivisions	= 16;
			capDivisions	= 8;
			stackDivisions	= 0;
			winding			= WINDING_COUNTER_CLOCKWISE;
		}
	};

	// "UV capsule" generateion
	void AddCapsuleSurf( const capsuleInfo_t& info )
	{
		const uint32_t ringCount     = info.ringDivisions + 1;
		const uint32_t totalVtxRows  = 2 * info.capDivisions + info.stackDivisions + 2;
		const uint32_t totalQuadRows = totalVtxRows - 1;

		const size_t firstVtx = vb.size();
		size_t       ibIdx    = ib.size();

		vb.resize( firstVtx + totalVtxRows * ringCount );
		ib.resize( ibIdx    + 6 * totalQuadRows * info.ringDivisions );

		const float r = info.radius;
		const float H = info.cylinderHeight;

		// Vertices
		for ( uint32_t row = 0; row < totalVtxRows; ++row )
		{
			float    cosTheta, sinTheta;
			float    zCenter;
			bool     isCylinder = false;

			if ( row <= info.capDivisions )
			{
				const float theta = ( PI / 2.0f ) * row / info.capDivisions;
				cosTheta = std::cos( theta );
				sinTheta = std::sin( theta );
				zCenter  = H * 0.5f;
			}
			else if ( row <= info.capDivisions + info.stackDivisions )
			{
				isCylinder            = true;
				const uint32_t j      = row - info.capDivisions;
				zCenter               = H * 0.5f - static_cast<float>( j ) * H / ( info.stackDivisions + 1 );
				cosTheta              = 0.0f;
				sinTheta              = 1.0f;
			}
			else
			{
				const uint32_t j = row - info.capDivisions - info.stackDivisions - 1;
				const float theta = ( PI / 2.0f ) + ( PI / 2.0f ) * j / info.capDivisions;
				cosTheta = std::cos( theta );
				sinTheta = std::sin( theta );
				zCenter  = -H * 0.5f;
			}

			for ( uint32_t c = 0; c < ringCount; ++c )
			{
				const float phi    = 2.0f * PI * c / info.ringDivisions;
				const float cosPhi = std::cos( phi );
				const float sinPhi = std::sin( phi );

				vertex_t& vert = vb[ firstVtx + row * ringCount + c ];
				vert.color     = info.color;
				vert.texCoord  = vec2f(
					static_cast<float>( c )   / info.ringDivisions,
					static_cast<float>( row ) / ( totalVtxRows - 1 )
				);
				vert.tangent = vec3f( -sinPhi, cosPhi, 0.0f );

				if ( isCylinder )
				{
					vert.normal    = vec3f( cosPhi, sinPhi, 0.0f );
					vert.bitangent = vec3f( 0.0f, 0.0f, -1.0f );
					vert.pos       = info.origin + vec3f( r * cosPhi, r * sinPhi, zCenter );
				}
				else
				{
					vert.normal    = vec3f( sinTheta * cosPhi, sinTheta * sinPhi, cosTheta );
					vert.bitangent = vec3f( cosTheta * cosPhi, cosTheta * sinPhi, -sinTheta );
					vert.pos       = info.origin + r * vert.normal + vec3f( 0.0f, 0.0f, zCenter );
				}
			}
		}

		// Indices
		for ( uint32_t row = 0; row < totalQuadRows; ++row )
		{
			for ( uint32_t c = 0; c < info.ringDivisions; ++c )
			{
				const uint32_t vIx0 = static_cast<uint32_t>( firstVtx + ( row + 0 ) * ringCount + ( c + 0 ) );
				const uint32_t vIx1 = static_cast<uint32_t>( firstVtx + ( row + 0 ) * ringCount + ( c + 1 ) );
				const uint32_t vIx2 = static_cast<uint32_t>( firstVtx + ( row + 1 ) * ringCount + ( c + 0 ) );
				const uint32_t vIx3 = static_cast<uint32_t>( firstVtx + ( row + 1 ) * ringCount + ( c + 1 ) );

				if ( info.winding == WINDING_COUNTER_CLOCKWISE )
				{
					ib[ ibIdx++ ] = vIx0; ib[ ibIdx++ ] = vIx2; ib[ ibIdx++ ] = vIx1;
					ib[ ibIdx++ ] = vIx1; ib[ ibIdx++ ] = vIx2; ib[ ibIdx++ ] = vIx3;
				}
				else
				{
					ib[ ibIdx++ ] = vIx0; ib[ ibIdx++ ] = vIx1; ib[ ibIdx++ ] = vIx2;
					ib[ ibIdx++ ] = vIx2; ib[ ibIdx++ ] = vIx1; ib[ ibIdx++ ] = vIx3;
				}
			}
		}
	}


	// -----------------------------------------------------------------------
	// Pyramid
	// -----------------------------------------------------------------------

	struct pyramidInfo_t
	{
		vec3f		origin;
		vec4f		color;
		float		baseRadius;		// circumradius of base polygon
		float		height;			// apex height above base
		uint32_t	sides;
		winding_t	winding;

		pyramidInfo_t()
		{
			origin		= vec3f( 0.0f, 0.0f, 0.0f );
			color		= vec4f( 1.0f, 1.0f, 1.0f, 1.0f );
			baseRadius	= 1.0f;
			height		= 2.0f;
			sides		= 4;
			winding		= WINDING_COUNTER_CLOCKWISE;
		}
	};


	void AddPyramidSurf( const pyramidInfo_t& info )
	{
		// sides side triangles + sides base triangles, 3 verts each, no sharing
		const uint32_t triCount = 2 * info.sides;

		const size_t firstVtx = vb.size();
		size_t       ibIdx    = ib.size();

		vb.resize( firstVtx + 3 * triCount );
		ib.resize( ibIdx    + 3 * triCount );

		uint32_t vtxCursor = static_cast<uint32_t>( firstVtx );

		auto emitTri = [&]( const vec3f& p0, const vec3f& p1, const vec3f& p2,
		                    const vec2f& uv0, const vec2f& uv1, const vec2f& uv2 )
		{
			const bool   sw = ( info.winding == WINDING_CLOCKWISE );
			const vec3f  a  = p0,        b  = sw ? p2 : p1,  c  = sw ? p1 : p2;
			const vec2f  ta = uv0,       tb = sw ? uv2 : uv1, tc = sw ? uv1 : uv2;

			const vec3f N  = Cross( b - a, c - a ).Normalize();
			const vec3f T  = ( b - a ).Normalize();
			const vec3f Bt = Cross( T, N );		// T x Bt = -N, matches sphere convention

			vertex_t& v0 = vb[ vtxCursor ];
			v0.pos = a; v0.color = info.color; v0.normal = N; v0.tangent = T; v0.bitangent = Bt; v0.texCoord = ta;
			ib[ ibIdx++ ] = vtxCursor++;

			vertex_t& v1 = vb[ vtxCursor ];
			v1.pos = b; v1.color = info.color; v1.normal = N; v1.tangent = T; v1.bitangent = Bt; v1.texCoord = tb;
			ib[ ibIdx++ ] = vtxCursor++;

			vertex_t& v2 = vb[ vtxCursor ];
			v2.pos = c; v2.color = info.color; v2.normal = N; v2.tangent = T; v2.bitangent = Bt; v2.texCoord = tc;
			ib[ ibIdx++ ] = vtxCursor++;
		};

		const vec3f apex = info.origin + vec3f( 0.0f, 0.0f, info.height );

		// Side faces
		for ( uint32_t i = 0; i < info.sides; ++i )
		{
			const float phi0 = 2.0f * PI * i / info.sides;
			const float phi1 = 2.0f * PI * ( i + 1 ) / info.sides;

			const vec3f A = info.origin + vec3f( info.baseRadius * std::cos( phi0 ), info.baseRadius * std::sin( phi0 ), 0.0f );
			const vec3f B = info.origin + vec3f( info.baseRadius * std::cos( phi1 ), info.baseRadius * std::sin( phi1 ), 0.0f );

			emitTri( A, B, apex,
			         vec2f( 0.0f, 0.0f ), vec2f( 1.0f, 0.0f ), vec2f( 0.5f, 1.0f ) );
		}

		// Base fan: center → B_i → A_i gives N = -Z (outward downward)
		const vec3f baseCenter = info.origin;
		for ( uint32_t i = 0; i < info.sides; ++i )
		{
			const float phi0 = 2.0f * PI * i / info.sides;
			const float phi1 = 2.0f * PI * ( i + 1 ) / info.sides;

			const vec3f A = info.origin + vec3f( info.baseRadius * std::cos( phi0 ), info.baseRadius * std::sin( phi0 ), 0.0f );
			const vec3f B = info.origin + vec3f( info.baseRadius * std::cos( phi1 ), info.baseRadius * std::sin( phi1 ), 0.0f );

			const vec2f uvCenter = vec2f( 0.5f, 0.5f );
			const vec2f uvA      = vec2f( 0.5f + 0.5f * std::cos( phi0 ), 0.5f + 0.5f * std::sin( phi0 ) );
			const vec2f uvB      = vec2f( 0.5f + 0.5f * std::cos( phi1 ), 0.5f + 0.5f * std::sin( phi1 ) );

			emitTri( baseCenter, B, A, uvCenter, uvB, uvA );
		}
	}


	// -----------------------------------------------------------------------
	// Diamond
	// -----------------------------------------------------------------------

	struct diamondInfo_t
	{
		vec3f		origin;
		vec4f		color;
		float		radius;			// equatorial radius
		float		topHeight;		// distance from equator to top apex
		float		bottomHeight;	// distance from equator to bottom apex
		uint32_t	sides;
		winding_t	winding;

		diamondInfo_t()
		{
			origin			= vec3f( 0.0f, 0.0f, 0.0f );
			color			= vec4f( 1.0f, 1.0f, 1.0f, 1.0f );
			radius			= 1.0f;
			topHeight		= 1.0f;
			bottomHeight	= 0.5f;
			sides			= 6;
			winding			= WINDING_COUNTER_CLOCKWISE;
		}
	};


	void AddDiamondSurf( const diamondInfo_t& info )
	{
		const uint32_t triCount = 2 * info.sides;

		const size_t firstVtx = vb.size();
		size_t       ibIdx    = ib.size();

		vb.resize( firstVtx + 3 * triCount );
		ib.resize( ibIdx    + 3 * triCount );

		uint32_t vtxCursor = static_cast<uint32_t>( firstVtx );

		auto emitTri = [&]( const vec3f& p0, const vec3f& p1, const vec3f& p2,
		                    const vec2f& uv0, const vec2f& uv1, const vec2f& uv2 )
		{
			const bool   sw = ( info.winding == WINDING_CLOCKWISE );
			const vec3f  a  = p0,        b  = sw ? p2 : p1,  c  = sw ? p1 : p2;
			const vec2f  ta = uv0,       tb = sw ? uv2 : uv1, tc = sw ? uv1 : uv2;

			const vec3f N  = Cross( b - a, c - a ).Normalize();
			const vec3f T  = ( b - a ).Normalize();
			const vec3f Bt = Cross( T, N );

			vertex_t& v0 = vb[ vtxCursor ];
			v0.pos = a; v0.color = info.color; v0.normal = N; v0.tangent = T; v0.bitangent = Bt; v0.texCoord = ta;
			ib[ ibIdx++ ] = vtxCursor++;

			vertex_t& v1 = vb[ vtxCursor ];
			v1.pos = b; v1.color = info.color; v1.normal = N; v1.tangent = T; v1.bitangent = Bt; v1.texCoord = tb;
			ib[ ibIdx++ ] = vtxCursor++;

			vertex_t& v2 = vb[ vtxCursor ];
			v2.pos = c; v2.color = info.color; v2.normal = N; v2.tangent = T; v2.bitangent = Bt; v2.texCoord = tc;
			ib[ ibIdx++ ] = vtxCursor++;
		};

		const vec3f apexTop = info.origin + vec3f( 0.0f, 0.0f,  info.topHeight );
		const vec3f apexBot = info.origin + vec3f( 0.0f, 0.0f, -info.bottomHeight );

		for ( uint32_t i = 0; i < info.sides; ++i )
		{
			const float phi0 = 2.0f * PI * i / info.sides;
			const float phi1 = 2.0f * PI * ( i + 1 ) / info.sides;

			const vec3f A = info.origin + vec3f( info.radius * std::cos( phi0 ), info.radius * std::sin( phi0 ), 0.0f );
			const vec3f B = info.origin + vec3f( info.radius * std::cos( phi1 ), info.radius * std::sin( phi1 ), 0.0f );

			emitTri( A, B, apexTop, vec2f( 0.0f, 0.0f ), vec2f( 1.0f, 0.0f ), vec2f( 0.5f, 1.0f ) );
			emitTri( B, A, apexBot, vec2f( 0.0f, 0.0f ), vec2f( 1.0f, 0.0f ), vec2f( 0.5f, 1.0f ) );
		}
	}
};
