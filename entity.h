#pragma once

#include "matrix.h"
#include "handle.h"
#include "aabb.h"

enum entityFlags_t {
	ENT_FLAG_NONE			= 0,
	ENT_FLAG_SELECTABLE		= ( 1 << 0 ),
	ENT_FLAG_NO_DRAW		= ( 1 << 1 ),
	ENT_FLAG_NO_SHADOWS		= ( 1 << 2 ),
	ENT_FLAG_WIREFRAME		= ( 1 << 3 ),
};

class Entity
{
public:
	Entity() {
		matrix = mat4x4f( 1.0f );
		modelHdl = INVALID_HDL;
		materialHdl = INVALID_HDL;
		flags = ENT_FLAG_NONE;
		outline = false;
	}

	std::string		dbgName;
	bool			outline;
	hdl_t			modelHdl;
	hdl_t			materialHdl;

	AABB			GetBounds() const;
	vec3f			GetOrigin() const;
	void			SetOrigin( const vec3f& origin );
	void			SetScale( const vec3f& scale );
	void			SetRotation( const vec3f& xyzDegrees );
	mat4x4f			GetMatrix() const;
	void			SetFlag( const entityFlags_t flag );
	void			ClearFlag( const entityFlags_t flag );
	bool			HasFlag( const entityFlags_t flag ) const;

private:
	entityFlags_t	flags;
	mat4x4f			matrix;
};