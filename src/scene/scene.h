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

#include <vector>

#include "camera.h"
#include "../core/common.h"
#include "../core/assetLib.h"
#include "../math/vector.h"
#include "../image/color.h"
#include "../resource_types/texture.h"
#include "../resource_types/material.h"
#include "../resource_types/gpuProgram.h"
#include "../resource_types/model.h"
#include "entity.h"
#include "assetManager.h"

const uint32_t MaxLights = 3;

struct light_t
{
	vec4f	lightPos;
	vec4f	intensity;
	vec4f	lightDir;
	Color	color;
};

struct Ray;

extern AssetManager gAssets;

class Scene
{
public:
	Camera						camera;
	std::vector<Entity*>		entities;
	light_t						lights[ MaxLights ];
	float						defaultNear = 0.1f;
	float						defaultFar = 1000.0f;
	Entity*						selectedEntity = nullptr;

	virtual void Update( const float dt ) {}
	virtual void Init() {}

	Scene()
	{
		camera = Camera( vec4f( 0.0f, 1.66f, 1.0f, 0.0f ) );
		camera.far = defaultFar;
		camera.near = defaultNear;
		camera.focalLength = camera.far;

		camera.SetFov( Radians( 90.0f ) );
		camera.SetAspectRatio( 1.0f );
	}

	void			CreateEntityBounds( const hdl_t modelHdl, Entity& entity );
	Entity*			GetTracedEntity( const Ray& ray );

	uint32_t		EntityCount() const;
	Entity*			FindEntity( const uint32_t entityIx );
	const Entity*	FindEntity( const uint32_t entityIx ) const ;
	Entity*			FindEntity( const char* name );
	const Entity*	FindEntity( const char* name ) const;
};