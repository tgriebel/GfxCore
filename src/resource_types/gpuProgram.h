#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "../core/handle.h"


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


struct GpuProgram
{
	static const uint32_t MaxShaders = 2;

	shaderSource_t			shaders[ MaxShaders ];
#ifdef USE_VULKAN
	VkShaderModule			vk_shaders[ MaxShaders ];
#endif
	hdl_t					pipeline;
	uint32_t				shaderCount;
	bool					isCompute;
};