/*
* MIT License
*
* Copyright( c ) 2021-2023 Thomas Griebel
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
#include <cstdint>
#include <string>
#include <vector>
#include "../core/handle.h"
#include "../core/asset.h"
#include "../io/io.h"


class ShaderBindSet;
class Serializer;


enum shaderType_t : uint32_t
{
	UNSPECIFIED = 0,
	VERTEX,
	PIXEL,
	COMPUTE,
};


enum class pipelineType_t : uint32_t
{
	UNSPECIFIED = 0,
	RASTER,
	COMPUTE,
	RAYTRACING,
};


struct shaderSource_t
{
	std::string			name;
	std::vector<char>	blob;
	shaderType_t		type;
};


enum shaderFlags_t : uint32_t
{
	SHADER_FLAG_NONE			= 0,
	SHADER_FLAG_USE_SAMPLING_MS	= ( 1 << 0 ),
	SHADER_FLAG_IMAGE_SHADER	= ( 1 << 1 ),
};


class GpuProgram
{
public:
	static const uint32_t MaxShaders = 2;
	static const uint32_t MaxBindSets = 5;

	pipelineType_t			type;
	shaderSource_t			shaders[ MaxShaders ];
#ifdef USE_VULKAN
	VkShaderModule			vk_shaders[ MaxShaders ];
#endif
	const ShaderBindSet*	bindsets[ MaxBindSets ];
	uint64_t				bindHash; // Only one bindset is specified in the shader right now
	uint32_t				shaderCount;
	uint32_t				bindsetCount;
	shaderFlags_t			flags;

	friend class LoadHandler<GpuProgram>;

	void Serialize( Serializer* s )
	{
	}
};


class GpuProgramLoader : public LoadHandler<GpuProgram>
{
private:
	std::string		basePath;
	std::string		vsFileName;
	std::string		psFileName;
	std::string		csFileName;
	uint64_t		bindHash;
	shaderFlags_t	flags;

	bool LoadRasterProgram( GpuProgram& program )
	{
		program.type = pipelineType_t::RASTER;
		program.shaderCount = 2;
		program.bindsetCount = 0;

		program.shaders[ 0 ].name = vsFileName;
		program.shaders[ 0 ].blob = ReadFile( basePath + vsFileName );
		program.shaders[ 0 ].type = shaderType_t::VERTEX;

		program.shaders[ 1 ].name = psFileName;
		program.shaders[ 1 ].blob = ReadFile( basePath + psFileName );
		program.shaders[ 1 ].type = shaderType_t::PIXEL;
		
#ifdef USE_VULKAN
		program.vk_shaders[ 0 ] = VK_NULL_HANDLE;
		program.vk_shaders[ 1 ] = VK_NULL_HANDLE;
#endif

		return true;
	}

	bool LoadComputeProgram( GpuProgram& program )
	{
		program.type = pipelineType_t::COMPUTE;
		program.shaderCount = 1;
		program.bindsetCount = 0;

		program.shaders[ 0 ].name = csFileName;
		program.shaders[ 0 ].blob = ReadFile( basePath + csFileName );
		program.shaders[ 0 ].type = shaderType_t::COMPUTE;	

#ifdef USE_VULKAN
		program.vk_shaders[ 0 ] = VK_NULL_HANDLE;
		program.vk_shaders[ 1 ] = VK_NULL_HANDLE;
#endif

		return true;
	}

	bool Load( Asset<GpuProgram>& programAsset )
	{
		GpuProgram& program = programAsset.Get();

		program.bindHash = bindHash;
		program.flags = flags;

		if( ( !vsFileName.empty() ) && ( !psFileName.empty() ) )
		{
			return LoadRasterProgram( program );
		}
		else if( !csFileName.empty() )
		{
			return LoadComputeProgram( program );
		}
		return false;
	}

public:
	GpuProgramLoader() {}
	GpuProgramLoader( const std::string& path, const std::string& vertexFileName, const std::string& pixelFileName )
	{
		flags = SHADER_FLAG_NONE;
		bindHash = 0;
		SetBasePath( path );
		AddFilePaths( vertexFileName, pixelFileName, "" );
	}

	GpuProgramLoader( const std::string& path, const std::string& computeFileName )
	{
		flags = SHADER_FLAG_NONE;
		bindHash = 0;
		SetBasePath( path );
		AddFilePaths( "", "", computeFileName );
	}

	void SetBasePath( const std::string& path )
	{
		basePath = path;
	}

	void SetBindSet( const std::string& setName )
	{
		bindHash = Hash( setName );
	}

	void SetFlags( const shaderFlags_t shaderFlags )
	{
		flags = shaderFlags;
	}

	void AddFilePaths( const std::string& vertexFileName, const std::string& pixelFileName, const std::string& computeFileName )
	{
		vsFileName = vertexFileName;
		psFileName = pixelFileName;
		csFileName = computeFileName;
	}
};

using pShaderLoader_t = Asset<GpuProgram>::loadHandlerPtr_t;