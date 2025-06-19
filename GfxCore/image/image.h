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


inline uint32_t MipPixelCount( const uint32_t width, const uint32_t height )
{
	const uint32_t highestMipPixelCount = RoundPow2( width ) * RoundPow2( height );
	uint32_t mipChainPixelCount = 0;
	if ( ( highestMipPixelCount & 0xAAAAAAAB ) != 0 ) {
		mipChainPixelCount = ( highestMipPixelCount - 1 ) & 0xAAAAAAAB; // All squared power of 2's aka alternating bits + 0-bit for 1x1
	} else {
		mipChainPixelCount = ( highestMipPixelCount - 1 ) & 0x55555555; // Alternating bits
	}
	return ( highestMipPixelCount | mipChainPixelCount );
}


inline uint32_t MipCount( const uint32_t width, const uint32_t height )
{
	return static_cast<uint32_t>( std::floor( std::log2( Max( width, height ) ) ) ) + 1;
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
	void*			data;				// Initialization data
	uint32_t		dataByteCount;
};

// References a MxN image within a buffer
struct slice_t
{
	uint32_t	offset;
	uint32_t	size;
	uint8_t*	ptr;
	uint8_t*	end;
};

class ImageBufferInterface
{
private:
	static const uint32_t Version = 4;
	uint32_t		width;				// Width of image (highest mip)
	uint32_t		height;				// Height of image (highest mip)
	uint32_t		length;				// Number of elements in buffer
	uint32_t		layers;				// Depth for 3D volumes, sides for cubemaps, 1 normally
	uint32_t		mipCount;			// Number of mips in buffer, 1 normally
	uint32_t		bpp;				// Bytes per pixel
	uint32_t		byteCount;			// Bytes of buffer
	uint32_t		sliceCount;			// Number of MxN image slices
	slice_t*		slices = nullptr;	// Buffer needs to be continuous data, this helps index into that pool in a structured way
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
		if ( slices != nullptr )
		{
			delete[] slices;
			slices = nullptr;
		}

		const bool clearbuffers = true;

		name = _name;

		width = _info.width;
		height = _info.height;
		layers = _info.layers;
		mipCount = _info.mipCount;
		bpp = _info.bpp;
		length = width * height * layers;
		sliceCount = layers * mipCount;

		// 1. Compute partitions
		slices = new slice_t[ sliceCount ];
		if( clearbuffers ) {
			memset( slices, 0, sliceCount * sizeof( slice_t ) );
		}

		byteCount = 0;
		for ( uint32_t mip = 0; mip < mipCount; ++mip )
		{
			const uint32_t mipOffset = mip * layers;
			for( uint32_t layerId = 0; layerId < layers; ++layerId )
			{
				slice_t& slice = slices[ layerId + mipOffset ];
				const uint32_t size = width * height * bpp;

				slice.offset = byteCount;	
				slice.size = size;

				byteCount += size;
			}
		}

		// 2. Allocate
		buffer = new uint8_t[ byteCount ];
		if ( clearbuffers ) {
			memset( buffer, 0, byteCount );
		}

		// 3. Create convenience pointers
		for ( uint32_t mip = 0; mip < mipCount; ++mip )
		{
			const uint32_t mipOffset = mip * layers;
			for ( uint32_t layerId = 0; layerId < layers; ++layerId )
			{
				slice_t& slice = slices[ layerId + mipOffset ];
				slice.ptr = buffer + slice.offset;
				slice.end = slice.ptr + slice.size;
			}
		}
	}

