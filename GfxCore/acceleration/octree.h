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

#include <vector>
#include "../primitives/geom.h"
#include "../primitives/ray.h"

enum octreeRegion_t : uint8_t
{
	XN_YN_ZN = 0x0,
	XN_YN_ZP = 0x1,
	XN_YP_ZN = 0x2,
	XN_YP_ZP = 0x3,
	XP_YN_ZN = 0x4,
	XP_YN_ZP = 0x5,
	XP_YP_ZN = 0x6,
	XP_YP_ZP = 0x7,
	REGION_COUNT
};


template<typename T>
class Octree
{
public:

	static const uint32_t MaxHeight = 1000;

	Octree()
	{
		height = 0;
		bitSet = 0;
		parent = nullptr;
	}

	Octree( const vec3f& _min, const vec3f& _max )
	{
		height = 0;
		bitSet = 0;
		parent = nullptr;
		aabb.Expand( _min );
		aabb.Expand( _max );
	}

	bool Insert( const AABB& bounds, const T& item, uint32_t depth = 0 )
	{
		if( aabb.Inside( bounds ) && ( depth < MaxHeight ) )
		{
			bool inChildNode = false;

			if( !HasChildren() )
			{
				Split(); // For now just split the node if it's inside
			}

			const uint32_t childCnt = static_cast<uint32_t>( children.size() );
			for( uint32_t i = 0; i < childCnt; ++i )
			{
				if( children[ i ].Insert( bounds, item, depth + 1 ) )
				{
					inChildNode = true;
					break;
				}
			}

			if( !inChildNode )
			{
				items.push_back( item );
			}

			return true;
		}
		return false;
	}

	// TODO: move out of tree
	bool Intersect( const Ray& ray, std::vector<T>& hitItems ) const
	{
		float tNear;
		float tFar;

		if ( aabb.Intersect( ray, tNear, tFar ) )
		{
			const uint32_t childCnt = static_cast<uint32_t>( children.size() );
			for ( uint32_t i = 0; i < childCnt; ++i )
			{
				children[ i ].Intersect( ray, hitItems );
			}

			const uint32_t itemCnt = static_cast<uint32_t>( items.size() );
			for ( uint32_t itemIx = 0; itemIx < itemCnt; ++itemIx )
			{
				hitItems.push_back( items[ itemIx ] );
			}

			return true;
		}
		return false;
	}

	void IncreaseHeight()
	{
		height += 1;

		if( parent != nullptr )
		{
			parent->IncreaseHeight();
		}
	}

	bool HasChild( const octreeRegion_t region ) const
	{
		const uint8_t regionMask = ( 1 << region );
		return ( ( bitSet & regionMask ) != 0 );
	}

	bool HasChildren() const
	{
		return ( children.size() > 0 );
	}

	void Split()
	{
		// Can only 0 or 8 children currently
		if( children.size() >= REGION_COUNT )
		{
			return;
		}

		children.reserve( 8 );

		for( uint8_t i = 0; i < REGION_COUNT; ++i )
		{
			AddChild( (octreeRegion_t)i );
		}

		IncreaseHeight();
	}

	void SplitChild( const octreeRegion_t region )
	{
		if ( HasChild( region ) )
		{
			children[ region ].Split();
		}
	}

	AABB GetAABB() const
	{
		return aabb;
	}

private:
	void AddChild( const octreeRegion_t region )
	{
		const vec3f halfDist = 0.5f * ( aabb.max - aabb.min );
		
		const vec3f mask = vec3f(	( region & 0x04 ) ? 1.0f : 0.0f,
									( region & 0x02 ) ? 1.0f : 0.0f,
									( region & 0x01 ) ? 1.0f : 0.0f );

		const vec3f offset = vec3f( mask[ 0 ] * halfDist[ 0 ], mask[ 1 ] * halfDist[ 1 ], mask[ 2 ] * halfDist[ 2 ] );
		const vec3f nodeMin = aabb.min + offset;
		const vec3f nodeMax = nodeMin + halfDist;
		
		children.push_back( Octree( nodeMin, nodeMax ) );
		children.back().parent = this;

		bitSet |= ( 1 << region );
	}

	AABB				aabb;
	Octree<T>*			parent;
	uint32_t			height;
	uint8_t				bitSet;

	// TODO: make private
public:
	std::vector<T>				items;
	std::vector< Octree<T> >	children;
};