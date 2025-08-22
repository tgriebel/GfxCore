/*
* MIT License
*
* Copyright( c ) 2013-2025 Thomas Griebel
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
#include <limits>
#include <iostream>
#include <string>
#include <assert.h>
#include <stdint.h>

class Serializer;

template <size_t D, typename T>
class Vector;

template<size_t D, typename T>
void FlushDenorms( Vector<D, T>& v );

template<size_t D, typename T>
[[nodiscard]]
T Length( const Vector<D, T>& v );

template<size_t D, typename T>
[[nodiscard]]
Vector<D, T> Normalize( const Vector<D, T>& u );

template<size_t D, typename T>
[[nodiscard]]
Vector<D, T> Reverse( const Vector<D, T>& u );

template<size_t D, typename T>
void Fill( Vector<D, T>& v, const T& value );

template<size_t D, typename T>
void Copy( const Vector<D, T>& src, Vector<D, T>& dst );

template<size_t D, typename T>
void Copy( const T src[], Vector<D, T>& dst );

template<size_t D, typename T>
void Copy( const Vector<D, T>& src, const T dst[] );

template<size_t D, typename T>
[[nodiscard]]
Vector<D, T> Add( const Vector<D, T>& a, const Vector<D, T>& b );

template<size_t D, typename T>
[[nodiscard]]
Vector<D, T> Subtract( const Vector<D, T>& a, const Vector<D, T>& b );

template<size_t D, typename T>
[[nodiscard]]
Vector<D, T> Divide( const Vector<D, T>& a, const Vector<D, T>& b );

template<size_t D, typename T>
[[nodiscard]]
Vector<D, T> Divide( const Vector<D, T>& a, const T& b );

template<size_t D, typename T>
[[nodiscard]]
Vector<D, T> Multiply( const Vector<D, T>& a, const Vector<D, T>& b );

template<size_t D, typename T>
[[nodiscard]]
Vector<D, T> Multiply( const Vector<D, T>& a, const T& b );

template<size_t D, typename T>
[[nodiscard]]
Vector<D, T> Multiply( const T& a, const Vector<D, T>& b );

template<size_t D, typename T>
[[nodiscard]]
const Vector<D, T> *const Cast( const T values[ D ] )
{
	return reinterpret_cast< const Vector<D, T> *const >( values );
}

template<size_t D, typename T>
[[nodiscard]]
Vector<D, T> * Cast( T values[ D ] )
{
	return reinterpret_cast< Vector<D, T>* >( values );
}

template<size_t D, typename T>
void Serialize( Serializer* serializer, Vector<D, T>& v );

#define SAFE_OPERATORS _DEBUG

#if SAFE_OPERATORS
static float _vector_trap[ 8 ] = {};
#define TRAP( index, LENGTH )   if ( index >= LENGTH )                                                      \
                                {                                                                           \
                                    assert( false );                                                        \
                                    return *reinterpret_cast<T*>( &_vector_trap[ 0 ] );                     \
                                }
#else
#define TRAP( index, LENGTH )
#endif

#define VECTOR_CORE(D,T)    static constexpr T Epsilon = std::numeric_limits< T >::epsilon() * ( (T)2.0 );											\
							static const size_t Size = D;																							\
                            inline T                Length()                        const { return ::Length( *this ); }								\
                            inline Vector<D, T>     Normalize()	                    const { return ::Normalize( *this ); }							\
                            inline Vector<D, T>     Reverse()	                    const { return ::Reverse( *this ); }							\
                            inline void			    Fill( const T& value )		          { ::Fill( *this, value ); }								\
                            inline void             FlushDenorms()                        { ::FlushDenorms( *this ); }								\
                            inline const T&         operator[]( const size_t i )    const { TRAP( i, D ) return elements[ i ]; }					\
                            inline T&               operator[]( const size_t i )          { TRAP( i, D ) return elements[ i ]; }					\
							inline Vector<D, T>&    operator=( const Vector<D, T>& u )    { if ( this != &u ) { Copy( u, *this ); } return *this; }	\
							inline Vector<D, T>&    operator+=( const Vector<D, T>& u )   { *this = Add( *this, u ); return *this; }				\
                            inline Vector<D, T>&    operator-=( const Vector<D, T>& u )   { *this = Subtract( *this, u ); return *this; }			\
                            inline Vector<D, T>&    operator*=( const T& s )              { *this = Multiply( *this, s ); return *this; }			\
                            inline Vector<D, T>&    operator/=( const T& s )              { *this = Divide( *this, s ); return *this; }				\
							void					Serialize( Serializer* serializer )	  { ::Serialize( serializer, *this ); }

#define VECTOR_INIT(D,T)    Vector()                            { Fill( 0 ); };										\
                            Vector( const T& value )            { Fill( value ); };									\
                            Vector( const Vector<D, T>& vec )   { if ( this != &vec ) { Copy( vec, *this ); } }		\
                            Vector( T values[] )                { Copy( values, *this ); }

#define VECTOR_ASSERTS(D,T)	static_assert( sizeof( elements ) == ( D * sizeof( T ) ), "Must be plain-old-data." );

#define VECTOR_COMMON(D,T)	VECTOR_CORE(D,T)	\
							VECTOR_INIT(D,T)	\
							VECTOR_ASSERTS(D,T)

/* 
    Generic Vector Class
*/
template <size_t D, typename T>
class Vector
{
public:

