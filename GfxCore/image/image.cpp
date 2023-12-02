#include <cstdint>

#include "image.h"
#include "color.h"


void WrapUV( float& u, float& v )
{
	u = ( u > 1.0 ) ? ( u - floor( u ) ) : u;
	v = ( v > 1.0 ) ? ( v - floor( v ) ) : v;

	u = Saturate( u );
	v = Saturate( v );
}


void WrapUV( float& u, float& v, float& w )
{
	u = ( u > 1.0 ) ? ( u - floor( u ) ) : u;
	v = ( v > 1.0 ) ? ( v - floor( v ) ) : v;
	w = ( w > 1.0 ) ? ( w - floor( v ) ) : w;

	u = Saturate( u );
	v = Saturate( v );
	w = Saturate( w );
}


inline void ImageConvertPixel( const Color& src, RGBA& dst )
{
	dst = src.AsRGBA();
}


inline void ImageConvertPixel( const RGBA& src, Color& dst )
{
	dst = Color( src );
}


template<class SourceType, class DestType>
void ImageConvert( const ImageBuffer<SourceType>& from, ImageBuffer<DestType>& to )
{
	const char* name = to.GetName();
	to.Destroy();
	to.Init( from.GetWidth(), from.GetHeight(), from.GetLayers(), name );

	const uint32_t layers = to.GetLayers();
	const uint32_t height = to.GetHeight();
	const uint32_t width = to.GetWidth();

	for ( uint32_t l = 0; l < layers; ++l )
	{	
		for ( uint32_t y = 0; y < height; ++y )
		{		
			for ( uint32_t x = 0; x < width; ++x )
			{
				DestType rgba;
				ImageConvertPixel( from.GetPixel( x, y, l ), rgba );

				to.SetPixel( x, y, l, rgba );
			}
		}
	}
}