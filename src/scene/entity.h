#pragma once

#include "../math/matrix.h"
#include "../core/handle.h"
#include "../acceleration/aabb.h"

enum entityFlags_t
{
	ENT_FLAG_NONE			= 0,
	ENT_FLAG_SELECTABLE		= ( 1 << 0 ),
	ENT_FLAG_NO_DRAW		= ( 1 << 1 ),
	ENT_FLAG_NO_SHADOWS		= ( 1 << 2 ),
	ENT_FLAG_WIREFRAME		= ( 1 << 3 ),
	ENT_FLAG_DEBUG			= ( 1 << 4 ),
};

class Entity
{
public:
	Entity()
	{
		orientation = mat4x4f( 1.0f );
		scale = mat4x4f( 1.0f );
		translation = vec4f( 0.0f );
		modelHdl = INVALID_HDL;
		materialHdl = INVALID_HDL;
		flags = ENT_FLAG_NONE;
		outline = false;
	}

	Entity( const Entity& ent )
	{
		orientation = ent.orientation;
		scale = ent.scale;
		translation = ent.translation;
		modelHdl = ent.modelHdl;
		materialHdl = ent.materialHdl;
		flags = ent.flags;
		outline = ent.flags;
	}

	std::string		name;
	bool			outline;
	hdl_t			modelHdl;
	hdl_t			materialHdl;

	AABB			GetBounds() const;
	void			ExpandBounds( const AABB& bounds );
	vec3f			GetOrigin() const;
	void			SetOrigin( const vec3f& origin );
	vec3f			GetScale() const;
	void			SetScale( const vec3f& scale );
	mat4x4f			GetRotation() const;
	void			SetRotation( const vec3f& xyzDegrees );
	mat4x4f			GetMatrix() const;
	void			SetFlag( const entityFlags_t flag );
	void			ClearFlag( const entityFlags_t flag );
	bool			HasFlag( const entityFlags_t flag ) const;

private:
	entityFlags_t	flags;
	mat4x4f			orientation;
	mat4x4f			scale;
	vec4f			translation;
	AABB			bounds;
};