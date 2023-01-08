#include "entity.h"
#include "../core/util.h"
#include "../primitives/geom.h"

AABB Entity::GetBounds() const {
	const vec3f min = vec3f( GetMatrix().Transpose() * vec4f( bounds.GetMin(), 1.0f ) );
	const vec3f max = vec3f( GetMatrix().Transpose() * vec4f( bounds.GetMax(), 1.0f ) );
	return AABB( min, max );
}

void Entity::ExpandBounds( const AABB& modelBounds ) {
	bounds.Expand( modelBounds.GetMin() );
	bounds.Expand( modelBounds.GetMax() );
}

vec3f Entity::GetOrigin() const {
	return vec3f( matrix[ 3 ][ 0 ], matrix[ 3 ][ 1 ], matrix[ 3 ][ 2 ] );
}

void Entity::SetOrigin( const vec3f& origin ) {
	matrix[ 3 ][ 0 ] = origin[ 0 ];
	matrix[ 3 ][ 1 ] = origin[ 1 ];
	matrix[ 3 ][ 2 ] = origin[ 2 ];
}

void Entity::SetScale( const vec3f& scale ) {
	matrix[ 0 ][ 0 ] = scale[ 0 ];
	matrix[ 1 ][ 1 ] = scale[ 1 ];
	matrix[ 2 ][ 2 ] = scale[ 2 ];
}

void Entity::SetRotation( const vec3f& xyzDegrees ) {
	const mat4x4f rotationMatrix = ComputeRotationZYX( xyzDegrees[ 0 ], xyzDegrees[ 1 ], xyzDegrees[ 2 ] );
	matrix = rotationMatrix * matrix;
}

mat4x4f Entity::GetMatrix() const {
	return matrix;
}

void Entity::SetFlag( const entityFlags_t flag ) {
	flags = static_cast<entityFlags_t>( flags | flag );
}

void Entity::ClearFlag( const entityFlags_t flag ) {
	flags = static_cast<entityFlags_t>( flags & ~flag );
}

bool Entity::HasFlag( const entityFlags_t flag ) const {
	return ( ( flags & flag ) != 0 );
}