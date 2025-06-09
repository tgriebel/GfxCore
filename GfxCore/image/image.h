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

#include <assert.h>
#include <cstdint>
#include <algorithm>
#include "../core/common.h"

void WrapUV( float& u, float& v );
void WrapUV( float& u, float& v, float& w );

inline uint32_t MipCount( const uint32_t width, const uint32_t height )
{
	return static_cast<uint32_t>( std::floor( std::log2( std::max( width, height ) ) ) ) + 1;
}


inline void MipDimensions( const uint32_t level, const uint32_t baseWidth, const uint32_t baseHeight, uint32_t* levelWidth, uint32_t* levelHeight )
{
	if( ( levelWidth == nullptr ) || ( levelHeight == nullptr ) ) {
		return;
	}

	*levelWidth = ( baseWidth >> level );
	*levelHeight = ( baseHeight >> level );

	*levelWidth = ( *levelWidth == 0 ) ? 1 : *levelWidth;
	*levelHeight = ( *levelHeight == 0 ) ? 1 : *levelHeight;
}

class Serializer;

struct imageBufferInfo_t
{
	uint32_t		width;				// Width of image (highest mip)
	uint32_t		height;				// Height of image (highest mip)
	uint32_t		layers;				// Depth for 3D volumes, sides for cubemaps, 1 normally
	uint32_t		mipCount;			// Number of MIP levels, 1 normally
	uint32_t		bpp;				// Bytes per pixel
};

class ImageBufferInterface
{
private:
	static const uint32_t Version = 3;
	uint32_t		width;				// Width of image (highest mip)
	uint32_t		height;				// Height of image (highest mip)
	uint32_t		length;				// Number of elements in buffer
	uint32_t		layers;				// Depth for 3D volumes, sides for cubemaps, 1 normally
	uint32_t		bpp;				// Bytes per pixel
	uint8_t*		buffer = nullptr;
	const char*		name;

protected:
	void _Init( const imageBufferInfo_t& _info, const char* _name = "" )
	{
		if ( buffer != nullptr )
		{
			delete[] buffer;
			buffer = nullptr;
		}

		name = _name;

		width = _info.width;
		height = _info.height;
		layers = _info.layers;
		bpp = _info.bpp;
		length = width * height * layers;

		buffer = new uint8_t[ bpp * length ];
	}

public:
	ImageBufferInterface()
	{
		width = 0;
		height = 0;
		length = 0;
		layers = 1;
		name = "";
		buffer = nullptr;
	}

	ImageBufferInterface( const ImageBufferInterface* _image )
	{
		if ( buffer != nullptr ) {
			delete[] buffer;
		}

		const uint32_t byteCount = _image->GetByteCount();
		buffer = new uint8_t[ byteCount ];

		const uint8_t* srcBuffer = _image->Ptr();
		for ( uint32_t i = 0; i < byteCount; ++i ) {
			buffer[ i ] = srcBuffer[ i ];
		}

		width = _image->GetWidth();
		height = _image->GetHeight();
		length = _image->GetPixelCount();
		layers = _image->GetLayers();
		bpp = _image->GetBpp();
		name = _image->GetName();
	}

	void Init( const uint32_t _width, const uint32_t _height, const uint32_t _bpp, const char* _name = "", const bool clear = false )
	{
		imageBufferInfo_t info {};
		info.width = _width;
		info.height = _height;
		info.layers = 1;
		info.mipCount = 1;
		info.bpp = _bpp;

		_Init( info, _name );

		if ( clear ) {
			Clear();
		}
	}


	void Init( const imageBufferInfo_t& _info, const char* _name = "", const bool clear = false )
	{
		_Init( _info, _name );

		if ( clear ) {
			Clear();
		}
	}

	void Destroy()
	{
		width = 0;
		height = 0;
		length = 0;
		layers = 1;
		name = "";

		if ( buffer == nullptr )
		{
			delete[] buffer;
		}
	}

	void Clear()
	{
		memset( buffer, 0, GetByteCount() );
	}

	inline const uint8_t* const Ptr() const
	{
		return buffer;
	}

	inline uint8_t* const Ptr()
	{
		return buffer;
	}

	inline uint8_t* GetLayerPtr( const uint32_t layer ) const
	{
		uint64_t offset = layer;
		offset *= uint64_t( width ) * uint64_t( height );
		return buffer + offset;
	}

	inline uint32_t GetWidth() const
	{
		return width;
	}

	inline uint32_t GetHeight() const
	{
		return height;
	}

	inline uint32_t GetLayers() const
	{
		return layers;
	}

	inline uint32_t GetBpp() const
	{
		return bpp;
	}

	inline uint32_t GetPixelCount() const
	{
		return length;
	}

	inline uint32_t GetByteCount() const
	{
		return length * bpp;
	}

	inline const char* GetName() const
	{
		return name;
	}

	void Serialize( Serializer* s );
};

template<typename T>
class ImageBuffer : public ImageBufferInterface
{
public:

	ImageBuffer() : ImageBufferInterface()
	{
	}

