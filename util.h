#pragma once

#include "common.h"
#include "mathVector.h"
#include "matrix.h"
#include "color.h"
#include "bitmap.h"

// Put useful functions that use other core classes here
// Core classes should be fairly independent from eachother
// this file alongs for dependencies


inline mat4x4d ComputeRotationX( const double degrees )
{
	const double theta = Radians( degrees );
	return CreateMatrix4x4( 1.0, 0.0, 0.0, 0.0,
		0.0, cos( theta ), -sin( theta ), 0.0,
		0.0, sin( theta ), cos( theta ), 0.0,
		0.0, 0.0, 0.0, 1.0 );
}


inline mat4x4d ComputeRotationY( const double degrees )
{
	const double theta = Radians( degrees );
	return CreateMatrix4x4( cos( theta ), 0.0, sin( theta ), 0.0,
		0.0, 1.0, 0.0, 0.0,
		-sin( theta ), 0.0, cos( theta ), 0.0,
		0.0, 0.0, 0.0, 1.0 );
}


inline mat4x4d ComputeRotationZ( const double degrees )
{
	const double theta = Radians( degrees );
	return CreateMatrix4x4( cos( theta ), -sin( theta ), 0.0, 0.0,
		sin( theta ), cos( theta ), 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0 );
}


inline void SetTranslation( mat4x4d& inoutMatrix, const vec3d& translation )
{
	inoutMatrix[ 0 ][ 3 ] = translation[ 0 ];
	inoutMatrix[ 1 ][ 3 ] = translation[ 1 ];
	inoutMatrix[ 2 ][ 3 ] = translation[ 2 ];
	inoutMatrix[ 3 ][ 3 ] = 1.0;
}


inline mat4x4d ComputeScale( const vec3d& scale )
{
	mat4x4d mat;
	mat[ 0 ][ 0 ] = scale[ 0 ];
	mat[ 1 ][ 1 ] = scale[ 1 ];
	mat[ 2 ][ 2 ] = scale[ 2 ];
	mat[ 3 ][ 3 ] = 1.0;
	return mat;
}


static inline void BitmapToImage( const Bitmap& bitmap, Image<Color>& image )
{
	image.Clear( Color::Black );

	const uint32_t srcWidth = bitmap.GetWidth();
	const uint32_t srcHeight = bitmap.GetHeight();

	const uint32_t dstWidth = std::min( srcWidth, image.GetWidth() );
	const uint32_t dstHeight = std::min( srcHeight, image.GetHeight() );

	for ( int32_t y = 0; y < dstHeight; ++y )
	{
		for ( int32_t x = 0; x < dstWidth; ++x )
		{
			const uint32_t r8g8b8a8 = bitmap.GetPixel( x, y );
			image.SetPixel( x, y, Color( r8g8b8a8 ) );
		}
	}
}