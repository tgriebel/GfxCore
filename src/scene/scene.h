#pragma once

#include <vector>

#include "camera.h"
#include "../core/common.h"
#include "../core/assetLib.h"
#include "../math/mathVector.h"
#include "entity.h"

const uint32_t MaxLights = 3;

struct light_t
{
	vec4f	lightPos;
	vec4f	intensity;
	vec4f	lightDir;
};

class Model;
struct texture_t;
class GpuProgram;

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
		camera.fov = Radians( 90.0f );
		camera.far = defaultFar;
		camera.near = defaultNear;
		camera.aspect = 1.0f;

		camera.halfFovX = tan( 0.5f * Radians( 90.0f ) );
		camera.halfFovY = tan( 0.5f * Radians( 90.0f ) ) / camera.aspect;
		camera.near = camera.near;
		camera.far = camera.far;
		camera.aspect = camera.aspect;
		camera.focalLength = camera.far;
		camera.viewportWidth = 2.0f * camera.halfFovX;
		camera.viewportHeight = 2.0f * camera.halfFovY;
	}

	void CreateEntity( const hdl_t modelHdl, Entity& entity )
	{
		const Model* model = modelLib.Find( modelHdl );
		entity.modelHdl = modelHdl.Get();
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