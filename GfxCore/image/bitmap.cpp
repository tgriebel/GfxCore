/*
* MIT License
*
* Copyright( c ) 2009-2023 Thomas Griebel
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

#include"bitmap.h"

Bitmap::Bitmap( const std::string& filename )
{
	Load( filename );
}


Bitmap::~Bitmap()
{
	if ( mapdata == nullptr )
	{
		delete[] mapdata;
	}
	pixelCnt = 0;
	memset( &h, 0, sizeof( headerInfo_t ) );
}


Bitmap::Bitmap( const Bitmap& bitmap )
{
	h = bitmap.h;

	pixelCnt = bitmap.pixelCnt;

	if( mapdata == nullptr )
	{
		delete[] mapdata;
	}
	mapdata = new rgba8_t[ pixelCnt ];

	for ( uint32_t i = 0; i < pixelCnt; ++i )
	{
		mapdata[ i ] = bitmap.mapdata[ i ];
	}
}


Bitmap::Bitmap( const uint32_t width, const uint32_t height, const uint32_t color )
{
	pixelCnt = ( width * height );

	mapdata = new rgba8_t[ pixelCnt ];

	h.magicNum[ 0 ]	= 'B';
	h.magicNum[ 1 ]	= 'M';

	h.size			= ( pixelCnt * 4 );
	h.reserve1		= 0;
	h.reserve2		= 0;
	h.offset		= 54;

	h.hSize			= 40;
	h.width			= width;
	h.height		= height;
	h.cPlanes		= 1;
	h.bpPixels		= 32;
	h.compression	= 0;
	h.imageSize		= ( h.bpPixels * pixelCnt );
	h.hRes			= 0;
	h.vRes			= 0;
	h.colors		= 0;
	h.iColors		= 0;

	for ( size_t i = 0; i < pixelCnt; ++i )
	{
		mapdata[ i ] = rgba8_t( Color( color ).AsRgba8() );
	}
}


Bitmap& Bitmap::operator=( const Bitmap& bitmap )
{
	assert(0);
	return *this;
}


template<int ByteCnt>
void Bitmap::WriteInt( uint32_t value )
{
	assert( ByteCnt <= 4 );

	uint32_t temp = value;
	uint8_t bin[ ByteCnt ];
	
	for ( int32_t i = 0; i < ByteCnt; ++i )
	{
		bin[ i ] = static_cast<uint8_t>( temp & 0xFF );
		temp >>= 8;
	}

	outstream.write( reinterpret_cast<char*>( bin ), ByteCnt );
}


template<int ByteCnt>
uint32_t Bitmap::ReadInt()
{
	assert( ByteCnt <= 4 );

	uint8_t bin[ ByteCnt ];

	instream.read( reinterpret_cast<char*>( bin ), ByteCnt );

	const int cbyte = 8;
	uint32_t numeral = 0;

	for ( uint32_t i = 1; i <= ByteCnt; ++i )
	{
		uint32_t temp = static_cast<uint32_t>( bin[ ByteCnt - i ] );
		temp &= 0x000000FF;
		temp <<= ( ByteCnt - i ) * cbyte;
		numeral |= temp;
	}

	return numeral;
}


bool Bitmap::Load( const std::string& filename )
{
	instream.open( filename.c_str(), std::ios::in | std::ios::binary );

	if( instream.fail() )
	{
		return false;
	}

	instream.read( reinterpret_cast<char*>( h.magicNum ), 2 );

	h.size			= ReadInt<4>();
	h.reserve1		= ReadInt<2>();
	h.reserve2		= ReadInt<2>();
	h.offset		= ReadInt<4>();

	h.hSize			= ReadInt<4>();
	h.width			= ReadInt<4>();
	h.height		= ReadInt<4>();
	h.cPlanes		= ReadInt<2>();
	h.bpPixels		= ReadInt<2>();
	h.compression	= ReadInt<4>();
	h.imageSize		= ReadInt<4>();
	h.hRes			= ReadInt<4>();
	h.vRes			= ReadInt<4>();
	h.colors		= ReadInt<4>();
	h.iColors		= ReadInt<4>();

	uint32_t padding = 0;

	if ( ( h.width % 4 ) != 0 )
	{
		padding = 4 - ( h.width % 4 );
	}

	const uint32_t srcSize = h.imageSize;
	const uint16_t srcBitDepth = h.bpPixels;

	// Convert to supported format
	{
		h.bpPixels = 32;
		h.compression = 0;
		h.imageSize = 4 * ( h.width + padding ) * h.height;
	}

	instream.seekg( h.offset, std::ios_base::beg );

	pixelCnt = ( h.width + padding ) * h.height;
	mapdata = new rgba8_t[ pixelCnt ];

	const int32_t pixelBytes = ( srcBitDepth >> 3 );
	const int32_t lineBytes = ( pixelBytes * h.width );
	uint8_t* buffer = new uint8_t[ lineBytes ];

	for ( int32_t row = ( h.height - 1 ); row >= 0; row-- )
	{
		instream.read( reinterpret_cast<char*>( buffer ), lineBytes );

		for ( uint32_t pixNum = 0; pixNum < ( h.width * pixelBytes ); pixNum += pixelBytes )
		{
			rgba8_t pixel;
			//parse raw data width/24bits
			pixel.b	= buffer[ pixNum ];
			pixel.g	= buffer[ pixNum + 1 ];
			pixel.r	= buffer[ pixNum + 2 ];
			pixel.a	= (uint8_t) 0xFF;

			mapdata[ row * h.width + ( pixNum / pixelBytes ) ] = rgba8_t( pixel );
		}

		instream.seekg( padding + instream.tellg(), std::ios_base::beg );
	}

	delete[] buffer;
	buffer = nullptr;

	instream.close();

	return true;
}


void Bitmap::Write( const std::string& filename )
{
	outstream.open( filename.c_str(), std::ios::out | std::ios::binary | std::ios::trunc );

	outstream.write( reinterpret_cast<char*>( h.magicNum ), 2 );

	WriteInt<4>( h.size );
	WriteInt<2>( h.reserve1 );
	WriteInt<2>( h.reserve2 );
	WriteInt<4>( h.offset );

	WriteInt<4>( h.hSize );
	WriteInt<4>( h.width );
	WriteInt<4>( h.height );
	WriteInt<2>( h.cPlanes );
	WriteInt<2>( h.bpPixels );
	WriteInt<4>( h.compression );
	WriteInt<4>( h.imageSize );
	WriteInt<4>( h.hRes );
	WriteInt<4>( h.vRes );
	WriteInt<4>( h.colors );
	WriteInt<4>( h.iColors );

	outstream.seekp( h.offset, std::ios_base::beg );

	int padding = 0;
	if ( ( h.width % 4 ) != 0 )
	{
		padding = 4 - ( h.width % 4 );
	}

	const uint8_t pad[ 4 ] = { '\0','\0', '\0','\0' };

	for ( int32_t row = ( h.height - 1 ); row >= 0; row-- )
	{
		const int32_t rowOffset = ( row * h.width );
		for ( uint32_t colIx = 0; colIx < h.width; colIx++ )
		{
			rgba8_t pixel;
			CopyToPixel( mapdata[ colIx + rowOffset ], pixel, BITMAP_ARGB );
			WriteInt<4>( pixel.hex );
			// xARGB
		}

		outstream.write( reinterpret_cast<const char*>( pad ), padding );
	}

	outstream.close();
}


uint32_t Bitmap::GetSize() const
{
	return h.imageSize;
}


uint32_t Bitmap::GetWidth() const
{
	return h.width;
}


uint32_t Bitmap::GetHeight() const
{
	return h.height;
}


void Bitmap::ClearImage( const uint32_t color )
{
	for ( uint32_t i = 0; i < pixelCnt; ++i )
	{
		mapdata[ i ].hex = color;
	}
}


void Bitmap::GetBuffer( uint32_t buffer[] ) const
{
	for ( uint32_t i = 0; i < pixelCnt; ++i )
	{
		rgba8_t pixel;
		CopyToPixel( mapdata[ i ], pixel, BITMAP_BGRA );

		buffer[ i ] = pixel.hex;
	}
}


bool Bitmap::SetPixel( const int32_t x, const int32_t y, const uint32_t color )
{
	if ( ( x >= static_cast<int32_t>( h.width ) ) || ( x < 0 ) || ( y >= static_cast<int32_t>( h.height ) ) || ( y < 0 ) )
	{
		return false;
	}
	else
	{
		mapdata[ y * h.width + x ] = Color( color ).AsRgba8();
		return true;
	}
}


uint32_t Bitmap::GetPixel( const int32_t x, const int32_t y ) const
{
	if ( ( x >= static_cast<int32_t>( h.width ) ) || ( x < 0 ) || ( y >= static_cast<int32_t>( h.height ) ) || ( y < 0 ) )
	{
		return 0;
	}
	else
	{
		return mapdata[ y * h.width + x ].hex;
	}
}


void Bitmap::CopyToPixel( const rgba8_t& rgba, rgba8_t& pixel, BitmapFormat format )
{
	switch ( format )
	{
	case BITMAP_ABGR:
	{
		pixel.r = rgba.a;
		pixel.g = rgba.b;
		pixel.b = rgba.g;
		pixel.a = rgba.r;
	}
	break;

	case BITMAP_ARGB:
	{
		pixel.r = rgba.a;
		pixel.g = rgba.r;
		pixel.b = rgba.g;
		pixel.a = rgba.b;
	}
	break;

	case BITMAP_BGRA:
	{
		pixel.r = rgba.b;
		pixel.g = rgba.g;
		pixel.b = rgba.r;
		pixel.a = rgba.a;
	}
	break;

	default:
	case BITMAP_RGBA:
	{
		pixel = rgba;
	}
	break;
	}
}