	union
	{
		T elements[ D ];
	};

	VECTOR_COMMON( D, T )
};

/*
	Specialized Vector Classes for 3D graphics
*/
template <typename T>
class Vector<2, T>
{
public:

	union
	{
		T elements[ 2 ];

		struct
		{
			T x;
			T y;
		};
	};

	VECTOR_COMMON( 2, T )

	Vector( const T& x_, const T& y_ )
	{
		x = x_;
		y = y_;
	}

	Vector( const Vector<3, T>& vec )
	{
		x = vec.x;
		y = vec.y;
	}
};

template <typename T>
class Vector<3, T>
{
public:

	union
	{
		T elements[ 3 ];

		struct
		{
			T x;
			T y;
			T z;
		};

		struct
		{
			Vector<2, T> xy;
			T _alias0;
		};

		struct
		{
			T _alias1;
			Vector<2, T> yz;
		};
	};

	VECTOR_COMMON( 3, T )

	Vector( const T& x_, const T& y_, const T& z_ )
	{
		x = x_;
		y = y_;
		z = z_;
	}

	Vector( const Vector<2, T>& vec, T z_ )
	{
		x = vec.x;
		y = vec.y;
		z = z_;
	}
};

template <typename T>
class Vector<4, T>
{
public:

	union
	{
		T elements[ 4 ];

		struct
		{
			T x;
			T y;
			T z;
			T w;
		};

		struct
		{
			Vector<2, T> xy;
			Vector<2, T> zw;
		};

		struct
		{
			T _alias0;
			Vector<2, T> yz;
			T _alias1;
		};

		struct
		{
			Vector<3, T> xyz;
			T _alias2;
		};

		struct
		{
			T _alias3;
			Vector<3, T> yzw;
		};
	};

	VECTOR_COMMON( 4, T )

	Vector( const T& x_, const T& y_, const T& z_, const T& w_ )
	{
		x = x_;
		y = y_;
		z = z_;
		w = w_;
	}

	Vector( const Vector<3, T>& vec, T w_ )
	{
		x = vec.x;
		y = vec.y;
		z = vec.z;
		w = w_;
	}
};

/*
	Shorthand types
*/
using vec2i = Vector<2, int32_t>;
using vec3i = Vector<3, int32_t>;
using vec4i = Vector<4, int32_t>;
using vec2u = Vector<2, uint32_t>;
using vec3u = Vector<3, uint32_t>;
using vec4u = Vector<4, uint32_t>;
using vec2f = Vector<2, float>;
using vec3f = Vector<3, float>;
using vec4f = Vector<4, float>;
using vec2d = Vector<2, double>;
using vec3d = Vector<3, double>;
using vec4d = Vector<4, double>;

