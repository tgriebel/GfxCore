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
#include "../core/common.h"

class Serializer;

template<typename T>
class ImageBuffer
{
private:
	static const uint32_t Version = 1;
	uint32_t	width;
	uint32_t	height;
	uint32_t	length;
	T*			buffer = nullptr;
	const char*	name;

	void _Init( const uint32_t _width, const uint32_t _height )
	{
		if ( buffer != nullptr )
		{
			delete[] buffer;
			buffer = nullptr;
		}

		width = _width;
		height = _height;
		length = width * height;
		buffer = new T[ length ];
	}

public:

	ImageBuffer()
	{
		width = 0;
		height = 0;
		length = 0;
		name = "";
		buffer = nullptr;
	}

	ImageBuffer( const uint32_t _width, const uint32_t _height, const T _default = T(), const char* _name = "" )
	{
		Init( _width, _height, _default, _name );
	}

	ImageBuffer( const ImageBuffer& _image )
	{
		if ( buffer != nullptr )
		{
			delete[] buffer;
		}
		buffer = new T[ _image.length ];

		for ( uint32_t i = 0; i < _image.length; ++i )
		{
			buffer[ i ] = _image.buffer[ i ];
		}

		width = _image.width;
		height = _image.height;
		length = _image.length;
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
		name = "";

		if ( buffer == nullptr )
		{
			delete[] buffer;
		}
	}


	bool SetPixel( const int32_t x, const int32_t y, const T& pixel )
	{
		if ( ( x >= static_cast<int32_t>( width ) ) || ( x < 0 ) )
			return false;
		
		if ( ( y >= static_cast<int32_t>( height ) ) || ( y < 0 ) )
			return false;

		const uint32_t index = ( x + y * width );
		assert( index < length );

		if ( index >= length )
			return false;

		buffer[ index ] = pixel;
		return true;
	}

	T GetPixel( const int32_t x, const int32_t y ) const
	{
		if ( ( x >= static_cast<int32_t>( width ) ) || ( x < 0 ) )
			return T();
		
		if ( ( y >= static_cast<int32_t>( height ) ) || ( y < 0 ) )
			return T();

		const uint32_t index = ( x + y * width );
		assert( index < length );

		if ( index >= length )
			return T();

		return buffer[ index ];
	}

	bool SetPixelUV( float u, float v, const T& pixel )
	{
		WrapUV( u, v );

		const uint32_t x = static_cast<uint32_t>( u * GetWidth() );
		const uint32_t y = static_cast<uint32_t>( v * GetWidth() );

		return SetPixel( x, y, pixel );
	}

	T GetPixelUV( float u, float v ) const
	{
		WrapUV( u, v );

		const uint32_t x = static_cast<uint32_t>( u * GetWidth() );
		const uint32_t y = static_cast<uint32_t>( v * GetHeight() );

		return GetPixel( x, y );
	}

	void Clear( const T& fill )
	{
		for ( uint32_t i = 0; i < length; ++i )
		{
			buffer[ i ] = fill;
		}
	}

	inline const T* const GetRawBuffer() const
	{
		return &buffer[ 0 ];
	}

	inline uint32_t GetWidth() const
	{
		return width;
	}

	inline uint32_t GetHeight() const
	{
		return height;
	}

	inline uint32_t GetByteCount() const
	{
		return length * sizeof( T );
	}

	inline const char* GetName() const
	{
		return name;
	}

	static void WrapUV( float& u, float& v )
	{
		u = ( u > 1.0 ) ? ( u - floor( u ) ) : u;
		v = ( v > 1.0 ) ? ( v - floor( v ) ) : v;

		u = Saturate( u );
		v = Saturate( v );
	}

	void Serialize( Serializer* serializer );
};