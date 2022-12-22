#pragma once

#include <utility>
#include "../GfxCore/mathVector.h"
#include "../GfxCore/matrix.h"
#include "ray.h"

class AABB
{
public:
	vec3f min;
	vec3f max;

	AABB()
	{
		min[ 0 ] = FLT_MAX;
		min[ 1 ] = FLT_MAX;
		min[ 2 ] = FLT_MAX;

		max[ 0 ] = -FLT_MAX;
		max[ 1 ] = -FLT_MAX;
		max[ 2 ] = -FLT_MAX;
	}

	AABB( const AABB& aabb )
	{
		min = aabb.min;
		max = aabb.max;
	}

	AABB& operator=( const AABB& aabb ) {
		min = aabb.min;
		max = aabb.max;
		return *this;
	}

	AABB( const vec3f& point )
	{
		min = point;
		max = point;
	}

	AABB( const vec3f& _min, const vec3f& _max ) : AABB()
	{
		Expand( _min );
		Expand( _max );
	}

	// Adapts Christer Ericson's Kay-Kajiya slab based interection test from Real Time Collision Detection
	bool Intersect( const Ray& r, float& tMin, float& tMax ) const
	{
		tMin = -FLT_MAX;
		tMax = FLT_MAX;

		if ( ( min[ 0 ] == -FLT_MAX ) || ( min[ 1 ] == -FLT_MAX ) || ( min[ 2 ] == -FLT_MAX ) ) {
			return false;
		}

		if ( ( max[ 0 ] == FLT_MAX ) || ( max[ 1 ] == FLT_MAX ) || ( max[ 2 ] == FLT_MAX ) ) {
			return false;
		}

		for ( uint32_t i = 0; i < 3; ++i )
		{
			if ( abs( r.d[ i ] ) < 0.000001f ) // Parallel to slab...
			{
				if ( ( r.o[ i ] < min[ i ] ) || ( r.o[ i ] > max[ i ] ) ) { // ...and outside box
					return false;
				}
			}
			else
			{
				const float ood = 1.0f / r.d[ i ];
				float t1 = ( min[ i ] - r.o[ i ] ) * ood;
				float t2 = ( max[ i ] - r.o[ i ] ) * ood;

				if ( t1 > t2 ) {
					std::swap( t1, t2 );
				}
				if ( t1 > tMin ) {
					tMin = t1;
				}
				if ( t2 < tMax ) {
					tMax = t2;
				}
				if ( ( tMin > tMax ) || ( t2 < 0 ) ) {
					return false;
				}
			}
		}
		return true;
	}

	bool Inside( const vec3f& pt )
	{
		bool isInside = true;
		isInside = isInside && ( pt[ 0 ] >= min[ 0 ] && pt[ 0 ] <= max[ 0 ] );
		isInside = isInside && ( pt[ 1 ] >= min[ 1 ] && pt[ 1 ] <= max[ 1 ] );
		isInside = isInside && ( pt[ 2 ] >= min[ 2 ] && pt[ 2 ] <= max[ 2 ] );
		return isInside;
	}

	bool Inside( const AABB& aabb )
	{
		return ( Inside( aabb.min ) && Inside( aabb.max ) );
	}

	void Expand( const vec3f& pt )
	{
		min[ 0 ] = std::min( pt[ 0 ], min[ 0 ] );
		min[ 1 ] = std::min( pt[ 1 ], min[ 1 ] );
		min[ 2 ] = std::min( pt[ 2 ], min[ 2 ] );

		max[ 0 ] = std::max( pt[ 0 ], max[ 0 ] );
		max[ 1 ] = std::max( pt[ 1 ], max[ 1 ] );
		max[ 2 ] = std::max( pt[ 2 ], max[ 2 ] );
	}

	vec3f GetMin() const
	{
		return min;
	}

	vec3f GetMax() const
	{
		return max;
	}

	vec3f GetSize() const {
		return vec3f( abs( max[ 0 ] - min[ 0 ] ), abs( max[ 1 ] - min[ 1 ] ), abs( max[ 2 ] - min[ 2 ] ) );
	}

	vec3f GetCenter() const {
		return 0.5f * GetSize() + GetMin();
	}
};
