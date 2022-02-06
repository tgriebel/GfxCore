#pragma once

#include "common.h"
#include "mathVector.h"
#include "matrix.h"
#include "color.h"
#include "bitmap.h"
#include "image.h"

// Put useful functions that use other core classes here
// Core classes should be fairly independent from eachother
// this file alongs for dependencies


inline mat4x4f ComputeRotationX( const float degrees )
{
	const float theta = Radians( degrees );
	return CreateMatrix4x4( 1.0f, 0.0f, 0.0f, 0.0f,
							0.0f, cos( theta ), -sin( theta ), 0.0f,
							0.0f, sin( theta ), cos( theta ), 0.0f,
							0.0f, 0.0f, 0.0f, 0.0f );
}


inline mat4x4f ComputeRotationY( const float degrees )
{
	const float theta = Radians( degrees );
	return CreateMatrix4x4( cos( theta ), 0.0f, sin( theta ), 0.0f,
							0.0f, 1.0f, 0.0f, 0.0f,
							-sin( theta ), 0.0f, cos( theta ), 0.0f,
							0.0f, 0.0f, 0.0f, 0.0f );
}


inline mat4x4f ComputeRotationZ( const float degrees )
{
	const float theta = Radians( degrees );
	return CreateMatrix4x4( cos( theta ), -sin( theta ), 0.0f, 0.0f,
							sin( theta ), cos( theta ), 0.0f, 0.0f,
							0.0f, 0.0f, 1.0f, 0.0f,
							0.0f, 0.0f, 0.0f, 0.0f );
}


inline void SetTranslation( mat4x4f& inoutMatrix, const vec3f& translation )
{
	inoutMatrix[ 0 ][ 3 ] = translation[ 0 ];
	inoutMatrix[ 1 ][ 3 ] = translation[ 1 ];
	inoutMatrix[ 2 ][ 3 ] = translation[ 2 ];
	inoutMatrix[ 3 ][ 3 ] = 1.0;
}


inline mat4x4f ComputeScale( const vec3f& scale )
{
	mat4x4f mat;
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


static inline void ImageToBitmap( const Image<Color>& image, Bitmap& bitmap )
{
	bitmap.ClearImage( Color::Black );

	const uint32_t width = std::min( image.GetWidth(), bitmap.GetWidth() );
	const uint32_t height = std::min( image.GetHeight(), bitmap.GetHeight() );

	for ( int32_t y = 0; y < height; ++y )
	{
		for ( int32_t x = 0; x < width; ++x )
		{
			Color color = image.GetPixel( x, y );
			bitmap.SetPixel( x, y, color.AsR8G8B8A8() );
		}
	}
}


static inline void ImageToBitmap( const Image<float>& image, Bitmap& bitmap )
{
	bitmap.ClearImage( Color::Black );

	const uint32_t srcWidth = image.GetWidth();
	const uint32_t srcHeight = image.GetHeight();

	const uint32_t width = std::min( srcWidth, bitmap.GetWidth() );
	const uint32_t height = std::min( srcHeight, bitmap.GetHeight() );

	float minZ = FLT_MAX;
	float maxZ = -FLT_MAX;
	for ( int32_t y = 0; y < srcHeight; ++y )
	{
		for ( int32_t x = 0; x < srcWidth; ++x )
		{
			const float zValue = image.GetPixel( x, y );
			minZ = std::min( minZ, zValue );
			maxZ = std::max( maxZ, zValue );
		}
	}

	for ( int32_t y = 0; y < height; ++y )
	{
		for ( int32_t x = 0; x < width; ++x )
		{
			const float value = image.GetPixel( x, y );
			const float packed = ( value - minZ ) / ( maxZ - minZ );

			const Color c = Color( packed );
			bitmap.SetPixel( x, y, c.AsR8G8B8A8() );
		}
	}
}


static inline Color Vec3ToColor( const vec3f& v )
{
	return Color( static_cast<float>( v[ 0 ] ), static_cast<float>( v[ 1 ] ), static_cast<float>( v[ 2 ] ), 1.0f );
}


static inline Color Vec4ToColor( const vec4f& v )
{
	return Color( static_cast<float>( v[ 0 ] ), static_cast<float>( v[ 1 ] ), static_cast<float>( v[ 2 ] ), static_cast<float>( v[ 3 ] ) );
}


static inline vec4f ColorToVector( const Color& c )
{
	return vec4f( c[ 0 ], c[ 1 ], c[ 2 ], c[ 3 ] );
}


static inline float Random()
{
	return ( rand() / static_cast<float>( RAND_MAX ) );
}


static inline void RandomPointOnCircle( float& u, float& v )
{
	const float r = Random();
	const float alpha = r * 2.0f * 3.14159f;
	u = cos( alpha );
	v = sin( alpha );
}


static inline void RandomPointOnSphere( float& theta, float& phi )
{
	float u = Random();
	float v = Random();

	theta = 2.0f * 3.14159f;
	phi = acos( 2.0f * v - 1.0f );
}


static inline vec3f Randomvec3f( float r = 1.0f )
{
	float theta;
	float phi;
	RandomPointOnSphere( theta, phi );

	vec3f v;
	v[ 0 ] = r * sin( phi ) * cos( theta );
	v[ 1 ] = r * sin( phi ) * sin( theta );
	v[ 2 ] = r * cos( phi );
	return v;
}


static inline vec3f Reflectvec3f( const vec3f& n, const vec3f& v )
{
	return ( 2.0f * Dot( v, n ) * n - v );
}


static inline vec3f Refractvec3f( const vec3f& uv, const vec3f& n, float refractionRatio = 1.0f )
{
	const float dot = std::min( Dot( uv.Reverse(), n ), 1.0f );

	vec3f r0 = refractionRatio * ( uv + dot * n );
	vec3f r1 = -sqrt( fabs( 1.0f - Dot( r0, r0 ) ) ) * n;

	return ( r0 + r1 );
}