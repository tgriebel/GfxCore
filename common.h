#pragma once

#define _USE_MATH_DEFINES
#include <cmath>

static double PI = 3.14159265358979323846;

template<typename T>
inline T Min( const T& a, const T& b )
{
	return ( a < b ) ? a : b;
}

template<typename T>
inline T Max( const T& a, const T& b )
{
	return ( a > b ) ? a : b;
}

template<typename T>
inline T Clamp( const T& number, const T& min, const T& max )
{
	const T tmp = ( number < min ) ? min : number;
	return ( tmp > max ) ? max : tmp;
}


template<typename T>
inline T Saturate( const T& number )
{
	return Clamp( number, static_cast<T>( 0.0 ), static_cast<T>( 1.0 ) );
}


template<typename T1, typename T2>
inline T1 Lerp( const T1& v0, const T1& v1, T2 t )
{
	t = Saturate( t );
	T2 one = T2( static_cast<T2>( 1.0 ) );
	return ( one - t ) * v0 + t * v1;
}


template<typename T>
inline T Radians( const T& degrees )
{
	return ( degrees * ( PI / 180.0 ) );
}


template<typename T>
inline T Degrees( const T& radians )
{
	return ( radians * ( 180.0 / PI ) );
}