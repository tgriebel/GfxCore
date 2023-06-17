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
#include <systemUtils.h>
#include <serializer.h>
#include <common.h>
#include "../core/assetLib.h"
#include "../io/serializeClasses.h"

void Image::InitCpuImage()
{
	cpuImage.Init( info.width, info.height );
	for ( uint32_t py = 0; py < info.height; ++py )
	{
		for ( uint32_t px = 0; px < info.width; ++px )
		{
			RGBA rgba;
			rgba.r = bytes[ ( py * info.width + px ) * 4 + 0 ];
			rgba.g = bytes[ ( py * info.width + px ) * 4 + 1 ];
			rgba.b = bytes[ ( py * info.width + px ) * 4 + 2 ];
			rgba.a = bytes[ ( py * info.width + px ) * 4 + 3 ];
			cpuImage.SetPixel( px, py, rgba );
		}
	}
}


void Image::DestroyCpuImage()
{
	cpuImage.Destroy();
}


void Image::Serialize( Serializer* s )
{
	uint32_t version = Version;
	s->Next( version );
	if ( version != Version ) {
		throw std::runtime_error( "Wrong version number." );
	}

	cpuImage.Serialize( s );

	SerializeStruct( s, info );

	if ( s->GetMode() == serializeMode_t::LOAD )
	{
		sizeBytes = cpuImage.GetByteCount();
		bytes = new uint8_t[ sizeBytes ];

		uint32_t pixelIx = 0;
		for( uint32_t i = 0; i < sizeBytes; i += 4 )
		{
			const RGBA& rgba = cpuImage.Ptr()[ pixelIx ];
			bytes[ i + 0 ] = rgba.r;
			bytes[ i + 1 ] = rgba.g;
			bytes[ i + 2 ] = rgba.b;
			bytes[ i + 3 ] = rgba.a;
			++pixelIx;
		}
	}
}


bool ImageLoader::Load( Image& texture )
{
	const hdl_t handle = Library::Handle( ( fileName + "." + ext ).c_str() );
	const std::string hash = handle.String();
	const std::string bakedPath = ".\\baked\\" + basePath + hash + ".img.bin";
	if ( FileExists( bakedPath ) )
	{
		Serializer* s = new Serializer( MB( 128 ), serializeMode_t::LOAD );
		s->ReadFile( bakedPath );
		texture.Serialize( s );
		const bool loaded = ( s->Status() == serializeStatus_t::OK );
		assert( loaded );
		delete s;
		return loaded;
	}

	if ( cubemap ) {
		return LoadCubeMapImage( ( basePath + fileName ).c_str(), ext.c_str(), texture );
	} else {
		return LoadImage( ( basePath + fileName + "." + ext ).c_str(), texture );
	}
}


void ImageLoader::SetBasePath( const std::string& path )
{
	basePath = path;
}


void ImageLoader::SetTextureFile( const std::string& file )
{
	const size_t extIndex = file.find_last_of( "." );
	ext = file.substr( extIndex + 1 );
	fileName = file.substr( 0, extIndex );
}


void ImageLoader::LoadAsCubemap( const bool isCubemap )
{
	cubemap = isCubemap;
}