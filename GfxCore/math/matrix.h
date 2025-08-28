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

#include <math.h>
#include "vector.h"

//------------------------------------------------------------------------//
// Matrix class and matrix functions
//------------------------------------------------------------------------//

template< size_t M, size_t N, typename T>
class Matrix
{
private:
	Vector<N, T> rows[ M ];
	static constexpr T Epsilon = std::numeric_limits< T >::epsilon() * ( (T) 2.0 );

	static_assert( sizeof( rows ) == ( M * N * sizeof( T ) ), "Must be plain-old-data." );

public:

	static const size_t RowCount = N;
	static const size_t ColumnCount = M;

	Matrix( T diagonalValue = static_cast<T>( 0.0 ) )
	{
		for ( size_t j = 0; j < N; ++j )
		{
			for ( size_t i = 0; i < M; ++i )
			{
				rows[ j ][ i ] = ( i == j ) ? diagonalValue : static_cast<T>( 0.0 );
			}
		}
	}

	Matrix( const T values[] )
	{
		for ( size_t j = 0; j < N; ++j )
		{
			for ( size_t i = 0; i < M; ++i )
			{
				rows[ j ][ i ] = values[ i + ( j * N ) ];
			}
		}
	}

	static Matrix<N, M, T> Identity()
	{
		return Matrix<N, M, T>( 1.0f );
	}

	static Matrix<N, M, T> Zero()
	{
		return Matrix<N, M, T>( 0.0f );
	}

	Matrix<N, M, T>	Transpose( void );
	Matrix<N, M, T>	Transpose( void ) const;

	Vector<N, T>& operator[]( const size_t i );
	const Vector<N, T>& operator[]( const size_t i ) const;
};

// Convenience class for taking subsets of a matrix to avoid copies
// TODO: Define matrix functions in terms of MatrixView, polymorphism, or templatize them
template< size_t SourceM, size_t SourceN, size_t M, size_t N, typename T>
class MatrixView
{
//	static_assert( ( SourceM >= M ) && ( SourceN >= N ) ), "MatrixView must be subset of source matrix." );

private:
	size_t i, j;
	Vector<N, T>* rows[ M ]; // Allows for discontinuous memory

public:

	static const size_t RowCount = N;
	static const size_t ColumnCount = M;

	MatrixView( const Matrix< SourceM, SourceN, T>& A, const size_t colOffset, const size_t rowOffset ) : 
		i( colOffset ),
		j( rowOffset )
	{
		for ( size_t r = rowOffset; r < M; ++r )
		{
			rows[ r ] = reinterpret_cast< Vector<N, T>* >( &A[ r ] );
		}
	}

	Vector<N, T>& operator[]( size_t i )
	{
		return rows[ i ];
	}

	const Vector<N, T>& operator[]( size_t i ) const
	{
		return rows[ i ];
	}
};

template< typename T>
T Det( const Matrix<2, 2, T>& A );
template< typename T>
T Det( const Matrix<3, 3, T>& A );
template< typename T>
T Det( const Matrix<4, 4, T>& A );

using mat2x2f = Matrix<2, 2, float>;
using mat2x2d = Matrix<2, 2, double>;
using mat3x3f = Matrix<3, 3, float>;
using mat3x3d = Matrix<3, 3, double>;
using mat4x4f = Matrix<4, 4, float>;
using mat4x4d = Matrix<4, 4, double>;

template< size_t M, size_t N, typename T>
Matrix<N, M, T> Matrix<M, N, T>::Transpose( void )
{
	Matrix<N, M, T> MT;
	for ( size_t c = 0; c < N; ++c ) {
		for ( size_t r = 0; r < M; ++r )
		{
			MT[ c ][ r ] = rows[ r ][ c ];
		}
	}
	return MT;
}


template< size_t M, size_t N, typename T>
Matrix<N, M, T> Matrix<M, N, T>::Transpose( void ) const
{
	Matrix<N, M, T> MT;
	for ( size_t c = 0; c < N; ++c )
	{
		for ( size_t r = 0; r < M; ++r )
		{
			MT[ c ][ r ] = rows[ r ][ c ];
		}
	}
	return MT;
}


template< size_t M, size_t N, typename T>
Vector<N, T>& Matrix<M, N, T>::operator[]( const size_t i )
{
	return rows[ i ];
}


