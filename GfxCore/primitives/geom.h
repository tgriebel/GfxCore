/*
* MIT License
*
* Copyright( c ) 2020-2023 Thomas Griebel
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

#include <utility>
#include "../core/handle.h"
#include "../math/vector.h"
#include "../math/matrix.h"
#include "../image/color.h"
#include "../io/meshIO.h"
#include "../acceleration/aabb.h"
#include "../primitives/ray.h"
#include "../acceleration/octree.h"
#include "../core/common.h"
#include "../core/handle.h"
#include "../core/util.h"
#include "../asset_types/material.h"

class Entity;
class RtModel;
class AssetManager;
class ResourceManager;
using matHdl_t = int32_t;

struct plane_t
{
	vec4f halfWidth;
	vec4f halfHeight;
	vec4f origin;
};


struct vertex_t
{
	vec4f	pos;
	vec3f	normal;
	vec3f	tangent;
	vec3f	bitangent;
	vec2f	uv;
	vec2f	uv2;
	Color	color;
};


inline bool operator==( const vertex_t& vertex0, const vertex_t& vertex1 )
{
	return (	( vertex0.pos == vertex1.pos )
			&& 	( vertex0.normal == vertex1.normal )
			&& 	( vertex0.tangent == vertex1.tangent )
			&& 	( vertex0.bitangent == vertex1.bitangent )
			&&	( vertex0.uv == vertex1.uv )
			&&	( vertex0.uv2 == vertex1.uv2 )
			&&	( vertex0.color == vertex1.color ) );
}


template<> struct std::hash<vertex_t>
{
	size_t operator()( vertex_t const& vertex ) const
	{
		return Hash( reinterpret_cast<const uint8_t*>( &vertex ), sizeof( vertex_t ) );
	}
};


struct edge_t
{
	vec4f	pt0;
	vec4f	pt1;
	float	length;
	AABB	aabb;
};


struct box_t
{
	vec4f	pts[ 8 ];
	float	width;
	float	height;
	float	depth;
	AABB	aabb;
};

using imageHandle_t = uint32_t;

enum frontFace_t
{
	CLOCKWISE = 0,
	COUNTER_CLOCKWISE = 1,
};

struct Triangle
{
	vertex_t	v0;
	vertex_t	v1;
	vertex_t	v2;

	vec3f		e0;
	vec3f		e1;
	vec3f		e2;

	vec3f		n;
	vec3f		t;
	vec3f		b;
	AABB		aabb;
	hdl_t		materialId;

	frontFace_t	frontFace;

	Triangle( const vertex_t& _v0, const vertex_t& _v1, const vertex_t& _v2, frontFace_t _frontFace = CLOCKWISE, const hdl_t _materialId = 0 )
	{
		v0 = _v0;
		v1 = _v1;
		v2 = _v2;

		vec3f pt0 = Trunc<4, 1>( v0.pos );
		vec3f pt1 = Trunc<4, 1>( v1.pos );
		vec3f pt2 = Trunc<4, 1>( v2.pos );

		e0 = ( pt1 - pt0 );
		e1 = ( pt2 - pt1 );
		e2 = ( pt2 - pt0 );

		n = Cross( e0, e1 ).Normalize();

		t = e0.Normalize();
		b = Cross( n, t );

		aabb.Expand( pt0 );
		aabb.Expand( pt1 );
		aabb.Expand( pt2 );

		// FIXME: think about this
		if ( _frontFace == COUNTER_CLOCKWISE ) {
			n = n.Reverse();
			v0.normal = v0.normal.Reverse();
			v1.normal = v1.normal.Reverse();
			v2.normal = v2.normal.Reverse();
		}

		frontFace = _frontFace;
		materialId = _materialId;
	}
};

// TODO: deprecated
struct surface_t
{
	uint32_t		vb;
	uint32_t		ib;
	uint32_t		vbOffset;
	uint32_t		ibOffset;
	uint32_t		vbEnd;
	uint32_t		ibEnd;
	int32_t			materialId;
};

class ModelSource
{
public:
	static const uint32_t MaxMaterials = 3;

	std::string				name;
	std::vector<surface_t>	surfs;
};


class RtModel
{
public:
	std::vector<Triangle>	triCache;
	Octree<uint32_t>		octree;
	mat4x4f					transform;
	vec3f					centroid;

	void BuildAS()
	{
		AABB aabb;

		const size_t triCnt = triCache.size();
		for ( size_t i = 0; i < triCnt; ++i )
		{
			aabb.Expand( triCache[ i ].aabb.min );
			aabb.Expand( triCache[ i ].aabb.max );
		}

		octree = Octree<uint32_t>( aabb.min, aabb.max );

		for( uint32_t i = 0; i < triCnt; ++ i )
		{
			// Build the octree using triangle indices
			octree.Insert( triCache[ i ].aabb, i );
		}
	}
};


/*
===================================
PointToBarycentric
===================================
*/
inline vec3f PointToBarycentric( const vec3f& pt, const vec3f& v0, const vec3f& v1, const vec3f& v2 )
{
	vec3f e1 = v2 - v1;
	vec3f e2 = v0 - v2;
	vec3f e3 = v1 - v0;

	vec3f d1 = pt - v0;
	vec3f d2 = pt - v1;
	vec3f d3 = pt - v2;

	vec3f n = Cross( e1, e2 ) / ( Cross( e1, e2 ).Length() );

	const float area = Dot( Cross( e1, e2 ), n );

	vec3f baryPoint;
	baryPoint[ 0 ] = Dot( Cross( e1, d3 ), n ) / area;
	baryPoint[ 1 ] = Dot( Cross( e2, d1 ), n ) / area;
	baryPoint[ 2 ] = Dot( Cross( e3, d2 ), n ) / area;

	return baryPoint;
}


