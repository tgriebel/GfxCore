#pragma once

#include <cstdint>

struct GpuImage
{
#ifdef USE_VULKAN
	VkImage			vk_image;
	VkImageView		vk_view;
	AllocationVk	allocation;
#endif
};

enum textureType_t
{
	TEXTURE_TYPE_UNKNOWN,
	TEXTURE_TYPE_2D,
	TEXTURE_TYPE_CUBE,
};

struct textureInfo_t {
	uint32_t		width;
	uint32_t		height;
	uint32_t		channels;
	uint32_t		mipLevels;
	uint32_t		layers;
	textureType_t	type;
};

struct texture_t
{
	uint8_t*		bytes;
	uint32_t		sizeBytes;
	textureInfo_t	info;
	int				uploadId;

	GpuImage		image;

	texture_t() {
		info.width = 0;
		info.height = 0;
		info.channels = 0;
		info.mipLevels = 0;
		info.type = TEXTURE_TYPE_UNKNOWN;
		uploadId = -1;
		bytes = nullptr;
	}
};