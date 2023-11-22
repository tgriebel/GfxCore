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

template<typename T>
struct rgbaTuple_t
{
	T	a;
	T	b;
	T	g;
	T	r;
};
using RGBA = rgbaTuple_t<uint8_t>;

template<typename T>
struct rgbTuple_t
{
	rgbTuple_t() : r( 0 ), g( 0 ), b( 0 ) {}
	rgbTuple_t( const T& r, const T& g, const T& b ) {
		this->r = r;
		this->g = g;
		this->b = b;
	}

	T	b;
	T	g;
	T	r;
};
using RGB = rgbTuple_t<uint8_t>;

using rgbTuplef_t = rgbTuple_t<float>;
using rgbTupled_t = rgbTuple_t<double>;

union Pixel
{
private:
	uint8_t		vec[ 4 ];
public:
	uint32_t	r8g8b8a8;
	RGBA		rgba;

	Pixel() : r8g8b8a8( 0 ) {}
	Pixel( const uint32_t _hexColor ) : r8g8b8a8( _hexColor ) {}
	Pixel( const RGBA& _rgba ) : rgba( rgba ) {}

	uint8_t& operator[]( const uint32_t i )
	{
		if ( i >= 4 )
		{
			assert( false );
			return _pixel_trap;
		}

		return vec[ 3 - i ];
	}
};

enum rgbaChannel_t : uint32_t
{
	RGBA_R = 3,
	RGBA_G = 2,
	RGBA_B = 1,
	RGBA_A = 0,
};


enum rgbChannel_t : uint32_t
{
	RGB_R = 2,
	RGB_G = 1,
	RGB_B = 0,
};


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
inline rgbTuple_t<T> Swizzle( const rgbTuple_t<T>& rgb, rgbChannel_t r, rgbChannel_t g, rgbChannel_t b )
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

