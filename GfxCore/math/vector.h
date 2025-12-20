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

// Dummy types for template specialization
class PodStorage {};

template<size_t M, size_t N>
class ViewStorage {};

class SparseStorage {};

template <size_t D, typename T, typename S>
class Vector;

template<size_t D, typename T, typename S>
void FlushDenorms( Vector<D, T, S>& v );

template <size_t D, typename T, typename S>
void Flush( Vector<D, T, S>& v, const T tolerance );

template<size_t D, typename T, typename S>
[[nodiscard]]
T Length( const Vector<D, T, S>& v );

template<size_t D, typename T, typename S>
[[nodiscard]]
Vector<D, T, S> Normalize( const Vector<D, T, S>& u );

template<size_t D, typename T, typename S>
[[nodiscard]]
Vector<D, T, S> Reverse( const Vector<D, T, S>& u );

template<size_t D, typename T, typename S>
void Fill( Vector<D, T, S>& v, const T& value );

template<size_t D, typename T, typename S>
void FillRandom( Vector<D, T, S>& v );

template<size_t D, typename T, typename S>
void Copy( const Vector<D, T, S>& src, Vector<D, T, S>& dst );

template<size_t D, typename T, typename S>
void Copy( const T src[], Vector<D, T, S>& dst );

template<size_t D, typename T, typename S>
void Copy( const Vector<D, T, S>& src, const T dst[] );

template<size_t D, typename T, typename S>
[[nodiscard]]
Vector<D, T, S> Add( const Vector<D, T, S>& a, const Vector<D, T, S>& b );

template<size_t D, typename T, typename S>
[[nodiscard]]
Vector<D, T, S> Subtract( const Vector<D, T, S>& a, const Vector<D, T, S>& b );

template<size_t D, typename T, typename S>
[[nodiscard]]
Vector<D, T, S> Divide( const Vector<D, T, S>& a, const Vector<D, T, S>& b );

template<size_t D, typename T, typename S>
[[nodiscard]]
Vector<D, T, S> Divide( const Vector<D, T, S>& a, const T& b );

template<size_t D, typename T, typename S>
[[nodiscard]]
Vector<D, T, S> Multiply( const Vector<D, T, S>& a, const Vector<D, T, S>& b );

template<size_t D, typename T, typename S>
[[nodiscard]]
Vector<D, T, S> Multiply( const Vector<D, T, S>& a, const T& b );

template<size_t D, typename T, typename S>
[[nodiscard]]
Vector<D, T, S> Multiply( const T& a, const Vector<D, T, S>& b );

template<size_t D, typename T>
[[nodiscard]]
const Vector<D, T, PodStorage> *const Cast( const T values[ D ] )
{
	return reinterpret_cast< const Vector<D, T, PodStorage> *const >( values );
}

template<size_t D, typename T>
[[nodiscard]]
Vector<D, T, PodStorage> * Cast( T values[ D ] )
{
	return reinterpret_cast< Vector<D, T, PodStorage>* >( values );
}

template<size_t SourceDim, size_t Dim, typename T>
[[nodiscard]]
Vector<Dim, T, PodStorage>* const Cast( Vector<SourceDim, T, PodStorage>& v, const size_t offset = 0 )
{
	static_assert( SourceDim >= Dim, "Must be equal or smaller than source dimensions." );

	assert( ( offset + Dim ) <= SourceDim );

	return reinterpret_cast<Vector<Dim, T, PodStorage>*>( &( v[ offset ] ) );
}

template<size_t SourceDim, size_t Dim, typename T>
[[nodiscard]]
const Vector<Dim, T, PodStorage>* const Cast( const Vector<SourceDim, T, PodStorage>& v, const size_t offset = 0 )
{
	static_assert( SourceDim >= Dim, "Must be equal or smaller than source dimensions" );

	assert( ( offset + Dim ) <= SourceDim );

	return reinterpret_cast<const Vector<Dim, T, PodStorage>*>( &( v[ offset ] ) );
}

template <size_t D, typename T, typename S>
void Serialize( Serializer* serializer, Vector<D, T, S>& v );

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

