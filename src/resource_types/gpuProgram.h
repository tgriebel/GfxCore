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

	friend class LoadHandler<GpuProgram>;
};


class GpuProgramLoader : public LoadHandler<GpuProgram>
{
private:
	std::string basePath;
	std::string vsFileName;
	std::string psFileName;
	std::string csFileName;

	bool LoadRasterProgram( GpuProgram& program )
	{
		program.shaders[ 0 ].name = vsFileName;
		program.shaders[ 0 ].blob = ReadFile( basePath + vsFileName );
		program.shaders[ 1 ].name = psFileName;
		program.shaders[ 1 ].blob = ReadFile( basePath + psFileName );
		program.shaderCount = 2;
		program.isCompute = false;
		return true;
	}

	bool LoadComputeProgram( GpuProgram& program )
	{
		program.shaders[ 0 ].name = csFileName;
		program.shaders[ 0 ].blob = ReadFile( basePath + csFileName );
		program.shaderCount = 1;
		program.isCompute = true;
		return true;
	}

	bool Load( GpuProgram& program )
	{
		if( ( vsFileName != "" ) && ( psFileName != "" ) )
		{
			return LoadRasterProgram( program );
		}
		else if( csFileName != "" )
		{
			return LoadComputeProgram( program );
		}
	}

public:
	void SetBasePath( const std::string& path )
	{
		basePath = path;
	}

	void AddRasterPath( const std::string& vertexFileName, const std::string& pixelFileName )
	{
		vsFileName = vertexFileName;
		psFileName = pixelFileName;
		csFileName = "";
	}

	void AddComputePath( const std::string& name )
	{
		vsFileName = "";
		psFileName = "";
		csFileName = name;
	}
};