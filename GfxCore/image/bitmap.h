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

#pragma once

#include <iostream>
#include <fstream>
#include <assert.h>

#include "color.h"

enum BitmapFormat : uint32_t
{
	BITMAP_ABGR = 0,
	BITMAP_ARGB = 1,
	BITMAP_BGRA = 2,
	BITMAP_RGBA = 3,
};


class Bitmap
{
public:
	Bitmap() = delete;

	Bitmap( const std::string& filename );
	Bitmap( const Bitmap& bitmap );
	Bitmap( const uint32_t width, const uint32_t height, const uint32_t color = ~0x00 );
	~Bitmap();

	Bitmap& operator=( const Bitmap& bitmap );
	static void CopyToPixel( const RGBA& rgba, Pixel& pixel, BitmapFormat format );

	bool Load( const std::string& filename );
	void Write( const std::string& filename );

	uint32_t GetSize() const;
	uint32_t GetWidth() const;
	uint32_t GetHeight() const;

	void ClearImage( const uint32_t color = 0xFF );

	void GetBuffer( uint32_t buffer[] ) const;

	uint32_t GetPixel( const int32_t x, const int32_t y ) const;
	bool SetPixel( const int32_t x, const int32_t y, const uint32_t color );

private:

	struct headerInfo_t
	{
		uint8_t magicNum[ 2 ];
		uint32_t size;
		uint16_t reserve1;
		uint16_t reserve2;
		uint32_t offset;
		uint32_t hSize;
		uint32_t width;
		uint32_t height;
		uint16_t cPlanes;
		uint16_t bpPixels;
		uint32_t compression;
		uint32_t imageSize;
		uint32_t hRes;
		uint32_t vRes;
		uint32_t colors;
		uint32_t iColors;
	} h;

	Pixel* mapdata;
	uint32_t pixelCnt;

	std::ifstream instream;
	std::ofstream outstream;

	template<int ByteCnt>
	uint32_t ReadInt();

	template<int ByteCnt>
	void WriteInt( const uint32_t value );
};