#define VECTOR_CORE(D,T,S)  static constexpr T Epsilon = std::numeric_limits< T >::epsilon() * ( (T)2.0 );													\
							static const size_t Size = D;																									\
							typedef T ValueType;																											\
                            inline T					Length()                        const	{ return ::Length( *this ); }								\
                            inline Vector<D, T, S>		Normalize()	                    const	{ return ::Normalize( *this ); }							\
                            inline Vector<D, T, S>		Reverse()						const	{ return ::Reverse( *this ); }								\
                            inline void					Fill( const T& value )					{ ::Fill( *this, value ); }									\
                            inline const T&				operator[]( const size_t i )	const	{ TRAP( i, D ) return elements[ i ]; }						\
                            inline T&					operator[]( const size_t i )			{ TRAP( i, D ) return elements[ i ]; }						\
							inline Vector<D, T, S>&		operator=( const Vector<D, T, S>& u )	{ if ( this != &u ) { Copy( u, *this ); } return *this; }	\
							inline Vector<D, T, S>&		operator+=( const Vector<D, T, S>& u )	{ *this = Add( *this, u ); return *this; }					\
                            inline Vector<D, T, S>&		operator-=( const Vector<D, T, S>& u )	{ *this = Subtract( *this, u ); return *this; }				\
                            inline Vector<D, T, S>&		operator*=( const T& s )				{ *this = Multiply( *this, s ); return *this; }				\
                            inline Vector<D, T, S>&		operator/=( const T& s )				{ *this = Divide( *this, s ); return *this; }				\
							void						Serialize( Serializer* serializer )		{ ::Serialize( serializer, *this ); }

#define VECTOR_INIT(D,T,S)  Vector()                            { Fill( 0 ); };										\
                            Vector( const T& value )            { Fill( value ); };									\
                            Vector( const Vector<D, T,S>& vec )	{ if ( this != &vec ) { Copy( vec, *this ); } }		\
                            Vector( T values[] )                { Copy( values, *this ); }

#define VECTOR_ASSERTS(D,T,S)	static_assert( sizeof( elements ) == ( D * sizeof( T ) ), "Must be plain-old-data." );

#define VECTOR_COMMON(D,T,S)	VECTOR_CORE(D,T,S)	\
								VECTOR_INIT(D,T,S)	\
								VECTOR_ASSERTS(D,T,S)

/* 
    Generic Vector Class
*/
template <size_t D, typename T, typename S>
class Vector
{
public:

	union
	{
		T elements[ D ];
	};

	VECTOR_COMMON( D, T, S )
};

/*
	Specialized Vector Classes for 3D graphics
*/
template <typename T>
class Vector<2, T, PodStorage>
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

	VECTOR_COMMON( 2, T, PodStorage )

	Vector( const T& x_, const T& y_ )
	{
		x = x_;
		y = y_;
	}

	Vector( const Vector<3, T, PodStorage>& vec )
	{
		x = vec.x;
		y = vec.y;
	}
};

template <typename T>
class Vector<3, T, PodStorage>
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
			Vector<2, T, PodStorage> xy;
			T _alias0;
		};

		struct
		{
			T _alias1;
			Vector<2, T, PodStorage> yz;
		};
	};

	VECTOR_COMMON( 3, T, PodStorage )

	Vector( const T& x_, const T& y_, const T& z_ )
	{
		x = x_;
		y = y_;
		z = z_;
	}

	Vector( const Vector<2, T, PodStorage>& vec, T z_ )
	{
		x = vec.x;
		y = vec.y;
		z = z_;
	}
};

template <typename T>
class Vector<4, T, PodStorage>
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
			Vector<2, T, PodStorage> xy;
			Vector<2, T, PodStorage> zw;
		};

		struct
		{
			T _alias0;
			Vector<2, T, PodStorage> yz;
			T _alias1;
		};

		struct
		{
			Vector<3, T, PodStorage> xyz;
			T _alias2;
		};

		struct
		{
			T _alias3;
			Vector<3, T, PodStorage> yzw;
		};
	};

	VECTOR_COMMON( 4, T, PodStorage )

	Vector( const T& x_, const T& y_, const T& z_, const T& w_ )
	{
		x = x_;
		y = y_;
		z = z_;
		w = w_;
	}

	Vector( const Vector<3, T, PodStorage>& vec, T w_ )
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
template<size_t D, typename T>
using VectorP = Vector<D, T, PodStorage>;

template<typename T>
using Vector2 = VectorP<2, T>;

template<typename T>
using Vector3 = VectorP<3, T>;

template<typename T>
using Vector4 = VectorP<4, T>;

using vec2i = Vector2<int32_t>;
using vec3i = Vector3<int32_t>;
using vec4i = Vector4<int32_t>;
using vec2u = Vector2<uint32_t>;
using vec3u = Vector3<uint32_t>;
using vec4u = Vector4<uint32_t>;
using vec2f = Vector2<float>;
using vec3f = Vector3<float>;
using vec4f = Vector4<float>;
using vec2d = Vector2<double>;
using vec3d = Vector3<double>;
using vec4d = Vector4<double>;

/*
	Generic Implementations
*/
template <size_t D, typename T, typename S>
void FlushDenorms( Vector<D, T, S>& v )
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