/*
	Generic Implementations
*/
template<size_t D, typename T>
void FlushDenorms( Vector<D, T>& v )
{
	for ( size_t i = 0; i < D; ++i )
	{
		const int code = std::fpclassify( v[ i ] );
		if ( ( code == FP_SUBNORMAL ) || ( code == FP_NAN ) )
		{
			v[ i ] = static_cast<T>( 0.0 );
		}
	}
}


template<size_t D, typename T>
T Length( const Vector<D, T>& v )
{
	T mag = 0.0;
	for ( size_t i = 0; i < D; ++i ) {
		mag += v[ i ] * v[ i ];
	}
	return sqrt( mag );
}


template<size_t D, typename T>
Vector<D, T> Normalize( const Vector<D, T>& u )
{
	Vector< D, T> t;

	T m = Length( u );
	if ( m <= Vector<D, T>::Epsilon ) {
		m = static_cast<T>( 1.0 );
	}

	for ( size_t i = 0; i < D; ++i )
	{
		t[ i ] = u[ i ] / m;
		if ( fabs( t[ i ] ) <= Vector<D, T>::Epsilon ) {
			t[ i ] = static_cast<T>( 0.0 );
		}
	}
	return t;
}


template<size_t D, typename T>
Vector<D, T> Reverse( const Vector<D, T>& u )
{
	Vector< D, T> t;
	for ( size_t i = 0; i < D; ++i ) {
		t[ i ] = -u[ i ];
	}
	return t;
}


template<size_t D, typename T>
void Fill( Vector<D, T>& v, const T& value )
{
	for ( size_t i = 0; i < D; ++i ) {
		v[ i ] = value;
	}
}


template<size_t D, typename T>
void Copy( const Vector<D, T>& src, Vector<D, T>& dst )
{
	for ( size_t i = 0; i < D; ++i ) {
		dst[ i ] = src[ i ];
	}
}


template<size_t D, typename T>
void Copy( const T src[], Vector<D, T>& dst )
{
	for ( size_t i = 0; i < D; ++i ) {
		dst[ i ] = src[ i ];
	}
}


template<size_t D, typename T>
void Copy( const Vector<D, T>& src, const T dst[] )
{
	for ( size_t i = 0; i < D; ++i ) {
		dst[ i ] = src[ i ];
	}
}


template<size_t D, typename T>
[[nodiscard]]
Vector<D, T> Add( const Vector<D, T>& a, const Vector<D, T>& b )
{
	Vector<D, T> c;
	for ( size_t i = 0; i < D; ++i ) {
		c[ i ] = a[ i ] + b[ i ];
	}
	return c;
}


template<size_t D, typename T>
[[nodiscard]]
Vector<D, T> Subtract( const Vector<D, T>& a, const Vector<D, T>& b )
{
	Vector<D, T> c;

	for ( size_t i = 0; i < D; ++i ) {
		c[ i ] = a[ i ] - b[ i ];
	}
	return c;
}


template<size_t D, typename T>
[[nodiscard]]
Vector<D, T> Divide( const Vector<D, T>& a, const Vector<D, T>& b )
{
	Vector<D, T> c;

	for ( size_t i = 0; i < D; ++i ) {
		c[ i ] = a[ i ] / b[ i ];
	}
	return c;
}


template<size_t D, typename T>
[[nodiscard]]
Vector<D, T> Divide( const Vector<D, T>& a, const T& b )
{
	Vector<D, T> c;

	for ( size_t i = 0; i < D; ++i ) {
		c[ i ] = a[ i ] / b;
	}
	return c;
}


template<size_t D, typename T>
[[nodiscard]]
Vector<D, T> Multiply( const Vector<D, T>& a, const Vector<D, T>& b )
{
	Vector<D, T> c;

	for ( size_t i = 0; i < D; ++i ) {
		c[ i ] = a[ i ] * b[ i ];
	}
	return c;
}


template<size_t D, typename T>
[[nodiscard]]
Vector<D, T> Multiply( const Vector<D, T>& a, const T& b )
{
	Vector<D, T> c;

	for ( size_t i = 0; i < D; ++i ) {
		c[ i ] = a[ i ] * b;
	}
	return c;
}


template<size_t D, typename T>
[[nodiscard]]
Vector<D, T> Multiply( const T& a, const Vector<D, T>& b )
{
	return Multiply( b, a );
}