template< size_t M, size_t N, typename T>
const Vector<N, T>& Matrix<M, N, T>::operator[]( const size_t i ) const
{
	return rows[ i ];
}


template< size_t M, size_t N, typename T>
[[nodiscard]]
Matrix<M, N, T> operator+( const Matrix<M, N, T>& A, const Matrix<M, N, T>& B )
{
	Matrix<M, N, T> C;
	for ( size_t c( 0 ); c < N; ++c )
	{
		for ( size_t r( 0 ); r < M; ++r )
		{
			C[ c ][ r ] = A[ c ][ r ] + B[ c ][ r ];
		}
	}
	return	C;
}


template< size_t M, size_t N, typename T>
[[nodiscard]]
Matrix<M, N, T> operator-( const Matrix<M, N, T>& A, const Matrix<M, N, T>& B )
{
	Matrix<M, N, T> C;
	for ( size_t c( 0 ); c < N; ++c )
	{
		for ( size_t r( 0 ); r < M; ++r )
		{
			C[ c ][ r ] = A[ c ][ r ] - B[ c ][ r ];
		}
	}
	return	C;
}


template< size_t M, size_t N, typename T>
[[nodiscard]]
Matrix<M, N, T> operator/( const Matrix<M, N, T>& A, T s )
{
	Matrix<M, N, T> B;
	for ( size_t c = 0; c < M; ++c )
	{
		for ( size_t r = 0; r < N; ++r )
		{
			B[ c ][ r ] = A[ c ][ r ] / s;
		}
	}
	return	B;
}


template< size_t M1, size_t N1, size_t N2, typename T>
[[nodiscard]]
Matrix<M1, N2, T> operator*( const Matrix<M1, N1, T>& A, const Matrix<N1, N2, T>& B )
{
	Matrix<M1, N2, T> C;
	for ( size_t r = 0; r < M1; ++r )
	{
		for ( size_t c = 0; c < N2; ++c )
		{
			for ( size_t a = 0; a < M1; ++a )
			{
				C[ r ][ c ] += A[ r ][ a ] * B[ a ][ c ];
			}
		}
	}
	return C;
}


template< size_t M, size_t N, typename T>
[[nodiscard]]
Matrix<M, N, T> operator*( const Matrix<M, N, T>& A, T s )
{
	Matrix<M, N, T> B;
	for ( size_t c = 0; c < N; ++c ) {
		for ( size_t r = 0; r < M; ++r ) {
			B[ c ][ r ] = A[ c ][ r ] * s;
		}
	}
	return	B;
}


template< size_t M, size_t N, typename T>
[[nodiscard]]
Matrix<M, N, T> operator*( T s, const Matrix<M, N, T>& A )
{
	return A * s;
}


template< size_t M, size_t N, typename T>
[[nodiscard]]
Vector<N, T> operator*( const Vector<N, T>& u, const Matrix<M, N, T>& A )
{
	Vector< N, T > v;
	for ( size_t c = 0; c < N; ++c )
	{
		for ( size_t r = 0; r < M; ++r )
		{
			v[ c ] += u[ r ] * A[ r ][ c ];
		}
	}
	return v;
}


template< size_t M, size_t N, typename T>
[[nodiscard]]
Vector<N, T> operator*( const Matrix<M, N, T>& A, const Vector<M, T>& u )
{
	Vector< N, T > v;
	for ( size_t r = 0; r < M; ++r )
	{
		for ( size_t c = 0; c < N; ++c )
		{
			v[ r ] += A[ r ][ c ] * u[ c ];
		}
	}
	return v;
}


template< size_t M, size_t N, typename T>
void Flush( Matrix<M, N, T>& A, const T tolerance )
{
	for ( uint32_t r = 0; r < M; ++r ) {
		Flush( A[ r ], tolerance );
	}
}


template< size_t M, size_t N, typename T>
void Fill( Matrix<M, N, T>& A, const T& value )
{
	for ( uint32_t r = 0; r < M; ++r ) {
		Fill( A[ r ], value );
	}
}


template< size_t M, size_t N, typename T>
void FillRandom( Matrix<M, N, T>& A )
{
	for ( uint32_t r = 0; r < M; ++r ) {
		FillRandom( A[ r ] );
	}
}