template <size_t D, typename T, typename S>
void Flush( Vector<D, T, S>& v, const T tolerance )
{
	for ( size_t i = 0; i < D; ++i )
	{
		if ( fabs( v[ i ] ) < tolerance )
		{
			v[ i ] = static_cast<T>( 0.0 );
		}
	}
}


template <size_t D, typename T, typename S>
T Length( const Vector<D, T, S>& v )
{
	T mag = 0.0;
	for ( size_t i = 0; i < D; ++i ) {
		mag += v[ i ] * v[ i ];
	}
	return sqrt( mag );
}


template <size_t D, typename T, typename S>
Vector<D, T, S> Normalize( const Vector<D, T, S>& u )
{
	Vector<D, T, S> t;

	T m = Length( u );
	if ( m <= Vector<D, T, S>::Epsilon ) {
		m = static_cast<T>( 1.0 );
	}

	for ( size_t i = 0; i < D; ++i )
	{
		t[ i ] = u[ i ] / m;
		if ( fabs( t[ i ] ) <= Vector<D, T, S>::Epsilon ) {
			t[ i ] = static_cast<T>( 0.0 );
		}
	}
	return t;
}


template <size_t D, typename T, typename S>
Vector<D, T, S> Reverse( const Vector<D, T, S>& u )
{
	Vector<D, T, S> t;
	for ( size_t i = 0; i < D; ++i ) {
		t[ i ] = -u[ i ];
	}
	return t;
}


template <size_t D, typename T, typename S>
void Fill( Vector<D, T, S>& v, const T& value )
{
	for ( size_t i = 0; i < D; ++i ) {
		v[ i ] = value;
	}
}


template <size_t D, typename T, typename S>
void FillRandom( Vector<D, T, S>& v )
{
	for ( size_t i = 0; i < D; ++i ) {
		v[ i ] = ( rand() / static_cast<T>( RAND_MAX ) );
	}
}


template <size_t D, typename T, typename S>
void Copy( const Vector<D, T, S>& src, Vector<D, T, S>& dst )
{
	for ( size_t i = 0; i < D; ++i ) {
		dst[ i ] = src[ i ];
	}
}


template <size_t D, typename T, typename S>
void Copy( const T src[], Vector<D, T, S>& dst )
{
	for ( size_t i = 0; i < D; ++i ) {
		dst[ i ] = src[ i ];
	}
}


template <size_t D, typename T, typename S>
void Copy( const Vector<D, T, S>& src, const T dst[] )
{
	for ( size_t i = 0; i < D; ++i ) {
		dst[ i ] = src[ i ];
	}
}


template <size_t D, typename T, typename S>
[[nodiscard]]
Vector<D, T, S> Add( const Vector<D, T, S>& a, const Vector<D, T, S>& b )
{
	Vector<D, T, S> c;
	for ( size_t i = 0; i < D; ++i ) {
		c[ i ] = a[ i ] + b[ i ];
	}
	return c;
}


template <size_t D, typename T, typename S>
[[nodiscard]]
Vector<D, T, S> Subtract( const Vector<D, T, S>& a, const Vector<D, T, S>& b )
{
	Vector<D, T, S> c;

	for ( size_t i = 0; i < D; ++i ) {
		c[ i ] = a[ i ] - b[ i ];
	}
	return c;
}


template <size_t D, typename T, typename S>
[[nodiscard]]
Vector<D, T, S> Divide( const Vector<D, T, S>& a, const Vector<D, T, S>& b )
{
	Vector<D, T, S> c;

	for ( size_t i = 0; i < D; ++i ) {
		c[ i ] = a[ i ] / b[ i ];
	}
	return c;
}


template <size_t D, typename T, typename S>
[[nodiscard]]
Vector<D, T, S> Divide( const Vector<D, T, S>& a, const T& b )
{
	Vector<D, T, S> c;

	for ( size_t i = 0; i < D; ++i ) {
		c[ i ] = a[ i ] / b;
	}
	return c;
}


template <size_t D, typename T, typename S>
[[nodiscard]]
Vector<D, T, S> Multiply( const Vector<D, T, S>& a, const Vector<D, T, S>& b )
{
	Vector<D, T, S> c;

	for ( size_t i = 0; i < D; ++i ) {
		c[ i ] = a[ i ] * b[ i ];
	}
	return c;
}


template <size_t D, typename T, typename S>
[[nodiscard]]
Vector<D, T, S> Multiply( const Vector<D, T, S>& a, const T& b )
{
	Vector<D, T, S> c;

	for ( size_t i = 0; i < D; ++i ) {
		c[ i ] = a[ i ] * b;
	}
	return c;
}