public:
	ImageBufferInterface()
	{
		width = 0;
		height = 0;
		length = 0;
		layers = 1;
		mipCount = 1;
		bpp = 0;
		sliceCount = 0;
		byteCount = 0;
		
		name = "";
		buffer = nullptr;
		slices = nullptr;
	}

	ImageBufferInterface( const ImageBufferInterface* _image )
	{
		width = _image->width;
		height = _image->height;
		length = _image->length;
		layers = _image->layers;
		mipCount = _image->mipCount;
		bpp = _image->bpp;
		name = _image->name;
		byteCount = _image->byteCount;
		sliceCount = _image->sliceCount;

		buffer = new uint8_t[ byteCount ];
		slices = new slice_t[ sliceCount ];

		memcpy( buffer, _image->buffer, byteCount );
		memcpy( slices, _image->slices, sliceCount * sizeof( slice_t ) );
	}

	// Child classes don't manage any data so not virtual
	~ImageBufferInterface() {
		Destroy();
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
		mipCount = 1;
		byteCount = 0;
		name = "";

		if ( buffer == nullptr ) {
			delete[] buffer;
		}
		if ( slices == nullptr ) {
			delete[] slices;
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

	inline const slice_t GetSlice( const uint32_t layer, const uint32_t mipLevel ) const
	{
		uint32_t offset = 0;
		offset += layer < layers ? layer : layers - 1;
		offset += ( mipLevel < mipCount ? mipLevel : ( mipCount - 1 ) ) * layers;

		slice_t& slice = slices[ offset ];
		return slice;
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

	inline uint32_t GetMipCount() const
	{
		return mipCount;
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
		return byteCount;
	}

	inline const char* GetName() const
	{
		return name;
	}

	void Serialize( Serializer* s );
};

template<typename T>
struct imageRawBuffer_t
{
	uint32_t	pixelCount;
	T*			ptr;
};

// TODO: Figure out better class names since the child class is strictly a view of the data
template<typename T>
class ImageBuffer : public ImageBufferInterface
{
public:

	ImageBuffer() : ImageBufferInterface()
	{
	}

	ImageBuffer( const imageBufferInfo_t& _info, const char* _name = "" )
	{
		imageBufferInfo_t info = _info;
		info.bpp = sizeof( T );

		_Init( info, _name );

		if( ( info.data != nullptr ) && ( info.dataByteCount > 0 ) )
		{
			if ( info.dataByteCount == info.bpp ) {
				Clear( *reinterpret_cast<T*>( info.data ) );
			} else {
				memcpy( Ptr(), info.data, GetByteCount() );
			}
		} else {
			Clear( T() );
		}
	}

	ImageBuffer( const uint32_t _width, const uint32_t _height, const char* _name = "" )
	{
		imageBufferInfo_t info{};
		info.width = _width;
		info.height = _height;
		info.layers = 1;
		info.mipCount = 1;
		info.bpp = sizeof( T );
		info.data = nullptr;
		info.dataByteCount = 0;

		_Init( info, _name );
	}

	ImageBuffer( const uint32_t _width, const uint32_t _height, const uint32_t _layers, const T _default = T(), const char* _name = "" )
	{
		imageBufferInfo_t info{};
		info.width = _width;
		info.height = _height;
		info.layers = _layers;
		info.mipCount = 1;
		info.bpp = sizeof( T );
		info.data = nullptr;
		info.dataByteCount = 0;

		_Init( info, _name );
		Clear( _default );
	}

	ImageBuffer( const uint32_t _width, const uint32_t _height, const uint32_t _layers, const T* data, const char* _name = "" )
	{
		imageBufferInfo_t info{};
		info.width = _width;
		info.height = _height;
		info.layers = _layers;
		info.mipCount = 1;
		info.bpp = sizeof( T );
		info.data = nullptr;
		info.dataByteCount = 0;

		_Init( info, _name );
		memcpy( Ptr(), data, GetByteCount() );
	}

	ImageBuffer( const ImageBuffer& _image ) : ImageBufferInterface( reinterpret_cast<const ImageBufferInterface*>( &_image ) )
	{
	}

	~ImageBuffer()
	{
		Destroy();
	}

	inline imageRawBuffer_t<T> GetRawBuffer( const uint32_t layer, const uint32_t mipLevel ) const
	{
		const slice_t slice = GetSlice( layer, mipLevel );

		imageRawBuffer_t<T> rawBuffer;
		rawBuffer.ptr = reinterpret_cast<T*>( slice.ptr );
		rawBuffer.pixelCount = slice.size / sizeof( T );

		return rawBuffer;
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

		const uint32_t index = x + y * GetWidth();
		reinterpret_cast<T*>( GetSlice( z, 0 ).ptr )[ index ] = pixel;

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

		const uint32_t index = x + y * GetWidth();
		return reinterpret_cast<const T* const>( GetSlice( z, 0 ).ptr )[ index ];
	}

	inline bool SetPixelUV( float u, float v, const T& pixel )
	{
		SetPixelUV( u, v, 0.0f, pixel );
	}

	bool SetPixelUV( float u, float v, float w, const T& pixel )
	{
		WrapUV( u, v, w );

		const uint32_t x = static_cast<uint32_t>( u * GetWidth() );
		const uint32_t y = static_cast<uint32_t>( v * GetHeight() );
		const uint32_t z = static_cast<uint32_t>( w * GetLayers() );

		return SetPixel( pixel, x, y, z );
	}

	T GetPixelUV( float u, float v, float w = 0.0f ) const
	{
		WrapUV( u, v, w );

		const uint32_t x = static_cast<uint32_t>( u * GetWidth() );
		const uint32_t y = static_cast<uint32_t>( v * GetHeight() );
		const uint32_t z = static_cast<uint32_t>( w * GetLayers() );

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