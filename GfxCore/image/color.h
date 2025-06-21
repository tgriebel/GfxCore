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

static uint8_t _pixel_trap;

class Serializer;

enum rgbaChannel_t : uint32_t
{
	RGBA_R = 0,
	RGBA_G = 1,
	RGBA_B = 2,
	RGBA_A = 3,
};

// Specialized for little-endian
union rgba8_t
{
public:
	uint8_t		vec[ 4 ];
	uint32_t	hex;
	struct {
		uint8_t	a;
		uint8_t	b;
		uint8_t	g;
		uint8_t	r;
	};

	rgba8_t() : hex( 0 ) {}
	rgba8_t( const uint32_t _hexColor ) : hex( _hexColor ) {}
};

template<typename T>
struct rgbaTuple_t
{
	T	r;
	T	g;
	T	b;
	T	a;
};

using rgba16_t = rgbaTuple_t<uint16_t>;
using rgba32_t = rgbaTuple_t<float>;

struct rgb8_t
{
	rgb8_t() : r( 0 ), g( 0 ), b( 0 ) {}
	rgb8_t( const uint8_t& r, const uint8_t& g, const uint8_t& b ) {
		this->r = r;
		this->g = g;
		this->b = b;
	}
	uint8_t	r;
	uint8_t	g;
	uint8_t	b;
};

template<typename T>
struct rgbTuple_t
{
	rgbTuple_t() : r( 0 ), g( 0 ), b( 0 ) {}
	rgbTuple_t( const T& r, const T& g, const T& b ) {
		this->r = r;
		this->g = g;
		this->b = b;
	}

	T	r;
	T	g;
	T	b;
};
using rgb16_t = rgbTuple_t<uint16_t>;
using rgb32_t = rgbTuple_t<float>;

enum class blendMode_t : uint32_t
{
	SRCALPHA,
	DESTALPHA,
	INVSRCALPHA,
	INVDESTALPHA,
	SRCCOLOR,
	DESTCOLOR,
	INVSRCCOLOR,
	INVDESTCOLOR,
	ZERO,
	ONE,
	ADD,
	SUBTRACT,
	REVSUBTRACT,
	MIN,
	MAX,
	XOR,
	COUNT,
};


inline rgba8_t Swizzle( const rgba8_t& rgba, rgbaChannel_t r, rgbaChannel_t g, rgbaChannel_t b, rgbaChannel_t a )
{
	rgba8_t swizzle;
	swizzle.r = rgba.vec[ 3 - r ];
	swizzle.g = rgba.vec[ 3 - g ];
	swizzle.b = rgba.vec[ 3 - b ];
	swizzle.a = rgba.vec[ 3 - a ];
	return swizzle;
}

template<typename T>
inline rgbaTuple_t<T> Swizzle( const rgbaTuple_t<T>& rgba, rgbaChannel_t r, rgbaChannel_t g, rgbaChannel_t b, rgbaChannel_t a )
{
	union {
		T v[4];
		rgbaTuple_t<T> rgba;
	} px;
	px.rgba = rgba;

	rgbaTuple_t<T> swizzle;
	swizzle.r = px.v[ r ];
	swizzle.g = px.v[ g ];
	swizzle.b = px.v[ b ];
	swizzle.a = px.v[ a ];
	return swizzle;
}

template<typename T>
inline rgbTuple_t<T> Swizzle( const rgbTuple_t<T>& rgb, rgbaChannel_t r, rgbaChannel_t g, rgbaChannel_t b )
{
	union {
		T v[ 3 ];
		rgbTuple_t<T> rgb;
	} px;
	px.rgb = rgb;

	rgbTuple_t<T> swizzle;
	swizzle.r = px.v[ r ];
	swizzle.g = px.v[ g ];
	swizzle.b = px.v[ b ];
	return swizzle;
}

static float _color_trap;

class Color
{
protected:
	static const uint32_t Version = 1;
	union
	{
		float		vec[ 4 ];
		rgba32_t	rgba;
	};

public:
	static const uint32_t Red		= 0xFF0000FF;
	static const uint32_t Green		= 0x00FF00FF;
	static const uint32_t Blue		= 0x0000FFFF;
	static const uint32_t White		= 0xFFFFFFFF;
	static const uint32_t Black		= 0x000000FF;
	static const uint32_t LGrey		= 0xEEEEEEFF;
	static const uint32_t DGrey		= 0x333333FF;
	static const uint32_t Brown		= 0x111111FF;
	static const uint32_t Cyan		= 0x00FFFFFF;
	static const uint32_t Yellow	= 0xFFFF00FF;
	static const uint32_t Purple	= 0xFF00FFFF;
	static const uint32_t Orange	= 0xFFA500FF;
	static const uint32_t Pink		= 0xFF69B4FF;
	static const uint32_t Gold		= 0xFFD700FF;

