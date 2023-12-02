#include <cstdint>

#include "image.h"
#include "color.h"


inline void ImageConvertPixel( const Color& src, RGBA& dst )
{
	dst = src.AsRGBA();
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
				ImageAdaptPixel( from.GetPixel( x, y, l ), rgba );

				to.SetPixel( x, y, l, rgba );
			}
		}
	}
}