#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "../core/handle.h"
#include "../core/asset.h"
#include "../io/io.h"


enum shaderType_t : uint32_t
{
	UNSPECIFIED = 0,
	VERTEX,
	PIXEL,
	COMPUTE,
};


struct shaderSource_t
{
	std::string			name;
	std::vector<char>	blob;
	shaderType_t		type;
};


class GpuProgram
{
public:
	static const uint32_t MaxShaders = 2;

	shaderSource_t			shaders[ MaxShaders ];
#ifdef USE_VULKAN
	VkShaderModule			vk_shaders[ MaxShaders ];
#endif
	hdl_t					pipeline;
	uint32_t				shaderCount;
	bool					isCompute;

	void Serialize( Serializer* serializer );

	friend class LoadHandler<GpuProgram>;
};


class GpuProgramLoader : public LoadHandler<GpuProgram>
{
private:
	std::string vsPath;
	std::string psPath;
	std::string csPath;

	void LoadRasterProgram( GpuProgram& program )
	{
		program.shaders[ 0 ].name = vsPath;
		program.shaders[ 0 ].blob = ReadFile( vsPath );
		program.shaders[ 1 ].name = psPath;
		program.shaders[ 1 ].blob = ReadFile( psPath );
		program.shaderCount = 2;
		program.isCompute = false;
	}

	void LoadCsProgram( GpuProgram& program )
	{
		program.shaders[ 0 ].name = csPath;
		program.shaders[ 0 ].blob = ReadFile( csPath );
		program.shaderCount = 1;
		program.isCompute = true;
	}
public:
	void Load( GpuProgram& program )
	{
		if( ( vsPath != "" ) && ( psPath != "" ) )
		{
			LoadRasterProgram( program );
		}
		else if( psPath != "" )
		{
			LoadCsProgram( program );
		}
	}

	void AddRasterPath( const std::string vertexPath, const std::string pixelPath )
	{
		vsPath = vertexPath;
		psPath = pixelPath;
		csPath = "";
	}

	void AddComputePath( const std::string path )
	{
		vsPath = "";
		psPath = "";
		csPath = path;
	}
};