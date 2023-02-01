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

#include "../core/common.h"
#include "../math/vector.h"
#include "../math/matrix.h"
#include "../image/color.h"
#include "../image/bitmap.h"
#include "../image/image.h"

inline mat4x4f ComputeRotationX( const float degrees )
{
	const float theta = Radians( degrees );
	return CreateMatrix4x4( 1.0f,	0.0f,			0.0f,			0.0f,
							0.0f,	cos( theta ),	-sin( theta ),	0.0f,
							0.0f,	sin( theta ),	cos( theta ),	0.0f,
							0.0f,	0.0f,			0.0f,			1.0f );
}


inline mat4x4f ComputeRotationY( const float degrees )
{
	const float theta = Radians( degrees );
	return CreateMatrix4x4( cos( theta ),	0.0f,	sin( theta ),	0.0f,
							0.0f,			1.0f,	0.0f,			0.0f,
							-sin( theta ),	0.0f,	cos( theta ),	0.0f,
							0.0f,			0.0f,	0.0f,			1.0f );
}


inline mat4x4f ComputeRotationZ( const float degrees )
{
	const float theta = Radians( degrees );
	return CreateMatrix4x4( cos( theta ),	-sin( theta ),	0.0f, 0.0f,
							sin( theta ),	cos( theta ),	0.0f, 0.0f,
							0.0f,			0.0f,			1.0f, 0.0f,
							0.0f,			0.0f,			0.0f, 1.0f );
}


inline mat4x4f ComputeRotationZYX( const float xDegrees, const float yDegrees, const float zDegrees )
{
	const float alpha = Radians( xDegrees );
	const float beta = Radians( yDegrees );
	const float gamma = Radians( zDegrees );

	const float cosAlpha = cos( alpha );
	const float cosBeta = cos( beta );
	const float cosGamma = cos( gamma );

	const float sinAlpha = sin( alpha );
	const float sinBeta = sin( beta );
	const float sinGamma = sin( gamma );

	return CreateMatrix4x4( cosBeta * cosGamma,		sinAlpha * sinBeta * cosGamma - cosAlpha * sinGamma,	cosAlpha * sinBeta * cosGamma + sinAlpha * sinGamma,	0.0f,
							cosBeta * sinGamma,		sinAlpha * sinBeta * sinGamma + cosAlpha * cosGamma,	cosAlpha * sinBeta * sinGamma - sinAlpha * cosGamma,	0.0f,
							-sinBeta,				sinAlpha * cosBeta,										cosAlpha * cosBeta,										0.0f,
							0.0f,					0.0f,													0.0f,													1.0f );
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

	const int32_t dstWidth = static_cast<int32_t>( std::min( srcWidth, image.GetWidth() ) );
	const int32_t dstHeight = static_cast<int32_t>( std::min( srcHeight, image.GetHeight() ) );

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

	const int32_t width = static_cast<int32_t>( std::min( image.GetWidth(), bitmap.GetWidth() ) );
	const int32_t height = static_cast<int32_t>( std::min( image.GetHeight(), bitmap.GetHeight() ) );

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

	const int32_t srcWidth = static_cast< int32_t >( image.GetWidth() );
	const int32_t srcHeight = static_cast< int32_t >( image.GetHeight() );

	const int32_t width = std::min( srcWidth, static_cast<int32_t>( bitmap.GetWidth() ) );
	const int32_t height = std::min( srcHeight, static_cast<int32_t>( bitmap.GetHeight() ) );

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

static inline void RandSpherePoint( const float radius, vec3f& outPoint )
{
	float theta;
	float phi;
	RandomPointOnSphere( theta, phi );

	outPoint[ 0 ] = radius * sin( phi ) * cos( theta );
	outPoint[ 1 ] = radius * sin( phi ) * sin( theta );
	outPoint[ 2 ] = radius * cos( phi );
}

static inline void RandPlanePoint( vec2f& outPoint )
{
	outPoint[ 0 ] = Random();
	outPoint[ 1 ] = Random();
}

// Fowler–Noll–Vo Hash - fnv1a - 32bits
// https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
static inline uint32_t Hash( const uint8_t* bytes, const uint32_t sizeBytes ) {
	uint32_t result = 2166136261;
	const uint32_t prime = 16777619;
	for ( uint32_t i = 0; i < sizeBytes; ++i ) {
		result = ( result ^ bytes[ i ] ) * prime;
	}
	return result;
}

// Polynomial Rolling hash
static inline uint64_t Hash( const std::string& s ) {
	const int p = 31;
	const int m = static_cast<int>( 1e9 + 9 );
	uint64_t hash = 0;
	uint64_t pN = 1;
	const int stringLen = static_cast<int>( s.size() );
	for ( int i = 0; i < stringLen; ++i )
	{
		hash = ( hash + ( s[ i ] - (uint64_t)'a' + 1ull ) * pN ) % m;
		pN = ( pN * p ) % m;
	}
	return hash;
}

static inline vec3f RandomVector( float r = 1.0f )
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


static inline vec3f ReflectVector( const vec3f& n, const vec3f& v )
{
	return ( 2.0f * Dot( v, n ) * n - v );
}


static inline vec3f RefractVector( const vec3f& uv, const vec3f& n, float refractionRatio = 1.0f )
{
	const float dot = std::min( Dot( uv.Reverse(), n ), 1.0f );

	vec3f r0 = refractionRatio * ( uv + dot * n );
	vec3f r1 = -sqrt( fabs( 1.0f - Dot( r0, r0 ) ) ) * n;

	return ( r0 + r1 );
}