	Color()
	{
		for ( int32_t i = 0; i < 4; ++i )
		{
			vec[ i ] = 0.0f;
		}
	}

	Color( uint32_t _color )
	{
		rgba8_t pixel;
		pixel.hex = _color;

		rgba.r = pixel.r / 255.0f;
		rgba.g = pixel.g / 255.0f;
		rgba.b = pixel.b / 255.0f;
		rgba.a = pixel.a / 255.0f;
	}

	Color( const float _r, const float _g, const float _b, const float _a )
	{
		rgba.r	= Max( 0.0f, _r );
		rgba.g	= Max( 0.0f, _g );
		rgba.b	= Max( 0.0f, _b );
		rgba.a	= Max( 0.0f, _a );
	}

	Color( const float _r, const float _g, const float _b )
	{
		rgba.r	= Max( 0.0f, _r );
		rgba.g	= Max( 0.0f, _g );
		rgba.b	= Max( 0.0f, _b );
		rgba.a	= 1.0f;
	}

	Color( const float _value )
	{
		vec[ 0 ] = _value;
		vec[ 1 ] = _value;
		vec[ 2 ] = _value;
		vec[ 3 ] = _value;
	}

	Color( const rgb8_t& _rgb, uint8_t _a = 255 )
	{
		rgba.r = _rgb.r / 255.0f;
		rgba.g = _rgb.g / 255.0f;
		rgba.b = _rgb.b / 255.0f;
		rgba.a = _a / 255.0f;
	}

	Color( const rgba8_t& _rgba )
	{	
		rgba.r = _rgba.r / 255.0f;
		rgba.g = _rgba.g / 255.0f;
		rgba.b = _rgba.b / 255.0f;
		rgba.a = _rgba.a / 255.0f;
	}

	Color( const rgb32_t& _rgb, float _a = 1.0f )
	{
		rgba.r = _rgb.r;
		rgba.g = _rgb.g;
		rgba.b = _rgb.b;
		rgba.a = _a;
	}

	float& operator[]( const uint32_t i )
	{
		if ( i >= 4 )
		{
			assert( false );
			return _color_trap;
		}

		return vec[ i ];
	}

	float operator[]( const uint32_t i ) const
	{
		if ( i >= 4 )
		{
			assert( false );
			return _color_trap;
		}

		return vec[ i ];
	}

	Color& operator+=( const Color& color )
	{
		for ( int32_t i = 0; i < 4; ++i )
		{
			vec[ i ] += color.vec[ i ];
		}
		return *this;
	}

	Color& operator-=( const Color& color )
	{
		for ( int32_t i = 0; i < 4; ++i )
		{
			vec[ i ] -= color.vec[ i ];
		}
		return *this;
	}

	Color& operator*=( const Color& color )
	{
		for ( int32_t i = 0; i < 4; ++i )
		{
			vec[ i ] *= color.vec[ i ];
		}
		return *this;
	}

	bool operator==( const Color& color ) const
	{
		for ( int32_t i = 0; i < 4; ++i )
		{
			if ( fabs( vec[ i ] - color.vec[ i ] ) > 1e-7 )
			{
				return false;
			}
		}
		return true;
	}

	inline float& r()
	{
		return rgba.r;
	}

	inline float& g()
	{
		return rgba.g;
	}

	inline float& b()
	{
		return rgba.b;
	}

	inline float& a()
	{
		return rgba.a;
	}

	inline float r() const
	{
		return rgba.r;
	}

	inline float g() const
	{
		return rgba.g;
	}

	inline float b() const
	{
		return rgba.b;
	}

	inline float a() const
	{
		return rgba.a;
	}

	inline Color Inverse() const
	{
		Color outColor;
		for ( int32_t i = 0; i < 4; ++i )
		{
			outColor[ i ] = ( 1.0f - Min( 1.0f, Max( 0.0f, vec[ i ] ) ) );
		}
		return outColor;
	}