template< typename T>
[[nodiscard]]
T Det( const Matrix<2, 2, T>& A )
{
	return A[ 0 ][ 0 ] * A[ 1 ][ 1 ] - A[ 1 ][ 0 ] * A[ 0 ][ 1 ];
}


template< typename T>
[[nodiscard]]
T Det( const Matrix<3, 3, T>& A )
{
	T cof00[] = { A[ 1 ][ 1 ], A[ 1 ][ 2 ], A[ 2 ][ 1 ], A[ 2 ][ 2 ] };
	T cof01[] = { A[ 1 ][ 0 ], A[ 1 ][ 2 ], A[ 2 ][ 0 ], A[ 2 ][ 2 ] };
	T cof02[] = { A[ 1 ][ 0 ], A[ 1 ][ 1 ], A[ 2 ][ 0 ], A[ 2 ][ 1 ] };

	return A[ 0 ][ 0 ] * Det( Matrix< 2, 2, T >( cof00 ) ) - A[ 0 ][ 1 ] * Det( Matrix< 2, 2, T >( cof01 ) ) + A[ 0 ][ 2 ] * Det( Matrix< 2, 2, T >( cof02 ) );
}


template< typename T>
[[nodiscard]]
T Det( const Matrix<4, 4, T>& A )
{
	T cof00[] = { A[ 1 ][ 1 ], A[ 1 ][ 2 ], A[ 1 ][ 3 ],  A[ 2 ][ 1 ], A[ 2 ][ 2 ], A[ 2 ][ 3 ],  A[ 3 ][ 1 ], A[ 3 ][ 2 ], A[ 3 ][ 3 ] };
	T cof01[] = { A[ 1 ][ 0 ], A[ 1 ][ 2 ], A[ 1 ][ 3 ],  A[ 2 ][ 0 ], A[ 2 ][ 2 ], A[ 2 ][ 3 ],  A[ 3 ][ 0 ], A[ 3 ][ 2 ], A[ 3 ][ 3 ] };
	T cof02[] = { A[ 1 ][ 0 ], A[ 1 ][ 1 ], A[ 1 ][ 3 ],  A[ 2 ][ 0 ], A[ 2 ][ 1 ], A[ 2 ][ 3 ],  A[ 3 ][ 0 ], A[ 3 ][ 1 ], A[ 3 ][ 3 ] };
	T cof03[] = { A[ 1 ][ 0 ], A[ 1 ][ 1 ], A[ 1 ][ 2 ],  A[ 2 ][ 0 ], A[ 2 ][ 1 ], A[ 2 ][ 2 ],  A[ 3 ][ 0 ], A[ 3 ][ 1 ], A[ 3 ][ 2 ] };

	return A[ 0 ][ 0 ] * Det( Matrix< 3, 3, T >( cof00 ) ) - A[ 0 ][ 1 ] * Det( Matrix< 3, 3, T >( cof01 ) ) + A[ 0 ][ 2 ] * Det( Matrix< 3, 3, T >( cof02 ) ) - A[ 0 ][ 3 ] * Det( Matrix< 3, 3, T >( cof03 ) );
}


