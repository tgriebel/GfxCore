#pragma once

#include "../resource_types/texture.h"
#include "../resource_types/material.h"
#include "../resource_types/gpuProgram.h"
#include "../resource_types/model.h"
#include "../core/assetLib.h"
#include "../core/asset.h"

typedef AssetLib< Model >			AssetLibModels;
typedef AssetLib< Texture >			AssetLibImages;
typedef AssetLib< Material >		AssetLibMaterials;
typedef AssetLib< GpuProgram >		AssetLibGpuProgram;

class AssetManager
{
public:
	AssetLibModels				modelLib;
	AssetLibImages				textureLib;
	AssetLibMaterials			materialLib;
	AssetLibGpuProgram			gpuPrograms;

	inline bool HasPendingLoads()
	{
		bool hasItems = false;
		hasItems = hasItems || gpuPrograms.HasPendingLoads();
		hasItems = hasItems || modelLib.HasPendingLoads();
		hasItems = hasItems || textureLib.HasPendingLoads();
		hasItems = hasItems || materialLib.HasPendingLoads();
		return hasItems;
	}

	void RunLoadLoop( const uint32_t limit = 12 )
	{
		uint32_t i = 0;
		while ( HasPendingLoads() && ( i < limit ) )
		{
			gpuPrograms.LoadAll();		
			textureLib.LoadAll();
			materialLib.LoadAll();
			modelLib.LoadAll();
			++i;
		}
	}
};