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
	ENT_FLAG_CAMERA_LOCKED	= ( 1 << 5 ),
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

	AABB			GetLocalBounds() const;
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