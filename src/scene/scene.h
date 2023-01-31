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