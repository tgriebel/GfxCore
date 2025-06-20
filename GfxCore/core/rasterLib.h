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

#include <deque>
#include "../math/vector.h"
#include "../math/matrix.h"
#include "../image/image.h"
#include "../image/bitmap.h"

/*
===================================
CompositeBitmaps
===================================
*/
inline Bitmap CompositeBitmaps( Bitmap& bitmap1, Bitmap& bitmap2 )
{
	Bitmap outBitmap( bitmap1.GetWidth(), bitmap1.GetHeight() );

	for ( uint32_t j = 0; j < bitmap1.GetHeight(); ++j ) {
		for ( uint32_t i = 0; i < bitmap1.GetWidth(); ++i ) {

			uint32_t newColor = BlendColor( Color( bitmap1.GetPixel( i, j ) ), Color( bitmap2.GetPixel( i, j ) ), blendMode_t::SRCALPHA ).AsHex();

			outBitmap.SetPixel( i, j, newColor );
		}
	}

	return outBitmap;
}


/*
===================================
DrawLine
- Optimized bresenham algorithm
===================================
*/
inline void DrawLine( ImageBuffer<Color>& image, int32_t x0, int32_t y0, int32_t x1, int32_t y1, const Color& color, blendMode_t blendMode = blendMode_t::SRCALPHA )
{
	x0 = Clamp( x0, 0, (int32_t)image.GetWidth() - 1 );
	y0 = Clamp( y0, 0, (int32_t)image.GetHeight() - 1 );
	x1 = Clamp( x1, 0, (int32_t)image.GetWidth() - 1 );
	y1 = Clamp( y1, 0, (int32_t)image.GetHeight() - 1 );

	const int dx = abs( x1 - x0 );
	const int dy = abs( y1 - y0 );
	const int sx = ( x0 < x1 ) ? 1 : -1;
	const int sy = ( y0 < y1 ) ? 1 : -1;

	int32_t e = ( dx - dy );

	while ( ( x0 != x1 ) || ( y0 != y1 ) )
	{
		Color finalColor = BlendColor( color, image.GetPixel( x0, y0 ), blendMode );
		image.SetPixel( x0, y0, finalColor );

		const int e2 = 2 * e;

		if ( e2 > -dy )
		{
			e -= dy;
			x0 += sx;
		}

		if ( e2 < dx )
		{
			e += dx;
			y0 += sy;
		}
	}

	Color finalColor = BlendColor( color, image.GetPixel( x0, y0 ), blendMode );
	image.SetPixel( x0, y0, color );
}

/*
===================================
FloodFill
- Performs standard floodfill algorithm
===================================
*/
inline void FloodFill( Bitmap& bitmap, uint32_t x, uint32_t y, uint32_t tcolor, uint32_t rcolor )
{
	bool outBounds = ( x >= bitmap.GetWidth() || x < 0 ) || ( y >= bitmap.GetHeight() || y < 0 );
	if ( outBounds )
	{
		return;
	}

	if ( bitmap.GetPixel( x, y ) != tcolor )
	{
		return;
	}

	struct node_t
	{
		node_t( uint32_t xi, uint32_t yi ) : x( xi ), y( yi ) {};
		uint32_t x, y;
	};

	std::deque<node_t> q;
	q.push_back( node_t( x, y ) );

	while ( !q.empty() ) {

		node_t n = q.front();
		q.pop_front();

		if ( bitmap.GetPixel( n.x, n.y ) == tcolor ) {
			node_t w( n ), e( n );

			for ( ; ( bitmap.GetPixel( w.x - 1, w.y ) == tcolor ) && ( w.x != 0 ); w.x-- ); // Find min horizonal bond
			for ( ; ( bitmap.GetPixel( e.x + 1, e.y ) == tcolor ) && ( e.x != bitmap.GetWidth() - 1 ); e.x++ ); // Find max bond

			for ( uint32_t i = w.x; i <= e.x; ++i )		// Fill line
				bitmap.SetPixel( i, w.y, rcolor );

			for ( uint32_t i( w.x + 1 ); i < e.x; ++i ) { // Explore outside of filled line
				node_t t_node( i, 0 );

				t_node.y = w.y + 1;	// North of node
				if ( ( t_node.y > 0 ) && ( bitmap.GetPixel( t_node.x, t_node.y ) == tcolor ) )	q.push_back( t_node );

				t_node.y = w.y - 1;	// South of node
				if ( ( t_node.y < bitmap.GetHeight() ) && ( bitmap.GetPixel( t_node.x, t_node.y ) == tcolor ) )	q.push_back( t_node );
			}
		}
	}

	return;
}


