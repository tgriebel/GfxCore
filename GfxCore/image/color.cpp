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

#include "color.h"
#include "../core/util.h"


rgb16_t Color::AsRgb16() const
{
	rgb16_t rgb16;
	rgb16.r = PackFloat32( rgba.r );
	rgb16.g = PackFloat32( rgba.g );
	rgb16.b = PackFloat32( rgba.b );
	return rgb16;
}


rgba16_t Color::AsRgba16() const
{
	rgba16_t rgba16;
	rgba16.r = PackFloat32( rgba.r );
	rgba16.g = PackFloat32( rgba.g );
	rgba16.b = PackFloat32( rgba.b );
	rgba16.a = PackFloat32( rgba.a );
	return rgba16;
}


Color SrgbTolinear( const Color& color, const float gamma )
{
	Color outColor;
	for ( int32_t i = 0; i < 3; ++i )
	{
		outColor[ i ] = Saturate( pow( color[ i ], gamma ) );
	}
	outColor[ 3 ] = 1.0f;

	return outColor;
}


Color LinearToSrgb( const Color& color, const float gamma )
{
	Color outColor;
	for ( int32_t i = 0; i < 3; ++i )
	{
		outColor[ i ] = Saturate( pow( color[ i ], 1.0f / gamma ) );
	}
	outColor[ 3 ] = 1.0f;

	return outColor;
}


Color BlendColor( const Color& src, const Color& dest, const blendMode_t blendMode )
{
	switch ( blendMode )
	{
		default:
		case blendMode_t::SRCCOLOR:
		{
			return src;
		}
		case blendMode_t::DESTCOLOR:
		{
			return dest;
		}
		case blendMode_t::SRCALPHA:
		{
			return Lerp( dest, src, src.a() );
		}
		case blendMode_t::DESTALPHA:
		{
			return Lerp( dest, src, dest.a() );
		}
		case blendMode_t::INVSRCALPHA:
		{
			return Lerp( dest, src, 1.0f - src.a() );
		}
		case blendMode_t::INVDESTALPHA:
		{
			return Lerp( dest, src, 1.0f - dest.a() );
		}
		case blendMode_t::INVSRCCOLOR:
		{
			return src.Inverse();
		}
		case blendMode_t::INVDESTCOLOR:
		{
			return dest.Inverse();
		}
		case blendMode_t::ADD:
		{
			return ( src + dest );
		}
		case blendMode_t::SUBTRACT:
		{
			return ( src - dest );
		}
		case blendMode_t::REVSUBTRACT:
		{
			return ( dest - src );
		}
		case blendMode_t::MIN:
		{
			Color out;
			for ( int32_t i = 0; i < 4; ++i )
			{
				out[ i ] = std::min( src[ i ], dest[ i ] );
			}
			return out;
		}
		case blendMode_t::XOR:
		{
			return Color( src.AsHex() ^ dest.AsHex() );
		}
		case blendMode_t::MAX:
		{
			Color out;
			for ( int32_t i = 0; i < 4; ++i )
			{
				out[ i ] = std::max( src[ i ], dest[ i ] );
			}
			return out;
		}
		case blendMode_t::ZERO:
		{
			return Color( 0.0f );
		}
		case blendMode_t::ONE:
		{
			return Color( 1.0f );
		}
	}
}