template< typename T>
[[nodiscard]]
Matrix<4, 4, T> CofactorMatrix( const Matrix<4, 4, T>& A )
{
	// 00 01 02 03
	// 10 11 12 13
	// 20 21 22 23
	// 30 31 32 33

	//TODO: use cramer's rule combine with determinant

	T minor00[] = { A[ 1 ][ 1 ], A[ 1 ][ 2 ], A[ 1 ][ 3 ],  A[ 2 ][ 1 ], A[ 2 ][ 2 ], A[ 2 ][ 3 ],  A[ 3 ][ 1 ], A[ 3 ][ 2 ], A[ 3 ][ 3 ] };
	T minor01[] = { A[ 1 ][ 0 ], A[ 1 ][ 2 ], A[ 1 ][ 3 ],  A[ 2 ][ 0 ], A[ 2 ][ 2 ], A[ 2 ][ 3 ],  A[ 3 ][ 0 ], A[ 3 ][ 2 ], A[ 3 ][ 3 ] };
	T minor02[] = { A[ 1 ][ 0 ], A[ 1 ][ 1 ], A[ 1 ][ 3 ],  A[ 2 ][ 0 ], A[ 2 ][ 1 ], A[ 2 ][ 3 ],  A[ 3 ][ 0 ], A[ 3 ][ 1 ], A[ 3 ][ 3 ] };
	T minor03[] = { A[ 1 ][ 0 ], A[ 1 ][ 1 ], A[ 1 ][ 2 ],  A[ 2 ][ 0 ], A[ 2 ][ 1 ], A[ 2 ][ 2 ],  A[ 3 ][ 0 ], A[ 3 ][ 1 ], A[ 3 ][ 2 ] };

	T minor10[] = { A[ 0 ][ 1 ], A[ 0 ][ 2 ], A[ 0 ][ 3 ],  A[ 2 ][ 1 ], A[ 2 ][ 2 ], A[ 2 ][ 3 ],  A[ 3 ][ 1 ], A[ 3 ][ 2 ], A[ 3 ][ 3 ] };
	T minor11[] = { A[ 0 ][ 0 ], A[ 0 ][ 2 ], A[ 0 ][ 3 ],  A[ 2 ][ 0 ], A[ 2 ][ 2 ], A[ 2 ][ 3 ],  A[ 3 ][ 0 ], A[ 3 ][ 2 ], A[ 3 ][ 3 ] };
	T minor12[] = { A[ 0 ][ 0 ], A[ 0 ][ 1 ], A[ 0 ][ 3 ],  A[ 2 ][ 0 ], A[ 2 ][ 1 ], A[ 2 ][ 3 ],  A[ 3 ][ 0 ], A[ 3 ][ 1 ], A[ 3 ][ 3 ] };
	T minor13[] = { A[ 0 ][ 0 ], A[ 0 ][ 1 ], A[ 0 ][ 2 ],  A[ 2 ][ 0 ], A[ 2 ][ 1 ], A[ 2 ][ 2 ],  A[ 3 ][ 0 ], A[ 3 ][ 1 ], A[ 3 ][ 2 ] };

	T minor20[] = { A[ 0 ][ 1 ], A[ 0 ][ 2 ], A[ 0 ][ 3 ],  A[ 1 ][ 1 ], A[ 1 ][ 2 ], A[ 1 ][ 3 ],  A[ 3 ][ 1 ], A[ 3 ][ 2 ], A[ 3 ][ 3 ] };
	T minor21[] = { A[ 0 ][ 0 ], A[ 0 ][ 2 ], A[ 0 ][ 3 ],  A[ 1 ][ 0 ], A[ 1 ][ 2 ], A[ 1 ][ 3 ],  A[ 3 ][ 0 ], A[ 3 ][ 2 ], A[ 3 ][ 3 ] };
	T minor22[] = { A[ 0 ][ 0 ], A[ 0 ][ 1 ], A[ 0 ][ 3 ],  A[ 1 ][ 0 ], A[ 1 ][ 1 ], A[ 1 ][ 3 ],  A[ 3 ][ 0 ], A[ 3 ][ 1 ], A[ 3 ][ 3 ] };
	T minor23[] = { A[ 0 ][ 0 ], A[ 0 ][ 1 ], A[ 0 ][ 2 ],  A[ 1 ][ 0 ], A[ 1 ][ 1 ], A[ 1 ][ 2 ],  A[ 3 ][ 0 ], A[ 3 ][ 1 ], A[ 3 ][ 2 ] };

	T minor30[] = { A[ 0 ][ 1 ], A[ 0 ][ 2 ], A[ 0 ][ 3 ],  A[ 1 ][ 1 ], A[ 1 ][ 2 ], A[ 1 ][ 3 ],  A[ 2 ][ 1 ], A[ 2 ][ 2 ], A[ 2 ][ 3 ] };
	T minor31[] = { A[ 0 ][ 0 ], A[ 0 ][ 2 ], A[ 0 ][ 3 ],  A[ 1 ][ 0 ], A[ 1 ][ 2 ], A[ 1 ][ 3 ],  A[ 2 ][ 0 ], A[ 2 ][ 2 ], A[ 2 ][ 3 ] };
	T minor32[] = { A[ 0 ][ 0 ], A[ 0 ][ 1 ], A[ 0 ][ 3 ],  A[ 1 ][ 0 ], A[ 1 ][ 1 ], A[ 1 ][ 3 ],  A[ 2 ][ 0 ], A[ 2 ][ 1 ], A[ 2 ][ 3 ] };
	T minor33[] = { A[ 0 ][ 0 ], A[ 0 ][ 1 ], A[ 0 ][ 2 ],  A[ 1 ][ 0 ], A[ 1 ][ 1 ], A[ 1 ][ 2 ],  A[ 2 ][ 0 ], A[ 2 ][ 1 ], A[ 2 ][ 2 ] };

	T values[] = { Det( Matrix< 3, 3, T >( minor00 ) ), -Det( Matrix< 3, 3, T >( minor01 ) ), Det( Matrix< 3, 3, T >( minor02 ) ), -Det( Matrix< 3, 3, T >( minor03 ) ),
						-Det( Matrix< 3, 3, T >( minor10 ) ), Det( Matrix< 3, 3, T >( minor11 ) ), -Det( Matrix< 3, 3, T >( minor12 ) ), Det( Matrix< 3, 3, T >( minor13 ) ),
						Det( Matrix< 3, 3, T >( minor20 ) ), -Det( Matrix< 3, 3, T >( minor21 ) ), Det( Matrix< 3, 3, T >( minor22 ) ), -Det( Matrix< 3, 3, T >( minor23 ) ),
						-Det( Matrix< 3, 3, T >( minor30 ) ), Det( Matrix< 3, 3, T >( minor31 ) ), -Det( Matrix< 3, 3, T >( minor32 ) ), Det( Matrix< 3, 3, T >( minor33 ) ) };

	return mat4x4f( values );
}