/*
===================================
ApplyBlur
- Performs blur on image
===================================
*/
inline void ApplyBlur( Bitmap& bitmap, Bitmap& output )
{
	uint32_t kernelDim = 3;

	int32_t values[] = { 0, 0, 0,
							0, 1, 0,
							0, 0, 0 };

	const uint32_t kernelWidth = 3;
	const uint32_t kernelSize = ( kernelWidth * kernelWidth );

	using matKern_t = Matrix<kernelWidth, kernelWidth, int32_t>;

	matKern_t kernel( values );
	float divisor = 1; // shouldn't blur more than # pixels in covolution
	uint8_t mask = 0xE;

	for ( uint32_t i = ( kernelDim / 2 ); i < bitmap.GetWidth() - ( kernelDim / 2 ); ++i ) {

		for ( uint32_t j = ( kernelDim / 2 ); j < bitmap.GetHeight() - ( kernelDim / 2 ); ++j ) {

			uint32_t kValues[] = { bitmap.GetPixel( i - 1, j - 1 ), bitmap.GetPixel( i, j - 1 ), bitmap.GetPixel( i + 1, j - 1 ),
									bitmap.GetPixel( i - 1, j ), bitmap.GetPixel( i, j ), bitmap.GetPixel( i + 1, j ),
									bitmap.GetPixel( i - 1, j + 1 ), bitmap.GetPixel( i, j + 1 ), bitmap.GetPixel( i + 1, j + 1 )
			};

			rgba8_t rgba = Color( kValues[ 4 ] ).AsRgba8();

			//RED CHANNEL
			int32_t rSum = rgba.r;

			if ( mask & 0x8 ) {
				int32_t rChannel[ kernelSize ];
				for ( uint32_t v = 0; v < kernelSize; ++v )
				{
					rChannel[ v ] = Color( kValues[ v ] ).AsRgba8().r;
				}

				Matrix<3, 3, int32_t> rSample( rChannel );
				rSum = Convolution( rSample, kernel );
				rSum = (int32_t) ( (float) rSum / divisor );
			}

			// GREEN CHANNEL
			int32_t gSum = rgba.g;

			if ( mask & 0x4 ) {
				int32_t gChannel[ kernelSize ];
				for ( uint32_t v = 0; v < kernelSize; ++v )
					gChannel[ v ] = Color( kValues[ v ] ).AsRgba8().g;

				matKern_t g_sample( gChannel );
				gSum = Convolution( g_sample, kernel );
				gSum = (int32_t) ( (float) gSum / divisor );
			}

			// BLUE CHANNEL
			int32_t bSum = rgba.b;

			if ( mask & 0x2 ) {
				int32_t bChannel[ kernelSize ];
				for ( uint32_t v = 0; v < kernelSize; ++v )
					bChannel[ v ] = Color( (uint32_t) kValues[ v ] ).AsRgba8().b;

				matKern_t bSample( bChannel );
				bSum = Convolution( bSample, kernel );
				bSum = (int32_t) ( (float) bSum / divisor );
			}

			// ALPHA CHANNEL
			int32_t aSum = rgba.a;

			if ( mask & 0x1 ) {
				int32_t aChannel[ kernelSize ];
				for ( uint32_t v = 0; v < kernelSize; ++v )
					aChannel[ v ] = Color( kValues[ v ] ).AsRgba8().a;

				matKern_t aSample( aChannel );
				aSum = Convolution( aSample, kernel );
				aSum = (int32_t) ( (float) aSum / divisor );
			}

			//if(r_sum > 0xFF) cout << r_sum << endl;

			//if(r_sum < 0) r_sum = ~abs(r_sum);
			//if(g_sum < 0) g_sum = ~abs(g_sum);
			//if(b_sum < 0) b_sum = ~abs(b_sum);
			//if(a_sum < 0) r_sum = ~r_sum;

			rgba8_t pc;
			pc.r = (uint32_t) ( rSum < 0 ? 0 : rSum );
			pc.g = (uint32_t) ( gSum < 0 ? 0 : gSum );
			pc.b = (uint32_t) ( bSum < 0 ? 0 : bSum );
			pc.a = (uint32_t) ( aSum < 0 ? 0 : aSum );

			//cout << hex << convo.GetPixel(i,j) << endl;
			//cout << hex << pc.rawGetColor() << endl;

			output.SetPixel( i, j, pc.hex );
		}
	}

	for ( uint32_t i = 0; i < bitmap.GetWidth(); ++i ) {

		for ( uint32_t j = 0; j < bitmap.GetHeight(); ++j ) {

			output.SetPixel( i, j, bitmap.GetPixel( i, j ) | output.GetPixel( i, j ) );
		}
	}

	output.Write( "convolution.bmp" );
}

