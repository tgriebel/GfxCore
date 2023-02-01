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
#include "../math/vector.h"
#include "../math/matrix.h"
#include "../primitives/ray.h"

class AABB
{
private:
	static const uint32_t Version = 1;
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

	const vec3f& GetMin() const
	{
		return min;
	}

	const vec3f& GetMax() const
	{
		return max;
	}

	vec3f GetSize() const {
		return vec3f( abs( max[ 0 ] - min[ 0 ] ), abs( max[ 1 ] - min[ 1 ] ), abs( max[ 2 ] - min[ 2 ] ) );
	}

	vec3f GetCenter() const {
		return 0.5f * GetSize() + GetMin();
	}

	void Serialize( Serializer* serializer );
};


static inline std::ostream& operator<<( std::ostream& stream, const AABB& aabb )
{
	stream << "[min=(" << aabb.GetMin()[0] << ", " << aabb.GetMin()[1] << ", " << aabb.GetMin()[2] << ")";
	stream << ", ";
	stream << "max=(" << aabb.GetMax()[0] << ", " << aabb.GetMax()[1] << ", " << aabb.GetMax()[2] << ")]";
	return stream;
}