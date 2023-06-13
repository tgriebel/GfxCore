/*
* MIT License
*
* Copyright( c ) 2023 Thomas Griebel
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

#include "../asset_types/texture.h"
#include "../asset_types/material.h"
#include "../asset_types/gpuProgram.h"
#include "../asset_types/model.h"
#include "../core/assetLib.h"
#include "../core/asset.h"

typedef AssetLib< Model >			AssetLibModels;
typedef AssetLib< Image >			AssetLibImages;
typedef AssetLib< Material >		AssetLibMaterials;
typedef AssetLib< GpuProgram >		AssetLibGpuProgram;

class AssetManager
{
public:
	std::vector<Library*>		libraries;
	AssetLibModels				modelLib;
	AssetLibImages				textureLib;
	AssetLibMaterials			materialLib;
	AssetLibGpuProgram			gpuPrograms;

	AssetManager()
	{
		libraries.push_back( &modelLib );
		libraries.push_back( &textureLib );
		libraries.push_back( &materialLib );
		libraries.push_back( &gpuPrograms );
	}

	void Clear()
	{
		for( auto it = libraries.begin(); it != libraries.end(); ++it ) {
			(*it)->Clear();
		}
	}

	inline bool HasPendingLoads()
	{
		for ( auto it = libraries.begin(); it != libraries.end(); ++it ) {
			if( ( *it )->HasPendingLoads() ) {
				return true;
			}
		}
		return false;
	}

	void RunLoadLoop( const uint32_t limit = 12 )
	{
		uint32_t i = 0;
		while ( i < limit )
		{
			bool hasLoads = false;
			for ( auto it = libraries.begin(); it != libraries.end(); ++it )
			{
				if ( ( *it )->HasPendingLoads() )
				{
					( *it )->LoadAll();
					hasLoads = true;
				}
			}
			if( hasLoads == false ) {
				break;
			}
			++i;
		}
	}
};