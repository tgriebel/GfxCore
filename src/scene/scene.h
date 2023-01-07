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

const uint32_t MaxLights = 3;

struct light_t
{
	vec4f	lightPos;
	vec4f	intensity;
	vec4f	lightDir;
	Color	color;
};

typedef AssetLib< Model >			AssetLibModels;
typedef AssetLib< texture_t >		AssetLibImages;
typedef AssetLib< Material >		AssetLibMaterials;
typedef AssetLib< GpuProgram >		AssetLibGpuProgram;

struct Scene
{
	Camera						camera;
	std::vector<Entity*>		entities;
	AssetLibModels				modelLib;
	AssetLibImages				textureLib;
	AssetLibMaterials			materialLib;
	AssetLibGpuProgram			gpuPrograms;
	light_t						lights[ MaxLights ];
	float						defaultNear = 1000.0f;
	float						defaultFar = 0.1f;

	Scene()
	{
		camera = Camera( vec4f( 0.0f, 1.66f, 1.0f, 0.0f ) );
		camera.far = defaultFar;
		camera.near = defaultNear;
		camera.focalLength = camera.far;
		camera.SetFov( Radians( 90.0f ) );
		camera.SetAspectRatio( 1.0f );
	}

	void CreateEntity( const hdl_t modelHdl, Entity& entity )
	{
		const Model* model = modelLib.Find( modelHdl );
		entity.modelHdl = modelHdl.Get();
		entity.ExpandBounds( model->bounds );
	}

	Entity* FindEntity( const uint32_t entityIx ) {
		return entities[ entityIx ];
	}

	Entity* FindEntity( const char* name ) {
		const uint32_t entCount = static_cast<uint32_t>( entities.size() );
		for ( uint32_t i = 0; i < entCount; ++i ) {
			if ( entities[ i ]->dbgName == name ) {
				return entities[ i ];
			}
		}
		return nullptr;
	}
};