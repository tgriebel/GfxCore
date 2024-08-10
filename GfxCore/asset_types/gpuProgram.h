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
#include <syscore/systemUtils.h>
#include <syscore/common.h>


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
	std::string			binName;
	std::vector<char>	src;
	std::vector<char>	blob;
	shaderType_t		type;
};


enum class shaderFlags_t : uint32_t
{
	NONE					= 0,
	USE_MSAA				= ( 1 << 0 ),
	USE_CUBE_SAMPLER		= ( 1 << 1 ),
	IMAGE_SHADER			= ( 1 << 2 ),
};
DEFINE_ENUM_OPERATORS( shaderFlags_t, uint32_t )


struct shaderPerm_t
{
	shaderFlags_t	flags;
	std::string		macro;
	std::string		tag;
};


enum class shaderPermId_t : int32_t
{
	NONE				= -1,
	MSAA				= 0,
	SKY_CUBE_SAMPLER	= 1,
	COUNT
};
DEFINE_ENUM_OPERATORS( shaderPermId_t, uint32_t )

#define SHADER_PERM(FLAG, TAG) { shaderFlags_t::FLAG, #FLAG, TAG }

static const shaderPerm_t ShaderPerms[] = {	SHADER_PERM( USE_MSAA,			"msaa" ),
											SHADER_PERM( USE_CUBE_SAMPLER,	"skycube" )};

static shaderPermId_t GetPermId( const std::string& perm )
{
	for( uint32_t i = 0; i < shaderPermId_t::COUNT; ++i )
	{
		if( perm == ShaderPerms[ i ].tag ) {
			return shaderPermId_t( i );
		}
	}
	return shaderPermId_t::NONE;
}


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
	shaderPermId_t			perm;

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
	shaderPermId_t	perm;

	static std::string GetMacros( const std::string perms[] )
	{
	//	for
	}

	static std::string GetBinName( const std::string& fileName, const std::string perms[] )
	{
		std::string name;
		std::string ext;
		SplitFileName( fileName, name, ext );

		if( ext == "vert" ) {
			name += "VS";
		}
	}

	bool LoadRasterProgram( GpuProgram& program )
	{
		program.type = pipelineType_t::RASTER;
		program.shaderCount = 2;
		program.bindsetCount = 0;

		program.shaders[ 0 ].name = vsFileName;
		program.shaders[ 0 ].binName = "";
		program.shaders[ 0 ].blob = ReadFile( basePath + vsFileName );
		program.shaders[ 0 ].type = shaderType_t::VERTEX;

		program.shaders[ 1 ].name = psFileName;
		program.shaders[ 1 ].binName = "";
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
		program.shaders[ 0 ].binName = "";
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
		flags = shaderFlags_t::NONE;
		perm = shaderPermId_t::NONE;
		bindHash = 0;
		SetBasePath( path );
		AddFilePaths( vertexFileName, pixelFileName, "" );
	}

	GpuProgramLoader( const std::string& path, const std::string& computeFileName )
	{
		flags = shaderFlags_t::NONE;
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

	void SetPerm( const std::string& permName )
	{
		perm = GetPermId( permName );
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