template <size_t D, typename T, typename S>
[[nodiscard]]
Vector<D, T, S> Multiply( const T& a, const Vector<D, T, S>& b )
{
	return Multiply( b, a );
}


template<typename T, typename S>
Vector<3, T, S> Cross( const Vector<3, T, S>& u, const Vector<3, T, S>& v )
{
	Vector<3, T, S> w;
	w[ 0 ] = u[ 1 ] * v[ 2 ] - u[ 2 ] * v[ 1 ];
	w[ 1 ] = u[ 2 ] * v[ 0 ] - u[ 0 ] * v[ 2 ];
	w[ 2 ] = u[ 0 ] * v[ 1 ] - u[ 1 ] * v[ 0 ];
	return w;
}


template< size_t D, typename T, typename S>
T Angle( const Vector<D, T, S>& u, const Vector<D, T, S>& v )
{
	const T PI = static_cast<T>( 3.14159265358979323846 );

	T theta = static_cast<T>( 0.0 );
	T mag = u.Length() * v.Length();

	if ( mag > Vector<D, T, S>::Epsilon )
	{
		T result = Dot( u, v ) / mag;

		if ( result >= ( (T)-1.0 - Vector<D, T, S>::Epsilon ) && result <= ( (T)1.0 + Vector<D, T, S>::Epsilon ) ) {
			theta = acos( result ) * ( (T)180.0 / (T)PI );
		}
	}
	return theta;
}


template<typename T, typename S>
T TripleScalar( const Vector<3, T, S>& u, const Vector<3, T, S>& v, const Vector<3, T, S>& w )
{
	return Dot( Cross( u, v ), w );
}


template <size_t D, typename T, typename S>
bool operator==( const Vector<D, T, S>& u, const Vector<D, T, S>& v )
{
	for ( size_t i = 0; i < D; ++i ) {
		if ( u[ i ] != v[ i ] ) {
			return false;
		}
	}
	return true;
}


template <size_t D, typename T, typename S>
bool operator!=( const Vector<D, T, S>& u, const Vector<D, T, S>& v )
{
	return !( u == v );
}


template <size_t D, typename T, typename S>
Vector<D, T, S> operator+( const Vector<D, T, S>& u, const Vector<D, T, S>& v )
{
	return Add( u, v );
}


template <size_t D, typename T, typename S>
Vector<D, T, S> operator-( const Vector<D, T, S>& u, const Vector<D, T, S>& v )
{
	return Subtract( u, v );
}


template <size_t D, typename T, typename S>
Vector<D, T, S> operator-( const Vector<D, T, S>& u )
{
	return Reverse( u );
}


template <size_t D, typename T, typename S>
T Dot( const Vector<D, T, S>& u, const Vector<D, T, S>& v )
{
	T dot( 0.0 );
	for ( size_t i = 0; i < D; ++i ) {
		dot += u[ i ] * v[ i ];
	}
	return dot;
}


template <size_t D, typename T, typename S>
Vector<D, T, S> operator*( T s, const Vector<D, T, S>& u )
{
	return Multiply( s, u );
}


template<size_t D, typename T, typename S>
Vector<D, T, S> operator*( const Vector<D, T, S>& u, T s )
{
	return Multiply( u, s );
}


template<size_t D, typename T, typename S>
Vector<D, T, S> operator/( const Vector<D, T, S>& u, T s )
{
	return Divide( u, s );
}


template<size_t SrcLength, size_t TruncNum, typename T, size_t DestLength = ( SrcLength - TruncNum ), typename S>
Vector<DestLength, T, S> Trunc( const Vector< SrcLength, T, S>& u )
{
	Vector<DestLength, T, S> dstVec;

	for( size_t i = 0; i < DestLength; ++i ) {
		dstVec[ i ] = u[ i ];
	}
	return dstVec;
}


template<size_t SrcLength, size_t ConcatNum, typename T, size_t DestLength = ( SrcLength + ConcatNum ), typename S>
Vector<DestLength, T, S> Concat( const Vector<SrcLength, T, S>& u, const T fillValue = static_cast<T>( 0.0 ) )
{
	Vector<DestLength, T, S> dstVec;

	for ( size_t i = 0; i < SrcLength; ++i ) {
		dstVec[ i ] = u[ i ];
	}

	for ( size_t i = SrcLength; i < DestLength; ++i ) {
		dstVec[ i ] = fillValue;
	}
	return dstVec;
}


template<size_t D, typename T, typename S>
std::ostream& operator<<( std::ostream& stream, const Vector<D, T, S>& v )
{
	stream << "[";
	for ( size_t i = 0; i < D; ++i ) {
		stream << v[ i ] << ( ( i + 1 < D ) ? ", " : "" );
	}
	stream << " ]";
	return stream;
}