	inline void Swizzle( rgbaChannel_t r, rgbaChannel_t g, rgbaChannel_t b, rgbaChannel_t a )
	{
		Color src = *this;
		rgba.r = src.vec[ r ];
		rgba.g = src.vec[ g ];
		rgba.b = src.vec[ b ];
		rgba.a = src.vec[ a ];
	}

	inline rgba8_t AsRgba8() const
	{	
		float r = Min( 1.0f, Max( 0.0f, rgba.r ) );
		float g = Min( 1.0f, Max( 0.0f, rgba.g ) );
		float b = Min( 1.0f, Max( 0.0f, rgba.b ) );
		float a = Min( 1.0f, Max( 0.0f, rgba.a ) );

		rgba8_t rgba8;
		rgba8.r = static_cast<uint8_t>( 255.0f * r );
		rgba8.g = static_cast<uint8_t>( 255.0f * g );
		rgba8.b = static_cast<uint8_t>( 255.0f * b );
		rgba8.a = static_cast<uint8_t>( 255.0f * a );

		return rgba8;
	}

	rgba16_t AsRgba16() const;

	rgba32_t& AsRgba32()
	{
		return rgba;
	}

	rgba32_t AsRgba32() const
	{
		return rgba;
	}

	inline rgb8_t AsRgb8() const
	{
		float r = Min( 1.0f, Max( 0.0f, rgba.r ) );
		float g = Min( 1.0f, Max( 0.0f, rgba.g ) );
		float b = Min( 1.0f, Max( 0.0f, rgba.b ) );

		rgb8_t rgb8;
		rgb8.r = static_cast<uint8_t>( 255.0f * r );
		rgb8.g = static_cast<uint8_t>( 255.0f * g );
		rgb8.b = static_cast<uint8_t>( 255.0f * b );

		return rgb8;
	}

	rgb16_t AsRgb16() const;

	inline rgb32_t AsRgb32() const
	{
		rgb32_t rgb32;
		rgb32.r = rgba.r;
		rgb32.g = rgba.g;
		rgb32.b = rgba.b;
		return rgb32;
	}

	inline uint32_t AsHex() const
	{
		rgba8_t rgba = AsRgba8();
		return rgba.hex;
	}

	void Serialize( Serializer* serializer );
};
static_assert( sizeof( Color ) == 16, "Intended for continuous data-buffers" );


inline Color operator*( const float t, const Color& color )
{
	Color outColor;
	for ( int32_t i = 0; i < 4; ++i )
	{
		outColor[ i ] = t * color[ i ];
	}
	return outColor;
}


inline Color operator*( const Color& color, const float t )
{
	Color outColor;
	for ( int32_t i = 0; i < 4; ++i )
	{
		outColor[ i ] = t * color[ i ];
	}
	return outColor;
}


inline Color operator*( const Color& color0, const Color& color )
{
	Color outColor;
	for ( int32_t i = 0; i < 4; ++i )
	{
		outColor[ i ] = color0[ i ] * color[ i ];
	}
	return outColor;
}


inline Color operator+( const Color& color0, const Color& color1 )
{
	Color outColor;
	for ( int32_t i = 0; i < 4; ++i )
	{
		outColor[ i ] = color0[ i ] + color1[ i ];
	}
	return outColor;
}


inline Color operator-( const Color& color0, const Color& color1 )
{
	Color outColor;
	for ( int32_t i = 0; i < 4; ++i )
	{
		outColor[ i ] = color0[ i ] - color1[ i ];
	}
	return outColor;
}


static const Color ColorRed = Color::Red;
static const Color ColorGreen = Color::Green;
static const Color ColorBlue = Color::Blue;
static const Color ColorWhite = Color::White;
static const Color ColorBlack = Color::Black;
static const Color ColorLGrey = Color::LGrey;
static const Color ColorDGrey = Color::DGrey;
static const Color ColorBrown = Color::Brown;
static const Color ColorCyan = Color::Cyan;
static const Color ColorYellow = Color::Yellow;
static const Color ColorPurple = Color::Purple;
static const Color ColorOrange = Color::Orange;
static const Color ColorPink = Color::Pink;
static const Color ColorGold = Color::Gold;


Color SrgbTolinear( const Color& color, const float gamma = 2.2f );
Color LinearToSrgb( const Color& color, const float gamma = 2.2f );
Color BlendColor( const Color& src, const Color& dest, const blendMode_t blendMode );