template<typename T>
Vector<3, T> Cross( const Vector<3, T>& u, const Vector<3, T>& v )
{
	Vector<3, T> w;
	w[ 0 ] = u[ 1 ] * v[ 2 ] - u[ 2 ] * v[ 1 ];
	w[ 1 ] = u[ 2 ] * v[ 0 ] - u[ 0 ] * v[ 2 ];
	w[ 2 ] = u[ 0 ] * v[ 1 ] - u[ 1 ] * v[ 0 ];
	return w;
}


template< size_t D, typename T>
T Angle( const Vector<D, T>& u, const Vector<D, T>& v )
{
	const T PI = static_cast<T>( 3.14159265358979323846 );

	T theta = static_cast<T>( 0.0 );
	T mag = u.Length() * v.Length();

	if ( mag > Vector<D, T>::Epsilon )
	{
		T result = Dot( u, v ) / mag;

		if ( result >= ( (T)-1.0 - Vector<D, T>::Epsilon ) && result <= ( (T)1.0 + Vector<D, T>::Epsilon ) ) {
			theta = acos( result ) * ( (T)180.0 / (T)PI );
		}
	}
	return theta;
}


template<typename T>
T TripleScalar( const Vector<3, T>& u, const Vector<3, T>& v, const Vector<3, T>& w )
{
	return Dot( Cross( u, v ), w );
}


template<size_t D, typename T>
bool operator==( const Vector<D, T>& u, const Vector<D, T>& v )
{
	for ( size_t i = 0; i < D; ++i ) {
		if ( u[ i ] != v[ i ] ) {
			return false;
		}
	}
	return true;
}


template<size_t D, typename T>
bool operator !=( const Vector<D, T>& u, const Vector<D, T>& v )
{
	return !( u == v );
}


template<size_t D, typename T>
Vector<D, T> operator+( const Vector<D, T>& u, const Vector<D, T>& v )
{
	return Add( u, v );
}


template<size_t D, typename T>
Vector<D, T> operator-( const Vector<D, T>& u, const Vector<D, T>& v )
{
	return Subtract( u, v );
}


template<size_t D, typename T>
T Dot( const Vector<D, T>& u, const Vector<D, T>& v )
{
	T dot( 0.0 );
	for ( size_t i = 0; i < D; ++i ) {
		dot += u[ i ] * v[ i ];
	}
	return dot;
}


template<size_t D, typename T>
Vector<D, T> operator*( T s, const Vector<D, T>& u )
{
	return Multiply( s, u );
}


template< size_t D, typename T >
Vector<D, T> operator*( const Vector<D, T>& u, T s )
{
	return Multiply( u, s );
}


template< size_t D, typename T >
Vector<D, T> operator/( const Vector<D, T>& u, T s )
{
	return Divide( u, s );
}


template<size_t SrcLength, size_t TruncNum, typename T, size_t DestLength = ( SrcLength - TruncNum )>
Vector<DestLength, T> Trunc( const Vector< SrcLength, T>& u )
{
	Vector<DestLength, T> dstVec;

	for( size_t i = 0; i < DestLength; ++i ) {
		dstVec[ i ] = u[ i ];
	}
	return dstVec;
}


template<size_t SrcLength, size_t ConcatNum, typename T, size_t DestLength = ( SrcLength + ConcatNum )>
Vector<DestLength, T> Concat( const Vector<SrcLength, T>& u, const T fillValue = static_cast<T>( 0.0 ) )
{
	Vector<DestLength, T> dstVec;

	for ( size_t i = 0; i < SrcLength; ++i ) {
		dstVec[ i ] = u[ i ];
	}

	for ( size_t i = SrcLength; i < DestLength; ++i ) {
		dstVec[ i ] = fillValue;
	}
	return dstVec;
}


template<size_t D, typename T >
std::ostream& operator<<( std::ostream& stream, const Vector< D, T>& v )
{
	stream << "[";
	for ( size_t i = 0; i < D; ++i ) {
		stream << v[ i ] << ( ( i + 1 < D ) ? ", " : "" );
	}
	stream << " ]";
	return stream;
}