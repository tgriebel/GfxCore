#pragma once

#include <utility>
#include "handle.h"
#include "mathVector.h"
#include "matrix.h"
#include "color.h"
#include "meshIO.h"
#include "aabb.h"
#include "ray.h"
#include "octree.h"
#include "common.h"
#include "material.h"

class ResourceManager;

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
			&&	( vertex0.color == vertex1.color ) );
}


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
	int32_t		materialId;

	Triangle( const vertex_t& _v0, const vertex_t& _v1, const vertex_t& _v2, const int32_t _materialId = 0 )
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

		materialId = _materialId;
	}
};

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


class ModelInstance
{
public:
	std::vector<Triangle>	triCache;
	Octree<uint32_t>		octree;
	uint32_t				modelIx;
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

	outT = 0.0;
	outBackface = true;

	if ( fabs( det ) < epsilon ) // Ray is parallel
	{
		return false;
	}

	if ( ( u < 0.0 ) || ( u > 1.0 ) )
	{
		return false;
	}

	if ( ( v < 0.0 ) || ( ( u + v ) > 1.0 ) )
	{
		return false;
	}

	const float t = Dot( e1, q ) * invDet;

	if ( r.Inside( t ) ) // Within ray parameterization
	{
		outT = t;
		outBackface = ( det < epsilon );
		return true;
	}
	else
	{
		return false;
	}
}


void LoadMaterialObj( const std::string& path, ResourceManager& rm, material_t& material );
uint32_t LoadModelObj( const std::string& path, ResourceManager& rm );
void StoreModelObj( const std::string& path, ResourceManager& rm, const uint32_t modelIx );
uint32_t LoadModelBin( const std::string& path, ResourceManager& rm );
void StoreModelBin( const std::string& path, ResourceManager& rm, const uint32_t modelIx );
void CreateModelInstance( ResourceManager& rm, const uint32_t modelIx, const mat4x4f& modelMatrix, const bool smoothNormals, const Color& tint, ModelInstance* outInstance, const int32_t materialId = -1 );
uint32_t CreatePlaneModel( ResourceManager& rm, const vec2f& size, const vec2i& cellCnt, const int32_t materialId );