/*
===================================
RayToTriangleIntersection
- Möller–Trumbore ray-triangle intersection algorithm
===================================
*/
inline bool RayToTriangleIntersection( const Ray& r, const Triangle& tri, bool& outBackface, float& outT )
{
	const float		epsilon	= 1e-7f;
	const vec3f		e0		= tri.e0;
	const vec3f		e1		= tri.e2;
	const vec3f		p		= Cross( r.d, e1 );
	const vec3f		h		= ( r.GetOrigin() - Trunc<4, 1>( tri.v0.pos ) );
	const vec3f		q		= Cross( h, e0 );
	const float		det		= Dot( e0, p );
	const float		invDet	= ( 1.0f / det );
	const float		u		= invDet * Dot( h, p );
	const float		v		= invDet * Dot( r.d, q );

	outT = FLT_MAX;
	outBackface = true;

	if ( fabs( det ) < epsilon ) // Ray is parallel
	{
		return false;
	}

	if ( ( u < 0.0f ) || ( u > 1.0f ) )
	{
		return false;
	}

	if ( ( v < 0.0f ) || ( ( u + v ) > 1.0f ) )
	{
		return false;
	}

	const float t = Dot( e1, q ) * invDet;

	if ( r.Inside( t ) ) // Within ray parameterization
	{
		outT = t;
		outBackface = ( tri.frontFace == CLOCKWISE ) ? ( det < 0.0f ) : ( det >= 0.0f );
		return true;
	}
	else
	{
		return false;
	}
}

void CreateRayTraceModel( ResourceManager& rm, const uint32_t modelIx, const mat4x4f& modelMatrix, const bool smoothNormals, const Color& tint, RtModel* outInstance, const matHdl_t materialId = -1 );
void CreateRayTraceModel( AssetManager& assets, Entity* ent, RtModel* outInstance, const hdl_t overrideMaterial = INVALID_HDL );
uint32_t CreatePlaneModel( ResourceManager& rm, const vec2f& size, const vec2i& cellCnt, const matHdl_t materialId = -1 );