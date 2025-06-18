/*
* MIT License
*
* Copyright( c ) 2020-2023 Thomas Griebel
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

#define _USE_MATH_DEFINES
#include <cmath>
#include <stdint.h>

static constexpr float PI = 3.14159265358979323846f;

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
constexpr inline T Radians( const T& degrees )
{
	return ( degrees * ( PI / static_cast<T>( 180.0 ) ) );
}


template<typename T>
constexpr inline T Degrees( const T& radians )
{
	return ( radians * ( static_cast<T>( 180.0 ) / PI ) );
}


// https://graphics.stanford.edu/%7Eseander/bithacks.html#RoundUpPowerOf2
inline uint32_t RoundPow2( uint32_t num )
{
	uint32_t v = num;

	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;

	return v;
}