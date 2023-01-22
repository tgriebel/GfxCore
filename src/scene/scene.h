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

extern AssetManager gAssets;

class Scene
{
public:
	Camera						camera;
	std::vector<Entity*>		entities;
	light_t						lights[ MaxLights ];
	float						defaultNear = 0.1f;
	float						defaultFar = 1000.0f;

	Scene()
	{
		camera = Camera( vec4f( 0.0f, 1.66f, 1.0f, 0.0f ) );
		camera.far = defaultFar;
		camera.near = defaultNear;
		camera.focalLength = camera.far;
		camera.SetFov( Radians( 90.0f ) );
		camera.SetAspectRatio( 1.0f );
	}

	void CreateEntityBounds( const hdl_t modelHdl, Entity& entity )
	{
		const Model& model = gAssets.modelLib.Find( modelHdl )->Get();
		entity.modelHdl = modelHdl.Get();
		entity.ExpandBounds( model.bounds );
	}

	Entity* FindEntity( const uint32_t entityIx ) {
		return entities[ entityIx ];
	}

	Entity* FindEntity( const char* name ) {
		const uint32_t entCount = static_cast<uint32_t>( entities.size() );
		for ( uint32_t i = 0; i < entCount; ++i ) {
			if ( entities[ i ]->name == name ) {
				return entities[ i ];
			}
		}
		return nullptr;
	}
};