#pragma once

#include "../core/common.h"
#include "../math/vector.h"

#include <cmath>
#include <vector>

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
		vec3f		origin;
		vec4f		color;
		uint32_t	subDivisionsX;
		uint32_t	subDivisionsY;
		vec2f		gridSize;
		winding_t	winding;
		vec3f		up;
		vec3f		normal;
		vec3f		side;
		vec2f		uvOffset;
		vec2f		uvDx;
		vec2f		uvDy;
		bool		flipUv;
		bool		centerAtOrigin;

		planeInfo_t()
		{
			gridSize		= vec2f( 1.0f, 1.0f );
			subDivisionsX	= 1;
			subDivisionsY	= 1;
			uvOffset		= vec2f( 0.0f, 0.0f );
			uvDx			= vec2f( 1.0f, 0.0f );
			uvDy			= vec2f( 0.0f, 1.0f );
			origin			= vec3f( 0.0f, 0.0f, 0.0f );
			color			= vec4f( 1.0f, 1.0f, 1.0f, 1.0f );
			up				= vec3f( 0.0f, 0.0f, 1.0f );
			normal			= vec3f( 1.0f, 0.0f, 0.0f );
			side			= vec3f( 0.0f, 0.0f, 1.0f );
			winding			= WINDING_COUNTER_CLOCKWISE;
			centerAtOrigin	= true;
			flipUv			= false;
		}
	};

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

	std::vector<vertex_t>	vb;
	std::vector<uint32_t>	ib;

	GeoBuilder() {}

	void AddPlaneSurf( const planeInfo_t& info );
	void AddSphereSurf( const sphereInfo_t& info );
	void AddTorusSurf( const torusInfo_t& info );
	void AddBoxSurf( const vec3f origin, const float size );
	void AddCapsuleSurf( const capsuleInfo_t& info );
	void AddPyramidSurf( const pyramidInfo_t& info );
	void AddDiamondSurf( const diamondInfo_t& info );

private:
	void EmitFlatTri( const vec3f& p0, const vec3f& p1, const vec3f& p2,
	                  const vec2f& uv0, const vec2f& uv1, const vec2f& uv2,
	                  const vec4f& color, const winding_t winding,
	                  uint32_t& vtxCursor, size_t& ibIdx );
};
