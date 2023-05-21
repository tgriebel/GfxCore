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

enum textureType_t : uint8_t
{
	TEXTURE_TYPE_UNKNOWN,
	TEXTURE_TYPE_2D,
	TEXTURE_TYPE_2D_ARRAY,
	TEXTURE_TYPE_3D,
	TEXTURE_TYPE_3D_ARRAY,
	TEXTURE_TYPE_CUBE,
	TEXTURE_TYPE_CUBE_ARRAY,
	TEXTURE_TYPE_DEPTH,
	TEXTURE_TYPE_STENCIL,
	TEXTURE_TYPE_DEPTH_STENCIL,
};

enum textureTiling_t : uint8_t
{
	TEXTURE_TILING_LINEAR,
	TEXTURE_TILING_MORTON,
};

enum textureFmt_t : uint8_t
{
	TEXTURE_FMT_UNKNOWN,
	TEXTURE_FMT_R_8,
	TEXTURE_FMT_R_16,
	TEXTURE_FMT_R_32,
	TEXTURE_FMT_D_16,
	TEXTURE_FMT_D24S8,
	TEXTURE_FMT_D_32,
	TEXTURE_FMT_D_32_S8,
	TEXTURE_FMT_RGB_8,
	TEXTURE_FMT_RGBA_8,
	TEXTURE_FMT_ABGR_8,
	TEXTURE_FMT_BGR_8,
	TEXTURE_FMT_BGRA_8,
	TEXTURE_FMT_RGB_16,
	TEXTURE_FMT_RGBA_16,
	TEXTURE_FMT_R11G11B10,
};

enum textureSamples_t : uint8_t
{
	TEXTURE_SMP_1 = (1 << 0),
	TEXTURE_SMP_2 = ( 1 << 1 ),
	TEXTURE_SMP_4 = ( 1 << 2 ),
	TEXTURE_SMP_8 = ( 1 << 3 ),
	TEXTURE_SMP_16 = ( 1 << 4 ),
	TEXTURE_SMP_32 = ( 1 << 5 ),
	TEXTURE_SMP_64 = ( 1 << 6 ),
};

struct textureInfo_t
{
	uint32_t			width;
	uint32_t			height;
	uint32_t			channels;
	uint32_t			mipLevels;
	uint32_t			layers;
	textureSamples_t	subsamples;
	textureType_t		type;
	textureFmt_t		fmt;
	textureTiling_t		tiling;
};

class Texture
{
public:
	uint8_t*		bytes;
	uint32_t		sizeBytes;
	textureInfo_t	info;
	int				uploadId;
	bool			dirty;

	Image<Color>	cpuImage;
	GpuImage*		gpuImage;

	Texture()
	{
		info.width = 0;
		info.height = 0;
		info.channels = 0;
		info.mipLevels = 0;
		info.subsamples = TEXTURE_SMP_1;
		info.type = TEXTURE_TYPE_UNKNOWN;
		info.fmt = TEXTURE_FMT_UNKNOWN;
		info.tiling = TEXTURE_TILING_LINEAR;
		uploadId = -1;
		bytes = nullptr;
		sizeBytes = 0;
		dirty = false;
	}

	~Texture()
	{
		if( bytes != nullptr )
		{
			delete[] bytes;
			sizeBytes = 0;
			bytes = nullptr;
		}
	}
};


class TextureLoader : public LoadHandler<Texture>
{
private:
	std::string basePath;
	std::string fileName;
	std::string ext;
	bool cubemap;

	bool Load( Texture& texture )
	{
		if ( cubemap ) {
			return LoadTextureCubeMapImage( ( basePath + fileName ).c_str(), ext.c_str(), texture );
		} else {
			return LoadTextureImage( ( basePath + fileName + "." + ext ).c_str(), texture );
		}
	}

public:
	TextureLoader() : cubemap( false ) {}
	TextureLoader( const std::string& path, const std::string& file ) : cubemap( false )
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

using pTexLoader_t = Asset<Texture>::loadHandlerPtr_t;