	ImageBuffer( const uint32_t _width, const uint32_t _height, const uint32_t _layers, const T _default = T(), const char* _name = "" )
	{
		imageBufferInfo_t info{};
		info.width = _width;
		info.height = _height;
		info.layers = _layers;
		info.mipCount = 1;
		info.bpp = sizeof( T );

		_Init( info, _name );
	}

	ImageBuffer( const ImageBuffer& _image ) : ImageBufferInterface( reinterpret_cast<const ImageBufferInterface*>( &_image ) )
	{
	}

	~ImageBuffer()
	{
		Destroy();
	}

	void Init( const uint32_t _width, const uint32_t _height, const char* _name = "" )
	{
		imageBufferInfo_t info{};
		info.width = _width;
		info.height = _height;
		info.layers = 1;
		info.mipCount = 1;
		info.bpp = sizeof( T );

		_Init( info, _name );
	}

	void Init( const uint32_t _width, const uint32_t _height, const uint32_t _layers, const T& _default, const char* _name = "" )
	{
		imageBufferInfo_t info{};
		info.width = _width;
		info.height = _height;
		info.layers = _layers;
		info.mipCount = 1;
		info.bpp = sizeof( T );

		_Init( info, _name );
		Clear( _default );
	}

	void Init( const uint32_t _width, const uint32_t _height, const uint32_t _layers, const T* data, const char* _name = "" )
	{
		imageBufferInfo_t info{};
		info.width = _width;
		info.height = _height;
		info.layers = _layers;
		info.mipCount = 1;
		info.bpp = sizeof( T );

		_Init( info, _name );
		memcpy( Ptr(), data, GetByteCount() );
	}

	void Init( const uint32_t _width, const uint32_t _height, const T& _default, const char* _name = "" )
	{
		imageBufferInfo_t info{};
		info.width = _width;
		info.height = _height;
		info.layers = 1;
		info.mipCount = 1;
		info.bpp = sizeof( T );

		_Init( info, _name );
		Clear( _default );
	}

	inline bool SetPixel( const int32_t x, const int32_t y, const T& pixel )
	{
		return SetPixel( x, y, 0, pixel );
	}

	bool SetPixel( const int32_t x, const int32_t y, const int32_t z, const T& pixel )
	{
		if ( ( x >= static_cast<int32_t>( GetWidth() ) ) || ( x < 0 ) ) {
			return false;
		}
		
		if ( ( y >= static_cast<int32_t>( GetHeight() ) ) || ( y < 0 ) ) {
			return false;
		}

		if ( ( z >= static_cast<int32_t>( GetLayers() ) ) || ( z < 0 ) ) {
			return false;
		}

		const uint32_t index = ( x + y * GetWidth() ) + z * ( GetWidth() * GetHeight() );
		assert( index < GetPixelCount() );

		if ( index >= GetPixelCount() ) {
			return false;
		}

		RawBuffer()[ index ] = pixel;
		return true;
	}

	T GetPixel( const int32_t x, const int32_t y, const int32_t z = 0 ) const
	{
		if ( ( x >= static_cast<int32_t>( GetWidth() ) ) || ( x < 0 ) ) {
			return T();
		}
		
		if ( ( y >= static_cast<int32_t>( GetHeight() ) ) || ( y < 0 ) ) {
			return T();
		}

		if ( ( z >= static_cast<int32_t>( GetLayers() ) ) || ( z < 0 ) ) {
			return T();
		}

		const uint32_t index = ( x + y * GetWidth() ) + z * ( GetWidth() * GetHeight() );
		assert( index < GetPixelCount() );

		if ( index >= GetPixelCount() ) {
			return T();
		}

		return RawBuffer()[ index ];
	}

	inline bool SetPixelUV( float u, float v, const T& pixel )
	{
		SetPixelUV( u, v, 0.0f, pixel );
	}

	bool SetPixelUV( float u, float v, float w, const T& pixel )
	{
		WrapUV( u, v, w );

		const uint32_t x = static_cast<uint32_t>( u * GetWidth() );
		const uint32_t y = static_cast<uint32_t>( v * GetWidth() );
		const uint32_t z = static_cast<uint32_t>( w * GetWidth() );

		return SetPixel( pixel, x, y, z );
	}

	T GetPixelUV( float u, float v, float w = 0.0f ) const
	{
		WrapUV( u, v, w );

		const uint32_t x = static_cast<uint32_t>( u * GetWidth() );
		const uint32_t y = static_cast<uint32_t>( v * GetHeight() );
		const uint32_t z = static_cast<uint32_t>( w * GetWidth() );

		return GetPixel( x, y, z );
	}

	void Clear( const T& fill )
	{
		const uint32_t pixelCount = GetPixelCount();
		for ( uint32_t i = 0; i < pixelCount; ++i ) {
			RawBuffer()[ i ] = fill;
		}
	}

	inline const T* const RawBuffer() const
	{
		return reinterpret_cast<const T* const>( Ptr() );
	}

	inline T* const RawBuffer()
	{
		return reinterpret_cast<T*>( Ptr() );
	}

	void Serialize( Serializer* serializer );
};

template<class SourceType, class DestType>
void ImageAdaptor( const ImageBuffer<SourceType>& from, ImageBuffer<DestType>& to );