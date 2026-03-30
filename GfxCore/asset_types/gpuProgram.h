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
	NO_VERTEX_BUFFER		= ( 1 << 3 ),
	USE_MRT					= ( 1 << 4 ),
};
DEFINE_ENUM_OPERATORS( shaderFlags_t, uint32_t )


enum class shaderPermId_t : int32_t
{
	NONE				= 0,
	MSAA				= ( 1 << 0 ),
	SKY_CUBE_SAMPLER	= ( 1 << 1 ),
	MRT					= ( 1 << 2 ),
	COUNT				= 3
};
DEFINE_ENUM_OPERATORS( shaderPermId_t, uint32_t )


// Permutations are used for variations on a single shaders
// Flags are used to manage code. The same flag might be set for all permutations of a shader
// Right now just 1 flag is supported, but should change
struct shaderPerm_t
{
	shaderFlags_t	flags;
	shaderPermId_t	id;
	std::string		macro;
	std::string		tag;
};


#define SHADER_PERM(FLAG, PERM, TAG) { shaderFlags_t::FLAG, shaderPermId_t::PERM, #FLAG, TAG }

static const shaderPerm_t ShaderPerms[] = {	SHADER_PERM( USE_MSAA,			MSAA,				"msaa" ),
											SHADER_PERM( USE_MRT,			MRT,				"mrt" ),
											SHADER_PERM( USE_CUBE_SAMPLER,	SKY_CUBE_SAMPLER,	"skycube" )};

static shaderPermId_t GetPermId( const std::string& perm )
{
	for( uint32_t i = 0; i < shaderPermId_t::COUNT; ++i )
	{
		if( perm == ShaderPerms[ i ].tag ) {
			return ShaderPerms[ i ].id;
		}
	}
	return shaderPermId_t::NONE;
}


static const shaderPerm_t* FindPerm( const shaderPermId_t permId )
{
	for ( uint32_t i = 0; i < shaderPermId_t::COUNT; ++i )
	{
		if ( permId == ShaderPerms[ i ].id ) {
			return &ShaderPerms[ i ];
		}
	}
	return nullptr;
}


static const shaderPerm_t* FindPerm( const std::string& perm )
{
	for ( uint32_t i = 0; i < shaderPermId_t::COUNT; ++i )
	{
		if ( perm == ShaderPerms[ i ].tag ) {
			return &ShaderPerms[ i ];
		}
	}
	return nullptr;
}


class GpuProgram
{
public:
	static const uint32_t MaxShaders = 2;
	static const uint32_t MaxBindSets = 5;
	static const uint32_t MaxPermutations = ( 1 << static_cast<uint32_t>( shaderPermId_t::COUNT ) );

	pipelineType_t			type;
	uint64_t				bindHash;		// Only one bindset is specified in the shader right now
	uint32_t				shaderCount;	// Number of shaders within the pipeline (e.g. 2 for VS/PS or 1 for CS)
	uint32_t				bindsetCount;	// Bindset for inputs
	uint32_t				permCount;		// Number of unique permutation combinations
	shaderFlags_t			flags;
	shaderPermId_t			permSet;		// Superset of all available permutations. Subsets can be selected
	shaderSource_t			shaders[ MaxPermutations ][ MaxShaders ];
#ifdef USE_VULKAN
	VkShaderModule			vk_shaders[ MaxPermutations ][ MaxShaders ];
#endif
	const ShaderBindSet*	bindsets[ MaxBindSets ];

	friend class LoadHandler<GpuProgram>;

	GpuProgram()
	{
		type = pipelineType_t::UNSPECIFIED;

		shaderCount = 0;
		bindHash = 0;
		permCount = 0;
		flags = shaderFlags_t::NONE;
		permSet = shaderPermId_t::NONE;

		for ( uint32_t bindIndex = 0; bindIndex < MaxBindSets; ++bindIndex )
		{
			bindsets[ bindIndex ] = nullptr;
		}

		for ( uint32_t permIndex = 0; permIndex < GpuProgram::MaxPermutations; ++permIndex )
		{
			for ( uint32_t shaderIndex = 0; shaderIndex < GpuProgram::MaxShaders; ++shaderIndex )
			{
				shaders[ permIndex ][ shaderIndex ] = {};
#ifdef USE_VULKAN
				vk_shaders[ permIndex ][ shaderIndex ] = VK_NULL_HANDLE;
#endif
			}
		}
	}

	void Serialize( Serializer* s )
	{
	}
};


class GpuProgramLoader : public LoadHandler<GpuProgram>
{
private:
	std::string		srcPath;
	std::string		binPath;
	std::string		compilerPath;
	std::string		vsFileName;
	std::string		psFileName;
	std::string		csFileName;
	uint64_t		bindHash;
	uint32_t		permIdCount;
	shaderFlags_t	flags;
	shaderPermId_t	permList[ GpuProgram::MaxPermutations ];

	static std::string	GetBinName( const std::string& fileName, const shaderPermId_t permSet );
	std::string			GetCompileString( const std::string& srcPath, const std::string& binPath, const std::string& perms );
	void				CheckCompileShader( const std::string& path, const std::string& binPath, const shaderPermId_t permSet, const bool forceRebuild = false );
	bool				LoadRasterProgram( GpuProgram& program );
	bool				LoadComputeProgram( GpuProgram& program );
	bool				Load( Asset<GpuProgram>& programAsset );

public:
	GpuProgramLoader()
	{
		permIdCount = 1;
		for ( uint32_t i = 0; i < permIdCount; ++i ) {
			permList[ i ] = shaderPermId_t::NONE;
		}
	}

	void SetSourcePath( const std::string& path );
	void SetBinPath( const std::string& path );
	void SetBindSet( const std::string& setName );
	void AddPerm( const std::string& permName );
	void SetFlags( const shaderFlags_t shaderFlags );
	void SetCompilerPath( const std::string& path );
	void AddFilePaths( const std::string& vertexFileName, const std::string& pixelFileName, const std::string& computeFileName );
};

using pShaderLoader_t = Asset<GpuProgram>::loadHandlerPtr_t;