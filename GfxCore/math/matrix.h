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
	bool IsInvertible() const;
	bool IsOrthonormal( const float epsilon ) const;

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

	MatrixView( const Matrix< SourceM, SourceN, T>& m, const size_t colOffset, const size_t rowOffset ) : 
		i( colOffset ),
		j( rowOffset )
	{
		for ( size_t r = rowOffset; r < M; ++r )
		{
			rows[ r ] = reinterpret_cast< Vector<N, T>* >( &m[ r ] );
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
T Det( const Matrix<2, 2, T>& m );
template< typename T>
T Det( const Matrix<3, 3, T>& m );
template< typename T>
T Det( const Matrix<4, 4, T>& m );

using mat2x2f = Matrix<2, 2, float>;
using mat2x2d = Matrix<2, 2, double>;
using mat3x3f = Matrix<3, 3, float>;
using mat3x3d = Matrix<3, 3, double>;
using mat4x4f = Matrix<4, 4, float>;
using mat4x4d = Matrix<4, 4, double>;

template< size_t M, size_t N, typename T>
Matrix<N, M, T> Matrix<M, N, T>::Transpose( void )
{
	Matrix<N, M, T> mt;
	for ( size_t c = 0; c < N; ++c ) {
		for ( size_t r = 0; r < M; ++r )
		{
			mt[ c ][ r ] = rows[ r ][ c ];
		}
	}
	return mt;
}


template< size_t M, size_t N, typename T>
Matrix<N, M, T> Matrix<M, N, T>::Transpose( void ) const
{
	Matrix<N, M, T> mt;
	for ( size_t c = 0; c < N; ++c )
	{
		for ( size_t r = 0; r < M; ++r )
		{
			mt[ c ][ r ] = rows[ r ][ c ];
		}
	}
	return mt;
}


template< size_t M, size_t N, typename T>
bool Matrix<M, N, T>::IsInvertible() const
{
	return ( Det( *this ) != 0 );
}


template< size_t M, size_t N, typename T>
bool Matrix<M, N, T>::IsOrthonormal( const float epsilon ) const
{
	Matrix<M, N, T> m = (*this) * this->Transpose();
	for ( size_t c = 0; c < N; ++c )
	{
		for ( size_t r = 0; r < M; ++r )
		{
			if ( ( c == r ) && ( fabs( m[ r ][ c ] - 1.0 ) > epsilon ) )
			{
				return false;
			}
			else if ( ( c != r ) && ( fabs( m[ r ][ c ] ) > epsilon ) )
			{
				return false;
			}
		}
	}

	return true;
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
Matrix<M, N, T> operator+( const Matrix<M, N, T>& m1, const Matrix<M, N, T>& m2 )
{
	Matrix<M, N, T> m3;
	for ( size_t c( 0 ); c < N; ++c )
	{
		for ( size_t r( 0 ); r < M; ++r )
		{
			m3[ c ][ r ] = m1[ c ][ r ] + m2[ c ][ r ];
		}
	}
	return	m3;
}


template< size_t M, size_t N, typename T>
Matrix<M, N, T> operator-( const Matrix<M, N, T>& m1, const Matrix<M, N, T>& m2 )
{
	Matrix<M, N, T> m3;
	for ( size_t c( 0 ); c < N; ++c )
	{
		for ( size_t r( 0 ); r < M; ++r )
		{
			m3[ c ][ r ] = m1[ c ][ r ] - m2[ c ][ r ];
		}
	}
	return	m3;
}


template< size_t M, size_t N, typename T>
Matrix<M, N, T> operator/( const Matrix<M, N, T>& m, T s )
{
	Matrix<M, N, T> md;
	for ( size_t c = 0; c < M; ++c )
	{
		for ( size_t r = 0; r < N; ++r )
		{
			md[ c ][ r ] = m[ c ][ r ] / s;
		}
	}
	return	md;
}


template< size_t M1, size_t N1, size_t N2, typename T>
Matrix<M1, N2, T> operator*( const Matrix<M1, N1, T>& m1, const Matrix<N1, N2, T>& m2 )
{
	Matrix<M1, N2, T> m3;
	for ( size_t r = 0; r < M1; ++r )
	{
		for ( size_t c = 0; c < N2; ++c )
		{
			for ( size_t a = 0; a < M1; ++a )
			{
				m3[ r ][ c ] += m1[ r ][ a ] * m2[ a ][ c ];
			}
		}
	}
	return m3;
}


template< size_t M, size_t N, typename T>
Matrix<M, N, T> operator*( const Matrix<M, N, T>& m, T s )
{
	Matrix<M, N, T> md;
	for ( size_t c = 0; c < N; ++c ) {
		for ( size_t r = 0; r < M; ++r ) {
			md[ c ][ r ] = m[ c ][ r ] * s;
		}
	}
	return	md;
}


template< size_t M, size_t N, typename T>
Matrix<M, N, T> operator*( T s, const Matrix<M, N, T>& m )
{
	return m * s;
}


template< size_t M, size_t N, typename T>
Vector<N, T> operator*( const Vector<N, T>& u, const Matrix<M, N, T>& m )
{
	Vector< N, T > v;
	for ( size_t c = 0; c < N; ++c )
	{
		for ( size_t r = 0; r < M; ++r )
		{
			v[ c ] += u[ r ] * m[ r ][ c ];
		}
	}
	return v;
}


template< size_t M, size_t N, typename T>
Vector<N, T> operator*( const Matrix<M, N, T>& m, const Vector<M, T>& u )
{
	Vector< N, T > v;
	for ( size_t r = 0; r < M; ++r )
	{
		for ( size_t c = 0; c < N; ++c )
		{
			v[ r ] += m[ r ][ c ] * u[ c ];
		}
	}
	return v;
}


template< size_t M, size_t N, typename T>
void Flush( Matrix<M, N, T>& m, const T tolerance )
{
	for ( uint32_t r = 0; r < M; ++r ) {
		Flush( m[ r ], tolerance );
	}
}


template< size_t M, size_t N, typename T>
void Fill( Matrix<M, N, T>& m, const T& value )
{
	for ( uint32_t r = 0; r < M; ++r ) {
		Fill( m[ r ], value );
	}
}


template< size_t M, size_t N, typename T>
void FillRandom( Matrix<M, N, T>& m )
{
	for ( uint32_t r = 0; r < M; ++r ) {
		FillRandom( m[ r ] );
	}
}


template< typename T>
T Det( const Matrix<2, 2, T>& m )
{
	return m[ 0 ][ 0 ] * m[ 1 ][ 1 ] - m[ 1 ][ 0 ] * m[ 0 ][ 1 ];
}


template< typename T>
T Det( const Matrix<3, 3, T>& m )
{
	T cof00[] = { m[ 1 ][ 1 ], m[ 1 ][ 2 ], m[ 2 ][ 1 ], m[ 2 ][ 2 ] };
	T cof01[] = { m[ 1 ][ 0 ], m[ 1 ][ 2 ], m[ 2 ][ 0 ], m[ 2 ][ 2 ] };
	T cof02[] = { m[ 1 ][ 0 ], m[ 1 ][ 1 ], m[ 2 ][ 0 ], m[ 2 ][ 1 ] };

	return m[ 0 ][ 0 ] * Det( Matrix< 2, 2, T >( cof00 ) ) - m[ 0 ][ 1 ] * Det( Matrix< 2, 2, T >( cof01 ) ) + m[ 0 ][ 2 ] * Det( Matrix< 2, 2, T >( cof02 ) );
}


template< typename T>
T Det( const Matrix<4, 4, T>& m )
{
	T cof00[] = { m[ 1 ][ 1 ], m[ 1 ][ 2 ], m[ 1 ][ 3 ],  m[ 2 ][ 1 ], m[ 2 ][ 2 ], m[ 2 ][ 3 ],  m[ 3 ][ 1 ], m[ 3 ][ 2 ], m[ 3 ][ 3 ] };
	T cof01[] = { m[ 1 ][ 0 ], m[ 1 ][ 2 ], m[ 1 ][ 3 ],  m[ 2 ][ 0 ], m[ 2 ][ 2 ], m[ 2 ][ 3 ],  m[ 3 ][ 0 ], m[ 3 ][ 2 ], m[ 3 ][ 3 ] };
	T cof02[] = { m[ 1 ][ 0 ], m[ 1 ][ 1 ], m[ 1 ][ 3 ],  m[ 2 ][ 0 ], m[ 2 ][ 1 ], m[ 2 ][ 3 ],  m[ 3 ][ 0 ], m[ 3 ][ 1 ], m[ 3 ][ 3 ] };
	T cof03[] = { m[ 1 ][ 0 ], m[ 1 ][ 1 ], m[ 1 ][ 2 ],  m[ 2 ][ 0 ], m[ 2 ][ 1 ], m[ 2 ][ 2 ],  m[ 3 ][ 0 ], m[ 3 ][ 1 ], m[ 3 ][ 2 ] };

	return m[ 0 ][ 0 ] * Det( Matrix< 3, 3, T >( cof00 ) ) - m[ 0 ][ 1 ] * Det( Matrix< 3, 3, T >( cof01 ) ) + m[ 0 ][ 2 ] * Det( Matrix< 3, 3, T >( cof02 ) ) - m[ 0 ][ 3 ] * Det( Matrix< 3, 3, T >( cof03 ) );
}


template< typename T>
Matrix<4, 4, T> CofactorMatrix( const Matrix<4, 4, T>& m )
{

	// 00 01 02 03
	// 10 11 12 13
	// 20 21 22 23
	// 30 31 32 33

	//TODO: use cramer's rule combine with determinant

	T minor00[] = { m[ 1 ][ 1 ], m[ 1 ][ 2 ], m[ 1 ][ 3 ],  m[ 2 ][ 1 ], m[ 2 ][ 2 ], m[ 2 ][ 3 ],  m[ 3 ][ 1 ], m[ 3 ][ 2 ], m[ 3 ][ 3 ] };
	T minor01[] = { m[ 1 ][ 0 ], m[ 1 ][ 2 ], m[ 1 ][ 3 ],  m[ 2 ][ 0 ], m[ 2 ][ 2 ], m[ 2 ][ 3 ],  m[ 3 ][ 0 ], m[ 3 ][ 2 ], m[ 3 ][ 3 ] };
	T minor02[] = { m[ 1 ][ 0 ], m[ 1 ][ 1 ], m[ 1 ][ 3 ],  m[ 2 ][ 0 ], m[ 2 ][ 1 ], m[ 2 ][ 3 ],  m[ 3 ][ 0 ], m[ 3 ][ 1 ], m[ 3 ][ 3 ] };
	T minor03[] = { m[ 1 ][ 0 ], m[ 1 ][ 1 ], m[ 1 ][ 2 ],  m[ 2 ][ 0 ], m[ 2 ][ 1 ], m[ 2 ][ 2 ],  m[ 3 ][ 0 ], m[ 3 ][ 1 ], m[ 3 ][ 2 ] };

	T minor10[] = { m[ 0 ][ 1 ], m[ 0 ][ 2 ], m[ 0 ][ 3 ],  m[ 2 ][ 1 ], m[ 2 ][ 2 ], m[ 2 ][ 3 ],  m[ 3 ][ 1 ], m[ 3 ][ 2 ], m[ 3 ][ 3 ] };
	T minor11[] = { m[ 0 ][ 0 ], m[ 0 ][ 2 ], m[ 0 ][ 3 ],  m[ 2 ][ 0 ], m[ 2 ][ 2 ], m[ 2 ][ 3 ],  m[ 3 ][ 0 ], m[ 3 ][ 2 ], m[ 3 ][ 3 ] };
	T minor12[] = { m[ 0 ][ 0 ], m[ 0 ][ 1 ], m[ 0 ][ 3 ],  m[ 2 ][ 0 ], m[ 2 ][ 1 ], m[ 2 ][ 3 ],  m[ 3 ][ 0 ], m[ 3 ][ 1 ], m[ 3 ][ 3 ] };
	T minor13[] = { m[ 0 ][ 0 ], m[ 0 ][ 1 ], m[ 0 ][ 2 ],  m[ 2 ][ 0 ], m[ 2 ][ 1 ], m[ 2 ][ 2 ],  m[ 3 ][ 0 ], m[ 3 ][ 1 ], m[ 3 ][ 2 ] };

	T minor20[] = { m[ 0 ][ 1 ], m[ 0 ][ 2 ], m[ 0 ][ 3 ],  m[ 1 ][ 1 ], m[ 1 ][ 2 ], m[ 1 ][ 3 ],  m[ 3 ][ 1 ], m[ 3 ][ 2 ], m[ 3 ][ 3 ] };
	T minor21[] = { m[ 0 ][ 0 ], m[ 0 ][ 2 ], m[ 0 ][ 3 ],  m[ 1 ][ 0 ], m[ 1 ][ 2 ], m[ 1 ][ 3 ],  m[ 3 ][ 0 ], m[ 3 ][ 2 ], m[ 3 ][ 3 ] };
	T minor22[] = { m[ 0 ][ 0 ], m[ 0 ][ 1 ], m[ 0 ][ 3 ],  m[ 1 ][ 0 ], m[ 1 ][ 1 ], m[ 1 ][ 3 ],  m[ 3 ][ 0 ], m[ 3 ][ 1 ], m[ 3 ][ 3 ] };
	T minor23[] = { m[ 0 ][ 0 ], m[ 0 ][ 1 ], m[ 0 ][ 2 ],  m[ 1 ][ 0 ], m[ 1 ][ 1 ], m[ 1 ][ 2 ],  m[ 3 ][ 0 ], m[ 3 ][ 1 ], m[ 3 ][ 2 ] };

	T minor30[] = { m[ 0 ][ 1 ], m[ 0 ][ 2 ], m[ 0 ][ 3 ],  m[ 1 ][ 1 ], m[ 1 ][ 2 ], m[ 1 ][ 3 ],  m[ 2 ][ 1 ], m[ 2 ][ 2 ], m[ 2 ][ 3 ] };
	T minor31[] = { m[ 0 ][ 0 ], m[ 0 ][ 2 ], m[ 0 ][ 3 ],  m[ 1 ][ 0 ], m[ 1 ][ 2 ], m[ 1 ][ 3 ],  m[ 2 ][ 0 ], m[ 2 ][ 2 ], m[ 2 ][ 3 ] };
	T minor32[] = { m[ 0 ][ 0 ], m[ 0 ][ 1 ], m[ 0 ][ 3 ],  m[ 1 ][ 0 ], m[ 1 ][ 1 ], m[ 1 ][ 3 ],  m[ 2 ][ 0 ], m[ 2 ][ 1 ], m[ 2 ][ 3 ] };
	T minor33[] = { m[ 0 ][ 0 ], m[ 0 ][ 1 ], m[ 0 ][ 2 ],  m[ 1 ][ 0 ], m[ 1 ][ 1 ], m[ 1 ][ 2 ],  m[ 2 ][ 0 ], m[ 2 ][ 1 ], m[ 2 ][ 2 ] };

	T values[] = { Det( Matrix< 3, 3, T >( minor00 ) ), -Det( Matrix< 3, 3, T >( minor01 ) ), Det( Matrix< 3, 3, T >( minor02 ) ), -Det( Matrix< 3, 3, T >( minor03 ) ),
						-Det( Matrix< 3, 3, T >( minor10 ) ), Det( Matrix< 3, 3, T >( minor11 ) ), -Det( Matrix< 3, 3, T >( minor12 ) ), Det( Matrix< 3, 3, T >( minor13 ) ),
						Det( Matrix< 3, 3, T >( minor20 ) ), -Det( Matrix< 3, 3, T >( minor21 ) ), Det( Matrix< 3, 3, T >( minor22 ) ), -Det( Matrix< 3, 3, T >( minor23 ) ),
						-Det( Matrix< 3, 3, T >( minor30 ) ), Det( Matrix< 3, 3, T >( minor31 ) ), -Det( Matrix< 3, 3, T >( minor32 ) ), Det( Matrix< 3, 3, T >( minor33 ) ) };

	return mat4x4f( values );
}

template< typename T>
Matrix<3, 3, T> m_LUI( const Matrix<3, 3, T>& m, bool& invertible )
{
	T det_val = Det( m );

	if ( det_val == 0 ) { invertible = false; return Matrix<3, 3, T>(); }
	invertible = true;
	assert( false ); // TODO: transpose of cofactor matrix not transpose of matrix
	return m.Transpose() * ( 1. / det_val );
}

template< typename T>
Matrix<4, 4, T> m_LUI( const Matrix<4, 4, T>& m, bool& invertible )
{
	T det_val = Det( m );

	if ( det_val == 0 ) { invertible = false; return Matrix<4, 4, T>(); }
	invertible = true;

	return ( CofactorMatrix( m ).Transpose() ) * ( static_cast<T>( 1.0 ) / det_val ); // TODO: transpose of cofactor matrix not transpose of matrix
}

template< size_t M, size_t N, typename T>
T Convolution( const Matrix<M, N, T>& m1, const Matrix<M, N, T>& m2 )
{
	T sum = 0;
	for ( size_t r = 0; r < M; ++r )
	{
		for ( size_t c = 0; c < N; ++c )
		{
			sum += m1[ r ][ c ] * m2[ r ][ c ];
		}
	}
	return sum;
}


template< size_t M, size_t N, typename T>
Matrix<M, N, T> Identity()
{
	return Matrix<M, N, T>( static_cast<T>( 1.0 ) );
}


template< size_t N, typename T>
bool IsIdentity( const Matrix<N, N, T>& m, const T tolerance )
{
	for ( size_t r = 0; r < N; ++r )
	{
		if( fabs( m[ r ][ r ] - 1.0f ) > tolerance ) {
			return false;
		} else {
			continue;
		}

		for ( size_t c = 0; c < N; ++c ) {
			if( fabs( m[ r ][ c ] ) > tolerance ) {
				return false;
			}
		}
	}
	return true;
}


template< size_t M, size_t N, typename T >
std::ostream& operator<<( std::ostream& stream, const Matrix<M, N, T>& m )
{
	stream << "[";
	for ( size_t r = 0; r < M; ++r ) {
		stream << m[ r ] << ( ( r + 1 < M ) ? ",\n" : "" );
	}
	stream << "]";
	return stream;
}

template< typename T >
Matrix<2, 2, T> CreateMatrix2x2(	T m00, T m01,
									T m10, T m11 )
{
	Matrix<2, 2, T> m;

	m[ 0 ][ 0 ] = m00;
	m[ 0 ][ 1 ] = m01;

	m[ 1 ][ 0 ] = m10;
	m[ 1 ][ 1 ] = m11;

	return m;
}

template< typename T >
Matrix<3, 3, T> CreateMatrix3x3(	T m00, T m01, T m02,
									T m10, T m11, T m12,
									T m20, T m21, T m22 )
{
	Matrix<3, 3, T> m;

	m[ 0 ][ 0 ] = m00;
	m[ 0 ][ 1 ] = m01;
	m[ 0 ][ 2 ] = m02;

	m[ 1 ][ 0 ] = m10;
	m[ 1 ][ 1 ] = m11;
	m[ 1 ][ 2 ] = m12;

	m[ 2 ][ 0 ] = m20;
	m[ 2 ][ 1 ] = m21;
	m[ 2 ][ 2 ] = m22;

	return m;
}


template< typename T >
Matrix<3, 3, T> CreateMatrix3x3( const Vector<3, T>& X, const Vector<3, T>& Y, const Vector<3, T>& Z )
{
	Matrix<3, 3, T> m;

	m[ 0 ][ 0 ] = X[ 0 ];
	m[ 0 ][ 1 ] = Y[ 0 ];
	m[ 0 ][ 2 ] = Z[ 0 ];

	m[ 1 ][ 0 ] = X[ 1 ];
	m[ 1 ][ 1 ] = Y[ 1 ];
	m[ 1 ][ 2 ] = Z[ 1 ];

	m[ 2 ][ 0 ] = X[ 2 ];
	m[ 2 ][ 1 ] = Y[ 2 ];
	m[ 2 ][ 2 ] = Z[ 2 ];

	return m;
}


template< typename T >
Matrix<4, 4, T> CreateMatrix4x4( const Vector<4, T>& X, const Vector<4, T>& Y, const Vector<4, T>& Z, const Vector<4, T>& W )
{
	Matrix<4, 4, T> m;

	m[ 0 ][ 0 ] = X[ 0 ];
	m[ 0 ][ 1 ] = Y[ 0 ];
	m[ 0 ][ 2 ] = Z[ 0 ];
	m[ 0 ][ 3 ] = W[ 0 ];

	m[ 1 ][ 0 ] = X[ 1 ];
	m[ 1 ][ 1 ] = Y[ 1 ];
	m[ 1 ][ 2 ] = Z[ 1 ];
	m[ 1 ][ 3 ] = W[ 1 ];

	m[ 2 ][ 0 ] = X[ 2 ];
	m[ 2 ][ 1 ] = Y[ 2 ];
	m[ 2 ][ 2 ] = Z[ 2 ];
	m[ 2 ][ 3 ] = W[ 2 ];

	m[ 3 ][ 0 ] = X[ 3 ];
	m[ 3 ][ 1 ] = Y[ 3 ];
	m[ 3 ][ 2 ] = Z[ 3 ];
	m[ 3 ][ 3 ] = W[ 3 ];

	return m;
}


template< typename T >
Matrix<4, 4, T> CreateMatrix4x4(	T m00, T m01, T m02, T m03,
									T m10, T m11, T m12, T m13,
									T m20, T m21, T m22, T m23,
									T m30, T m31, T m32, T m33 )
{
	Matrix<4, 4, T> m;

	m[ 0 ][ 0 ] = m00;
	m[ 0 ][ 1 ] = m01;
	m[ 0 ][ 2 ] = m02;
	m[ 0 ][ 3 ] = m03;

	m[ 1 ][ 0 ] = m10;
	m[ 1 ][ 1 ] = m11;
	m[ 1 ][ 2 ] = m12;
	m[ 1 ][ 3 ] = m13;

	m[ 2 ][ 0 ] = m20;
	m[ 2 ][ 1 ] = m21;
	m[ 2 ][ 2 ] = m22;
	m[ 2 ][ 3 ] = m23;

	m[ 3 ][ 0 ] = m30;
	m[ 3 ][ 1 ] = m31;
	m[ 3 ][ 2 ] = m32;
	m[ 3 ][ 3 ] = m33;

	return m;
}