/*
===================================
Cohen�Sutherland Algorithm
- Classifies 2D point relative to clipping region
===================================
*/
enum clipRegion_t
{
	CLIP_REGION_INSIDE	= 0x00,
	CLIP_REGION_LEFT	= 0x01,
	CLIP_REGION_RIGHT	= 0x02,
	CLIP_REGION_BOTTOM	= 0x04,
	CLIP_REGION_TOP		= 0x08,
};


inline uint32_t ComputeClipCode( const vec2f& pt )
{
	uint32_t code;

	const float clipSize = 2.0;
	code = CLIP_REGION_INSIDE;

	if ( pt[ 0 ] < clipSize )
	{
		code |= CLIP_REGION_LEFT;
	}
	else if ( pt[ 0 ] > clipSize )
	{
		code |= CLIP_REGION_RIGHT;
	}

	if ( pt[ 1 ] < clipSize )
	{
		code |= CLIP_REGION_BOTTOM;
	}
	else if ( pt[ 1 ] > clipSize )
	{
		code |= CLIP_REGION_TOP;
	}

	return code;
}


/*
===================================
ProjectPoint
- Projects point from world space into screen space
===================================
*/
inline void ProjectPoint( const mat4x4f& mvp, const vec2i& screenSize, const vec4f& worldSpacePt, vec4f& outPoint )
{
	// Clip-Space
	vec4f csPt = mvp * vec4f( worldSpacePt[ 0 ], worldSpacePt[ 1 ], worldSpacePt[ 2 ], 1.0 );

	// Normalized-Device-Coordinates
	const float w = csPt[ 3 ] + 1e-7f;
	vec4f ndsPt = vec4f( csPt[ 0 ] / w, csPt[ 1 ] / w, csPt[ 2 ] / w, w );

	// Screen-Space
	outPoint[ 0 ] = 0.5f * screenSize[ 0 ] * ( ndsPt[ 0 ] + 1.0f );
	outPoint[ 1 ] = 0.5f * screenSize[ 1 ] * ( ndsPt[ 1 ] + 1.0f );
	outPoint[ 2 ] = ndsPt[ 2 ];
	outPoint[ 3 ] = w;
}


template<typename T>
inline T Interpolate( const vec3f& baryCoord, const T attrib[3] )
{
	T value;

	value = ( baryCoord[ 0 ] * attrib[ 0 ] );
	value += ( baryCoord[ 1 ] * attrib[ 1 ] );
	value += ( baryCoord[ 2 ] * attrib[ 2 ] );

	return value;
}