template< typename T>
[[nodiscard]]
Matrix<3, 3, T> Invert( const Matrix<3, 3, T>& A, bool& invertible )
{
	T det_val = Det( A );

	if ( det_val == 0 )
	{
		invertible = false;
		return Matrix<3, 3, T>();
	}
	invertible = true;
	assert( false ); // TODO: transpose of cofactor matrix not transpose of matrix
	return A.Transpose() * ( 1. / det_val );
}

template< typename T>
[[nodiscard]]
Matrix<4, 4, T> Invert( const Matrix<4, 4, T>& A, bool& invertible )
{
	T det_val = Det( A );

	if ( det_val == 0 )
	{
		invertible = false;
		return Matrix<4, 4, T>();
	}
	invertible = true;

	return ( CofactorMatrix( A ).Transpose() ) * ( static_cast<T>( 1.0 ) / det_val );
}

template< size_t M, size_t N, typename T>
T Convolution( const Matrix<M, N, T>& A, const Matrix<M, N, T>& B )
{
	T sum = 0;
	for ( size_t r = 0; r < M; ++r )
	{
		for ( size_t c = 0; c < N; ++c )
		{
			sum += A[ r ][ c ] * B[ r ][ c ];
		}
	}
	return sum;
}


template< size_t M, size_t N, typename T>
[[nodiscard]]
Matrix<M, N, T> Identity()
{
	return Matrix<M, N, T>( static_cast<T>( 1.0 ) );
}


template< size_t M, size_t N, typename T>
[[nodiscard]]
bool IsInvertible( const Matrix<N, N, T>& m )
{
	return ( Det( m ) != 0 );
}


template< size_t M, size_t N, typename T>
[[nodiscard]]
bool IsOrthonormal( const Matrix<N, N, T>& A, const float tolerance )
{
	Matrix<M, N, T> I = A * A.Transpose();
	return IsIdentity( I, tolerance );
}


template< size_t N, typename T>
[[nodiscard]]
bool IsIdentity( const Matrix<N, N, T>& A, const T tolerance )
{
	for ( size_t r = 0; r < N; ++r )
	{
		for ( size_t c = 0; c < N; ++c )
		{
			if ( ( c == r ) && ( fabs( A[ r ][ c ] - 1.0 ) > tolerance ) )
			{
				return false;
			}
			else if ( ( c != r ) && ( fabs( A[ r ][ c ] - 0.0 ) > tolerance ) )
			{
				return false;
			}
		}
	}
	return true;
}


template< size_t M, size_t N, typename T >
std::ostream& operator<<( std::ostream& stream, const Matrix<M, N, T>& A )
{
	stream << "[";
	for ( size_t r = 0; r < M; ++r ) {
		stream << A[ r ] << ( ( r + 1 < M ) ? ",\n" : "" );
	}
	stream << "]";
	return stream;
}

