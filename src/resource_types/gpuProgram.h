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
		return false;
	}

public:
	GpuProgramLoader() {}
	GpuProgramLoader( const std::string& path, const std::string& vertexFileName, const std::string& pixelFileName )
	{
		SetBasePath( path );
		AddRasterPath( vertexFileName, pixelFileName );
	}

	GpuProgramLoader( const std::string& path, const std::string& computeFileName )
	{
		SetBasePath( path );
		AddComputePath( computeFileName );
	}

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

using pShaderLoader_t = Asset<GpuProgramLoader>::loadHandlerPtr_t;