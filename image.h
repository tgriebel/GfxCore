#pragma once

#include <assert.h>
#include "common.h"

template<typename T>
class Image
{
public:

	Image()
	{
		width = 0;
		height = 0;
		length = 0;
		name = "";
		buffer = nullptr;
	}

	Image( const uint32_t _width, const uint32_t _height, const T _default = static_cast<T>( 0.0f ), const char* _name = "" )
	{
		name = _name;

		width = _width;
		height = _height;
		length = width * height;
		buffer = new T[ length ];

		Clear( _default );
	}

	Image( const Image& _image )
	{
		if ( buffer == nullptr )
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

	~Image()
	{
		width = 0;
		height = 0;
		length = 0;
		name = "";

		if( buffer == nullptr )
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

	inline uint32_t GetByteCnt() const
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

private:
	uint32_t	width;
	uint32_t	height;
	uint32_t	length;
	T*			buffer;
	const char*	name;
};