/*
* MIT License
*
* Copyright( c ) 2022-2023 Thomas Griebel
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

#include "entity.h"
#include "../core/util.h"
#include "../primitives/geom.h"

AABB Entity::GetBounds() const
{
	const vec3f min = vec3f( GetMatrix() * vec4f( bounds.GetMin(), 1.0f ) );
	const vec3f max = vec3f( GetMatrix() * vec4f( bounds.GetMax(), 1.0f ) );
	return AABB( min, max );
}


void Entity::ExpandBounds( const AABB& modelBounds )
{
	bounds.Expand( modelBounds.GetMin() );
	bounds.Expand( modelBounds.GetMax() );
}


vec3f Entity::GetOrigin() const
{
	return vec3f( translation );
}


vec3f Entity::GetScale() const
{
	return vec3f( scale[0][0], scale[1][1], scale[2][2] );
}


void Entity::SetOrigin( const vec3f& origin )
{
	translation[0] = origin[ 0 ];
	translation[1] = origin[ 1 ];
	translation[2] = origin[ 2 ];
}


void Entity::SetScale( const vec3f& s )
{
	scale[ 0 ][ 0 ] = s[ 0 ];
	scale[ 1 ][ 1 ] = s[ 1 ];
	scale[ 2 ][ 2 ] = s[ 2 ];
	scale[ 3 ][ 3 ] = 1.0f;
}


mat4x4f Entity::GetRotation() const
{
	return orientation;
}


void Entity::SetRotation( const vec3f& xyzDegrees )
{
	orientation = ComputeRotationZYX( xyzDegrees[ 0 ], xyzDegrees[ 1 ], xyzDegrees[ 2 ] );
}


mat4x4f Entity::GetMatrix() const
{
	mat4x4f result;
	result = scale * orientation;
	result[ 0 ][ 3 ] = translation[ 0 ];
	result[ 1 ][ 3 ] = translation[ 1 ];
	result[ 2 ][ 3 ] = translation[ 2 ];
	result[ 3 ][ 3 ] = 1.0f;
	return result;
}


void Entity::SetFlag( const entityFlags_t flag )
{
	flags = static_cast<entityFlags_t>( flags | flag );
}


void Entity::ClearFlag( const entityFlags_t flag )
{
	flags = static_cast<entityFlags_t>( flags & ~flag );
}


bool Entity::HasFlag( const entityFlags_t flag ) const
{
	return ( ( flags & flag ) != 0 );
}