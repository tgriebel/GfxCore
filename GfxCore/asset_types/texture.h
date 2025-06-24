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
	IMAGE_ASPECT_NONE = 0,
	IMAGE_ASPECT_COLOR_FLAG = ( 1 << 0 ),
	IMAGE_ASPECT_DEPTH_FLAG = ( 1 << 1 ),
	IMAGE_ASPECT_STENCIL_FLAG = ( 1 << 2 ),
	IMAGE_ASPECT_ALL = ( 1 << 3 ) - 1
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
	IMAGE_FMT_RGBA_8_UNORM,
	IMAGE_FMT_ABGR_8,
	IMAGE_FMT_BGR_8,
	IMAGE_FMT_BGRA_8,
	IMAGE_FMT_RG_32,
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
	bool					unused; // Deprecated v2
};


enum imageCubeFace : uint8_t
{
	IMAGE_CUBE_FACE_X_POS,
	IMAGE_CUBE_FACE_X_NEG,
	IMAGE_CUBE_FACE_Y_POS,
	IMAGE_CUBE_FACE_Y_NEG,
	IMAGE_CUBE_FACE_Z_POS,
	IMAGE_CUBE_FACE_Z_NEG,
};


struct imageSubResourceView_t
{
	uint32_t baseMip;
	uint32_t mipLevels;
	uint32_t baseArray;
	uint32_t arrayCount;
};


enum samplerAddress_t
{
	SAMPLER_ADDRESS_WRAP = 0,
	SAMPLER_ADDRESS_CLAMP_EDGE = 1,
	SAMPLER_ADDRESS_CLAMP_BORDER = 2,
	SAMPLER_ADDRESS_MODES,
};


enum samplerFilter_t
{
	SAMPLER_FILTER_NEAREST = 0,
	SAMPLER_FILTER_BILINEAR = 1,
	SAMPLER_FILTER_TRILINEAR = 2,
	SAMPLER_FILTER_MODES,
};


struct samplerState_t
{
	samplerAddress_t	addrMode;
	samplerFilter_t		filter;
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


inline imageInfo_t DefaultImage2dInfo( uint32_t w, uint32_t h )
{
	imageInfo_t info {};
	info.width = w;
	info.height = h;
	info.layers = 1;
	info.channels = 4;
	info.mipLevels = MipCount( w, h );
	info.subsamples = IMAGE_SMP_1;
	info.type = IMAGE_TYPE_2D;
	info.fmt = IMAGE_FMT_RGBA_8;
	info.aspect = IMAGE_ASPECT_COLOR_FLAG;
	info.tiling = IMAGE_TILING_MORTON;

	return info;
}


class Image
{
private:
	static const uint32_t Version = 2;
public:
	imageInfo_t				info;
	imageSubResourceView_t	subResourceView;
	samplerState_t			sampler;
	bool					generateMips;

	ImageBufferInterface*	cpuImage;
	GpuImage*				gpuImage;

	Image()
	{
		info = DefaultImage2dInfo( 1, 1 );

		subResourceView.baseArray = 0;
		subResourceView.arrayCount = 1;
		subResourceView.baseMip = 0;
		subResourceView.mipLevels = 1;

		generateMips = true;

		sampler.addrMode = SAMPLER_ADDRESS_WRAP;
		sampler.filter = SAMPLER_FILTER_BILINEAR;

		cpuImage = nullptr;
		gpuImage = nullptr;
	}

	Image( const imageInfo_t& _info ) : Image( _info, nullptr, nullptr ) {}

	Image( const imageInfo_t& _info, ImageBufferInterface* _cpuImage, GpuImage* _gpuImage )
	{
		Create( _info, _cpuImage, _gpuImage );
	}

	~Image()
	{
		Destroy();
	}

	void Create( const imageInfo_t& _info )
	{
		Create( _info, nullptr, 0u );
	}

	void Create( const imageInfo_t& _info, uint8_t* pixelBytes, const uint32_t byteCount );

	void Create( const imageInfo_t& _info, ImageBufferInterface* _cpuImage, GpuImage* _gpuImage );

	void Destroy();

	void Serialize( Serializer* serializer );
};


class ImageLoader : public LoadHandler<Image>
{
private:
	std::string		m_basePath;
	std::string		m_fileName;
	std::string		m_ext;
	bool			m_hdr;
	bool			m_cubemap;
	samplerState_t	m_sampler;

	bool Load( Asset<Image>& texture );

public:
	ImageLoader() : m_cubemap( false ), m_hdr( false ) {}
	ImageLoader( const std::string& path, const std::string& file ) : m_cubemap( false ), m_hdr( false )
	{
		SetBasePath( path );
		SetTextureFile( file );
	}

	void SetSampler( const samplerState_t& sampler );
	void SetBasePath( const std::string& path );
	void SetTextureFile( const std::string& file );
	void LoadAsCubemap( const bool isCubemap );
};

class BakedImageLoader : public LoadHandler<Image>
{
private:
	std::string m_basePath;
	std::string m_fileName;
	std::string m_ext;

	bool Load( Asset<Image>& texture );

public:
	BakedImageLoader() {}
	BakedImageLoader( const std::string& path, const std::string& ext )
	{
		SetBasePath( path );
		SetFileExt( ext );
	}

	void SetBasePath( const std::string& path );
	void SetFileExt( const std::string& ext );
};

using pImgLoader_t = Asset<Image>::loadHandlerPtr_t;