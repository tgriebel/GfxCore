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

#include "texture.h"
#include <syscore/systemUtils.h>
#include <syscore/serializer.h>
#include <syscore/common.h>
#include "../core/assetLib.h"
#include "../io/serializeClasses.h"


void Image::Create( const imageInfo_t& _info, uint8_t* pixelBytes, const uint32_t byteCount )
{
	info = _info;
	info.layers = ( _info.type == IMAGE_TYPE_CUBE ) ? 6 : _info.layers;

	subResourceView.arrayCount = info.layers;
	subResourceView.mipLevels = info.mipLevels;

	generateMips = true;

	assert( cpuImage == nullptr );

	imageBufferInfo_t bufferInfo{};
	bufferInfo.width = _info.width;
	bufferInfo.height = _info.height;
	bufferInfo.layers = _info.layers;
	bufferInfo.mipCount = _info.mipLevels;
	bufferInfo.data = pixelBytes;
	bufferInfo.dataByteCount = byteCount;

	switch ( info.fmt )
	{
		case IMAGE_FMT_R_8:
		{
			cpuImage = new ImageBuffer<uint8_t>( bufferInfo );
		} break;
		case IMAGE_FMT_D_16:
		case IMAGE_FMT_R_16:
		{
			cpuImage = new ImageBuffer<uint16_t>( bufferInfo );
		} break;
		case IMAGE_FMT_D_32:
		case IMAGE_FMT_R_32:
		{
			cpuImage = new ImageBuffer<float>( bufferInfo );
		} break;
		case IMAGE_FMT_RGB_8:
		{
			cpuImage = new ImageBuffer<rgb8_t>( bufferInfo );
		} break;
		case IMAGE_FMT_RGBA_8:
		case IMAGE_FMT_RGBA_8_UNORM:
		{
			cpuImage = new ImageBuffer<rgba8_t>( bufferInfo );
		} break;
		case IMAGE_FMT_RGB_16:
		{
			cpuImage = new ImageBuffer<rgb16_t>( bufferInfo );
		} break;
		case IMAGE_FMT_RGBA_16:
		{
			cpuImage = new ImageBuffer<rgba16_t>( bufferInfo );
		} break;
		default: assert( 0 );
	}
}


void Image::Create( const imageInfo_t& _info, ImageBufferInterface* _cpuImage, GpuImage* _gpuImage )
{
	info = _info;
	info.layers = ( _info.type == IMAGE_TYPE_CUBE ) ? 6 : _info.layers;

	subResourceView.baseArray = 0;
	subResourceView.arrayCount = info.layers;
	subResourceView.baseMip = 0;
	subResourceView.mipLevels = info.mipLevels;

	generateMips = true;

	sampler.addrMode = SAMPLER_ADDRESS_WRAP;
	sampler.filter = SAMPLER_FILTER_BILINEAR;

	cpuImage = _cpuImage;
	gpuImage = _gpuImage;
}


void Image::Destroy()
{
	if ( cpuImage != nullptr )
	{
		delete cpuImage;
		cpuImage = nullptr;
	}
}


void Image::Serialize( Serializer* s )
{
	uint32_t version = Version;
	s->Next( version );

	SerializeStruct( s, info );

	if ( s->GetMode() == serializeMode_t::LOAD ) {
		Create( info );
	}

	if ( version == 2 ) {
		s->Next( generateMips );
	}

	cpuImage->Serialize( s );
}


bool ImageLoader::Load( Asset<Image>& imageAsset )
{
	Image& image = imageAsset.Get();

	image.sampler = m_sampler;

	bakedAssetInfo_t info = {};
	
	const bool loadedBaked = LoadBaked( imageAsset, info, ".\\baked\\" + m_basePath, "img.bin" );
	if ( loadedBaked ) {
		return true;
	}

	if( m_ext == "img" ) {
		Serializer s( MB(32), serializeMode_t::LOAD );

		const std::string path = m_basePath + m_fileName + ".img";
		if ( FileExists( path ) == false ) {
			return false;
		}
		s.ReadFile( path );

		image.Serialize(&s);

		return ( s.Status() == serializeStatus_t::OK );
	}

	if ( m_cubemap ) {
		return LoadCubeMapImage( ( m_basePath + m_fileName ).c_str(), m_ext.c_str(), image );
	} else {
		if( m_hdr ) {
			return LoadImageHDR( ( m_basePath + m_fileName + "." + m_ext ).c_str(), image );			
		} else {
			return LoadImage( ( m_basePath + m_fileName + "." + m_ext ).c_str(), m_linearColor, image );
		}
	}
}


void ImageLoader::SetSampler( const samplerState_t& sampler )
{
	m_sampler = sampler;
}


void ImageLoader::SetBasePath( const std::string& path )
{
	m_basePath = path;
}


void ImageLoader::SetTextureFile( const std::string& file )
{
	SplitFileName( file, m_fileName, m_ext );

	if( m_ext == "hdr" ) {
		m_hdr = true;
	}
}


void ImageLoader::LoadAsCubemap( const bool isCubemap )
{
	m_cubemap = isCubemap;
}


void ImageLoader::LoadAsLinear( const bool isLinear )
{
	m_linearColor = isLinear;
}


bool BakedImageLoader::Load( Asset<Image>& imageAsset )
{
	Image& image = imageAsset.Get();

	bakedAssetInfo_t info = {};
	const bool loadedBaked = LoadBaked( imageAsset, info, m_basePath, m_ext );
	if ( loadedBaked ) {
		return true;
	}
	return false;
}


void BakedImageLoader::SetBasePath( const std::string& path )
{
	m_basePath = path;
}


void BakedImageLoader::SetFileExt( const std::string& ext )
{
	m_ext = ext;
}