class Color
{
private:
	static const uint32_t Version = 1;
	union color_t
	{
		float				vec[ 5 ];
		rgbaTuple_t<float>	rgba;
	} u;

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
			u.vec[ i ] = 0.0f;
		}
	}

	Color( uint32_t _color )
	{
		Pixel pixel;
		pixel.r8g8b8a8 = _color;

		(*this)[ 0 ] = pixel[ 0 ] / 255.0f;
		(*this)[ 1 ] = pixel[ 1 ] / 255.0f;
		(*this)[ 2 ] = pixel[ 2 ] / 255.0f;
		(*this)[ 3 ] = pixel[ 3 ] / 255.0f;
	}

	Color( const float _r, const float _g, const float _b, const float _a )
	{
		u.rgba.r	= std::max( 0.0f, _r );
		u.rgba.g	= std::max( 0.0f, _g );
		u.rgba.b	= std::max( 0.0f, _b );
		u.rgba.a	= _a;
	}

	Color( const float _r, const float _g, const float _b )
	{
		u.rgba.r	= std::max( 0.0f, _r );
		u.rgba.g	= std::max( 0.0f, _g );
		u.rgba.b	= std::max( 0.0f, _b );
		u.rgba.a	= 1.0f;
	}

	Color( const float _value )
	{
		u.vec[ 0 ] = _value;
		u.vec[ 1 ] = _value;
		u.vec[ 2 ] = _value;
		u.vec[ 3 ] = _value;
	}

	Color( const RGB& rgb, uint8_t a )
	{
		u.rgba.r = rgb.r / 255.0f;
		u.rgba.g = rgb.g / 255.0f;
		u.rgba.b = rgb.b;
		u.rgba.a = a;
	}

	Color( const RGBA& rgba )
	{	
		u.rgba.r = rgba.r / 255.0f;
		u.rgba.g = rgba.g / 255.0f;
		u.rgba.b = rgba.b / 255.0f;
		u.rgba.a = rgba.a / 255.0f;
	}

	Color( const rgbTuple_t<float>& rgb, float a = 1.0f )
	{
		u.rgba.r = rgb.r;
		u.rgba.g = rgb.g;
		u.rgba.b = rgb.b;
		u.rgba.a = a;
	}

	float& operator[]( const uint32_t i )
	{
		if ( i >= 4 )
		{
			assert( false );
			return u.vec[ 4 ];
		}

		return u.vec[ 3 - i ];
	}

	float operator[]( const uint32_t i ) const
	{
		if ( i >= 4 )
		{
			assert( false );
			return u.vec[ 4 ];
		}

		return u.vec[ 3 - i ];
	}

	Color& operator+=( const Color& color )
	{
		for ( int32_t i = 0; i < 4; ++i )
		{
			u.vec[ i ] += color.u.vec[ i ];
		}
		return *this;
	}

	Color& operator-=( const Color& color )
	{
		for ( int32_t i = 0; i < 4; ++i )
		{
			u.vec[ i ] -= color.u.vec[ i ];
		}
		return *this;
	}

	Color& operator*=( const Color& color )
	{
		for ( int32_t i = 0; i < 4; ++i )
		{
			u.vec[ i ] *= color.u.vec[ i ];
		}
		return *this;
	}

	bool operator==( const Color& color ) const
	{
		for ( int32_t i = 0; i < 4; ++i )
		{
			if ( fabs( u.vec[ i ] - color.u.vec[ i ] ) > 1e-7 )
			{
				return false;
			}
		}
		return true;
	}

	inline float& r()
	{
		return u.rgba.r;
	}

	inline float& g()
	{
		return u.rgba.g;
	}

	inline float& b()
	{
		return u.rgba.b;
	}

	inline float& a()
	{
		return u.rgba.a;
	}

	inline rgbaTuple_t<float>& rgba()
	{
		return u.rgba;
	}

	inline float r() const
	{
		return u.rgba.r;
	}

	inline float g() const
	{
		return u.rgba.g;
	}

	inline float b() const
	{
		return u.rgba.b;
	}

	inline float a() const
	{
		return u.rgba.a;
	}

	inline rgbaTuple_t<float> rgba() const
	{
		return u.rgba;
	}

	inline Color Inverse() const
	{
		Color outColor;
		for ( int32_t i = 0; i < 4; ++i )
		{
			outColor[ i ] = ( 1.0f - std::min( 1.0f, std::max( 0.0f, u.vec[ i ] ) ) );
		}
		return outColor;
	}

	inline RGBA AsRGBA() const
	{
		RGBA rgba;
		rgba.r = static_cast<uint8_t>( 255.0f * u.rgba.r );
		rgba.g = static_cast<uint8_t>( 255.0f * u.rgba.g );
		rgba.b = static_cast<uint8_t>( 255.0f * u.rgba.b );
		rgba.a = static_cast<uint8_t>( 255.0f * u.rgba.a );
		return rgba;
	}

	inline RGB AsRGB() const
	{
		RGB rgb;
		rgb.r = static_cast<uint8_t>( 255.0f * u.rgba.r );
		rgb.g = static_cast<uint8_t>( 255.0f * u.rgba.g );
		rgb.b = static_cast<uint8_t>( 255.0f * u.rgba.b );
		return rgb;
	}

	inline rgbTuple_t<float> AsRGBf() const
	{
		rgbTuple_t<float> rgb;
		rgb.r = u.rgba.r;
		rgb.g = u.rgba.g;
		rgb.b = u.rgba.b;
		return rgb;
	}

	inline uint32_t AsR8G8B8A8() const
	{
		const float r = std::min( 1.0f, std::max( 0.0f, u.rgba.r ) );
		const float g = std::min( 1.0f, std::max( 0.0f, u.rgba.g ) );
		const float b = std::min( 1.0f, std::max( 0.0f, u.rgba.b ) );
		const float a = std::min( 1.0f, std::max( 0.0f, u.rgba.a ) );

		const uint8_t p0 = static_cast<uint8_t>( 255.0f * r );
		const uint8_t p1 = static_cast<uint8_t>( 255.0f * g );
		const uint8_t p2 = static_cast<uint8_t>( 255.0f * b );
		const uint8_t p3 = static_cast<uint8_t>( 255.0f * a );

		Pixel abgr;
		abgr.rgba = { p3, p2, p1, p0 };
		return abgr.r8g8b8a8;
	}

	void Serialize( Serializer* serializer );
};


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
