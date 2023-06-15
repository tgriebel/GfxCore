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
#include "../io/io.h"
#include "../core/asset.h"

class GpuImage;

enum imageType_t : uint8_t
{
	IMAGE_TYPE_UNKNOWN,
	IMAGE_TYPE_2D,
	IMAGE_TYPE_2D_ARRAY,
	IMAGE_TYPE_3D,
	IMAGE_TYPE_3D_ARRAY,
	IMAGE_TYPE_CUBE,
	IMAGE_TYPE_CUBE_ARRAY,
	IMAGE_TYPE_DEPTH,
	IMAGE_TYPE_STENCIL,
	IMAGE_TYPE_DEPTH_STENCIL,
};


enum imageAspectFlags_t : uint8_t
{
	IMAGE_ASPECT_COLOR_FLAG = ( 1 << 0 ),
	IMAGE_ASPECT_DEPTH_FLAG = ( 1 << 1 ),
	IMAGE_ASPECT_STENCIL_FLAG = ( 1 << 2 ),
};


enum imageTiling_t : uint8_t
{
	IMAGE_TILING_LINEAR,
	IMAGE_TILING_MORTON,
};


enum imageFmt_t : uint8_t
{
	IMAGE_FMT_UNKNOWN,
	IMAGE_FMT_R_8,
	IMAGE_FMT_R_16,
	IMAGE_FMT_R_32,
	IMAGE_FMT_D_16,
	IMAGE_FMT_D24S8,
	IMAGE_FMT_D_32,
	IMAGE_FMT_D_32_S8,
	IMAGE_FMT_RGB_8,
	IMAGE_FMT_RGBA_8,
	IMAGE_FMT_ABGR_8,
	IMAGE_FMT_BGR_8,
	IMAGE_FMT_BGRA_8,
	IMAGE_FMT_RGB_16,
	IMAGE_FMT_RGBA_16,
	IMAGE_FMT_R11G11B10,
};


enum imageSamples_t : uint8_t
{
	IMAGE_SMP_1 = (1 << 0),
	IMAGE_SMP_2 = ( 1 << 1 ),
	IMAGE_SMP_4 = ( 1 << 2 ),
	IMAGE_SMP_8 = ( 1 << 3 ),
	IMAGE_SMP_16 = ( 1 << 4 ),
	IMAGE_SMP_32 = ( 1 << 5 ),
	IMAGE_SMP_64 = ( 1 << 6 ),
};


struct imageInfo_t
{
	uint32_t				width;
	uint32_t				height;
	uint32_t				channels;
	uint32_t				mipLevels;
	uint32_t				layers;
	imageSamples_t			subsamples;
	imageType_t				type;
	imageFmt_t				fmt;
	imageAspectFlags_t		aspect;
	imageTiling_t			tiling;
};


inline bool operator==( const imageInfo_t& info0, const imageInfo_t& info1 )
{
	bool equal =
		( info0.width == info1.width ) &&
		( info0.height == info1.height ) &&
		( info0.channels == info1.channels ) &&
		( info0.mipLevels == info1.mipLevels ) &&
		( info0.layers == info1.layers ) &&
		( info0.subsamples == info1.subsamples ) &&
		( info0.type == info1.type ) &&
		( info0.fmt == info1.fmt ) &&
		( info0.aspect == info1.aspect ) &&
		( info0.tiling == info1.tiling );
	return equal;
}


inline bool operator!=( const imageInfo_t& info0, const imageInfo_t& info1 )
{
	return !( info0 == info1 );
}


class Image
{
public:
	uint8_t*			bytes;
	uint32_t			sizeBytes;
	imageInfo_t			info;

	ImageBuffer<Color>	cpuImage;
	GpuImage*			gpuImage;

	Image()
	{
		info.width = 0;
		info.height = 0;
		info.channels = 0;
		info.mipLevels = 0;
		info.subsamples = IMAGE_SMP_1;
		info.type = IMAGE_TYPE_UNKNOWN;
		info.fmt = IMAGE_FMT_UNKNOWN;
		info.aspect = IMAGE_ASPECT_COLOR_FLAG;
		info.tiling = IMAGE_TILING_LINEAR;
		bytes = nullptr;
		sizeBytes = 0;

		gpuImage = nullptr;
	}

	~Image()
	{
		if( bytes != nullptr )
		{
			delete[] bytes;
			sizeBytes = 0;
			bytes = nullptr;
		}
	}
};


class ImageLoader : public LoadHandler<Image>
{
private:
	std::string basePath;
	std::string fileName;
	std::string ext;
	bool cubemap;

	bool Load( Image& texture )
	{
		if ( cubemap ) {
			return LoadCubeMapImage( ( basePath + fileName ).c_str(), ext.c_str(), texture );
		} else {
			return LoadImage( ( basePath + fileName + "." + ext ).c_str(), texture );
		}
	}

public:
	ImageLoader() : cubemap( false ) {}
	ImageLoader( const std::string& path, const std::string& file ) : cubemap( false )
	{
		SetBasePath( path );
		SetTextureFile( file );
	}

	void SetBasePath( const std::string& path )
	{
		basePath = path;
	}

	void SetTextureFile( const std::string& file )
	{
		const size_t extIndex = file.find_last_of( "." );
		ext = file.substr( extIndex + 1 );
		fileName = file.substr( 0, extIndex );
	}

	void LoadAsCubemap( const bool isCubemap )
	{
		cubemap = isCubemap;
	}
};

using pImgLoader_t = Asset<Image>::loadHandlerPtr_t;