#include "entity.h"
#include "../core/util.h"
#include "../primitives/geom.h"

AABB Entity::GetBounds() const
{
	const vec3f min = vec3f( GetMatrix().Transpose() * vec4f( bounds.GetMin(), 1.0f ) );
	const vec3f max = vec3f( GetMatrix().Transpose() * vec4f( bounds.GetMax(), 1.0f ) );
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
	result[ 3 ][ 0 ] = translation[ 0 ];
	result[ 3 ][ 1 ] = translation[ 1 ];
	result[ 3 ][ 2 ] = translation[ 2 ];
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