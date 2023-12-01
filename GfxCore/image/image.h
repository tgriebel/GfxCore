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

template<typename T>
class ImageBuffer
{
private:
	static const uint32_t Version = 2;
	uint32_t	width;				// Width of image (highest mip)
	uint32_t	height;				// Height of image (highest mip)
	uint32_t	length;				// Number of elements in buffer
	uint32_t	layers;				// Depth for 3D volumes, sides for cubemaps, 1 normally
	uint32_t	bpp;				// Bytes per pixel
	uint8_t*	buffer = nullptr;
	const char*	name;

	void _Init( const uint32_t _width, const uint32_t _height, const uint32_t _layers = 1 )
	{
		if ( buffer != nullptr )
		{
			delete[] buffer;
			buffer = nullptr;
		}

		width = _width;
		height = _height;
		layers = _layers;
		bpp = sizeof( T );
		length = width * height * layers;
		buffer = new uint8_t[ bpp * length ];
	}

public:

	ImageBuffer()
	{
		width = 0;
		height = 0;
		length = 0;
		layers = 1;
		name = "";
		buffer = nullptr;
	}

	ImageBuffer( const uint32_t _width, const uint32_t _height, const uint32_t _layers, const T _default = T(), const char* _name = "" )
	{
		Init( _width, _height, _layers, _default, _name );
	}

	ImageBuffer( const ImageBuffer& _image )
	{
		if ( buffer != nullptr )
		{
			delete[] buffer;
		}
		buffer = new uint8_t[ _image.bpp * _image.length ];

		const uint32_t byteCount = _image.GetByteCount();
		for ( uint32_t i = 0; i < byteCount; ++i )
		{
			buffer[ i ] = _image.buffer[ i ];
		}

		width = _image.width;
		height = _image.height;
		length = _image.length;
		layers = _image.layers;
		bpp = _image.bpp;
		name = _image.name;
	}

	~ImageBuffer()
	{
		Destroy();
	}


	void Init( const uint32_t _width, const uint32_t _height, const char* _name = "", const bool clear = false )
	{
		_Init( _width, _height );
		name = _name;

		if( clear ) {
			Clear( T() );
		}
	}


	void Init( const uint32_t _width, const uint32_t _height, const uint32_t _layers, const char* _name = "", const bool clear = false )
	{
		_Init( _width, _height, _layers );
		name = _name;

		if ( clear ) {
			Clear( T() );
		}
	}

	void Init( const uint32_t _width, const uint32_t _height, const uint32_t _layers, const T& _default, const char* _name = "" )
	{
		_Init( _width, _height, _layers );
		name = _name;

		Clear( _default );
	}

	void Init( const uint32_t _width, const uint32_t _height, const uint32_t _layers, const T* data, const char* _name = "" )
	{
		_Init( _width, _height, _layers );
		name = _name;

		memcpy( buffer, data, GetByteCount() );
	}

	void Init( const uint32_t _width, const uint32_t _height, const T& _default, const char* _name = "" )
	{
		_Init( _width, _height );
		name = _name;

		Clear( _default );
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

	inline bool SetPixel( const int32_t x, const int32_t y, const T& pixel )
	{
		return SetPixel( x, y, 0, pixel );
	}

	bool SetPixel( const int32_t x, const int32_t y, const int32_t z, const T& pixel )
	{
		if ( ( x >= static_cast<int32_t>( width ) ) || ( x < 0 ) ) {
			return false;
		}
		
		if ( ( y >= static_cast<int32_t>( height ) ) || ( y < 0 ) ) {
			return false;
		}

		if ( ( z >= static_cast<int32_t>( layers ) ) || ( z < 0 ) ) {
			return false;
		}

		const uint32_t index = ( x + y * width ) + z * ( width * height );
		assert( index < length );

		if ( index >= length ) {
			return false;
		}

		RawBuffer()[ index ] = pixel;
		return true;
	}

	T GetPixel( const int32_t x, const int32_t y, const int32_t z = 0 ) const
	{
		if ( ( x >= static_cast<int32_t>( width ) ) || ( x < 0 ) ) {
			return T();
		}
		
		if ( ( y >= static_cast<int32_t>( height ) ) || ( y < 0 ) ) {
			return T();
		}

		if ( ( z >= static_cast<int32_t>( layers ) ) || ( z < 0 ) ) {
			return T();
		}

		const uint32_t index = ( x + y * width ) + z * ( width * height );
		assert( index < length );

		if ( index >= length ) {
			return T();
		}

		return RawBuffer()[ index ];
	}

	inline bool SetPixelUV( const float u, const float v, const T& pixel )
	{
		SetPixelUV( u, v, 0.0f, pixel );
	}

	bool SetPixelUV( const float u, const float v, const float w, const T& pixel )
	{
		WrapUV( u, v );

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
		for ( uint32_t i = 0; i < length; ++i )
		{
			RawBuffer()[ i ] = fill;
		}
	}

	inline const uint8_t* const Ptr() const
	{
		return buffer;
	}

	inline uint8_t* const Ptr()
	{
		return buffer;
	}

	inline const T* const RawBuffer() const
	{
		return reinterpret_cast<const T* const>( buffer );
	}

	inline T* const RawBuffer()
	{
		return reinterpret_cast<T*>( buffer );
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
		return height;
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

	static void WrapUV( float& u, float& v )
	{
		float w;
		WrapUV( u, v, w );
	}

	static void WrapUV( float& u, float& v, float& w )
	{
		u = ( u > 1.0 ) ? ( u - floor( u ) ) : u;
		v = ( v > 1.0 ) ? ( v - floor( v ) ) : v;
		w = ( w > 1.0 ) ? ( w - floor( v ) ) : w;

		u = Saturate( u );
		v = Saturate( v );
		w = Saturate( w );
	}

	void Serialize( Serializer* serializer );
};

template<class SourceType, class DestType>
void ImageAdaptor( const ImageBuffer<SourceType>& from, ImageBuffer<DestType>& to );