template< typename T >
[[nodiscard]]
Matrix<2, 2, T> CreateMatrix2x2(	T m00, T m01,
									T m10, T m11 )
{
	Matrix<2, 2, T> A;

	A[ 0 ][ 0 ] = m00;
	A[ 0 ][ 1 ] = m01;

	A[ 1 ][ 0 ] = m10;
	A[ 1 ][ 1 ] = m11;

	return A;
}

template< typename T >
[[nodiscard]]
Matrix<3, 3, T> CreateMatrix3x3(	T m00, T m01, T m02,
									T m10, T m11, T m12,
									T m20, T m21, T m22 )
{
	Matrix<3, 3, T> A;

	A[ 0 ][ 0 ] = m00;
	A[ 0 ][ 1 ] = m01;
	A[ 0 ][ 2 ] = m02;

	A[ 1 ][ 0 ] = m10;
	A[ 1 ][ 1 ] = m11;
	A[ 1 ][ 2 ] = m12;

	A[ 2 ][ 0 ] = m20;
	A[ 2 ][ 1 ] = m21;
	A[ 2 ][ 2 ] = m22;

	return A;
}


template< typename T >
[[nodiscard]]
Matrix<3, 3, T> CreateMatrix3x3( const Vector<3, T>& X, const Vector<3, T>& Y, const Vector<3, T>& Z )
{
	Matrix<3, 3, T> A;

	A[ 0 ][ 0 ] = X[ 0 ];
	A[ 0 ][ 1 ] = Y[ 0 ];
	A[ 0 ][ 2 ] = Z[ 0 ];

	A[ 1 ][ 0 ] = X[ 1 ];
	A[ 1 ][ 1 ] = Y[ 1 ];
	A[ 1 ][ 2 ] = Z[ 1 ];

	A[ 2 ][ 0 ] = X[ 2 ];
	A[ 2 ][ 1 ] = Y[ 2 ];
	A[ 2 ][ 2 ] = Z[ 2 ];

	return A;
}


template< typename T >
[[nodiscard]]
Matrix<4, 4, T> CreateMatrix4x4( const Vector<4, T>& X, const Vector<4, T>& Y, const Vector<4, T>& Z, const Vector<4, T>& W )
{
	Matrix<4, 4, T> A;

	A[ 0 ][ 0 ] = X[ 0 ];
	A[ 0 ][ 1 ] = Y[ 0 ];
	A[ 0 ][ 2 ] = Z[ 0 ];
	A[ 0 ][ 3 ] = W[ 0 ];

	A[ 1 ][ 0 ] = X[ 1 ];
	A[ 1 ][ 1 ] = Y[ 1 ];
	A[ 1 ][ 2 ] = Z[ 1 ];
	A[ 1 ][ 3 ] = W[ 1 ];

	A[ 2 ][ 0 ] = X[ 2 ];
	A[ 2 ][ 1 ] = Y[ 2 ];
	A[ 2 ][ 2 ] = Z[ 2 ];
	A[ 2 ][ 3 ] = W[ 2 ];

	A[ 3 ][ 0 ] = X[ 3 ];
	A[ 3 ][ 1 ] = Y[ 3 ];
	A[ 3 ][ 2 ] = Z[ 3 ];
	A[ 3 ][ 3 ] = W[ 3 ];

	return A;
}


template< typename T >
Matrix<4, 4, T> CreateMatrix4x4(	T m00, T m01, T m02, T m03,
									T m10, T m11, T m12, T m13,
									T m20, T m21, T m22, T m23,
									T m30, T m31, T m32, T m33 )
{
	Matrix<4, 4, T> A;

	A[ 0 ][ 0 ] = m00;
	A[ 0 ][ 1 ] = m01;
	A[ 0 ][ 2 ] = m02;
	A[ 0 ][ 3 ] = m03;

	A[ 1 ][ 0 ] = m10;
	A[ 1 ][ 1 ] = m11;
	A[ 1 ][ 2 ] = m12;
	A[ 1 ][ 3 ] = m13;

	A[ 2 ][ 0 ] = m20;
	A[ 2 ][ 1 ] = m21;
	A[ 2 ][ 2 ] = m22;
	A[ 2 ][ 3 ] = m23;

	A[ 3 ][ 0 ] = m30;
	A[ 3 ][ 1 ] = m31;
	A[ 3 ][ 2 ] = m32;
	A[ 3 ][ 3 ] = m33;

	return A;
}