/*
* MIT License
*
* Copyright( c ) 2013-2023 Thomas Griebel
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

#ifndef MATRIX_H
#define MATRIX_H

#include <math.h>
#include "vector.h"

//------------------------------------------------------------------------//
// Matrix Class and matrix functions
//------------------------------------------------------------------------//

template< size_t N, typename T>
class MatrixRow
{
private:

	T row[ N ];

public:

	inline T& operator[]( size_t i )
	{
		return row[ i ];
	}

	inline const T& operator[]( size_t i ) const
	{
		return row[ i ];
	}
};

template< size_t M, size_t N, typename T>
class Matrix {

private:
	MatrixRow<N, T> data[ M ];
	static constexpr T epsilon = std::numeric_limits< T >::epsilon() * ( (T) 2.0 );

public:

	static const size_t Rows = N;
	static const size_t Cols = M;

	Matrix( T diagonalValue = static_cast<T>( 0.0 ) )
	{
		for ( size_t j = 0; j < N; ++j )
		{
			for ( size_t i = 0; i < M; ++i )
			{
				data[ j ][ i ] = ( i == j ) ? diagonalValue : static_cast<T>( 0.0 );
			}
		}
	}

	Matrix( const T values[] )
	{
		for ( size_t j = 0; j < N; ++j )
		{
			for ( size_t i = 0; i < M; ++i )
			{
				data[ j ][ i ] = values[ i + ( j * N ) ];
			}
		}
	}

	Matrix<N, M, T>	Transpose( void );
	Matrix<N, M, T>	Transpose( void ) const;
	bool IsInvertible() const;
	bool IsOrthonormal( const float epsilon ) const;
	// Matrix<M, N, T>	Inverse(bool&);

	MatrixRow<N, T>& operator[]( const size_t i );
	const MatrixRow<N, T>& operator[]( const size_t i ) const;
};

template< size_t N, typename T>
class MatrixSubViewRow
{
private:

	T columns[ N ];

public:

	inline T& operator[]( size_t i )
	{
		return columns[ i ];
	}

	inline const T& operator[]( size_t i ) const
	{
		return columns[ i ];
	}
};

template< size_t M, size_t N, typename T>
class MatrixSubView
{
private:

	size_t i, j;
	Matrix<M, N, T>& refMatrix;

public:

	MatrixSubView( const Matrix< M, N, T>& m, const size_t col_mask, const size_t row_mask ) : i( col_mask ), j( row_mask ), refMatrix( m ) {}

	MatrixSubViewRow<N, T>& operator[]( size_t i );
	const MatrixSubViewRow<N, T>& operator[]( size_t i ) const;
};

template< typename T>
T Det( Matrix<2, 2, T> m );
template< typename T>
T Det( Matrix<3, 3, T> m );
template< typename T>
T Det( Matrix<4, 4, T> m );

typedef Matrix<2, 2, double>	mat2x2d;
typedef Matrix<2, 2, float>		mat2x2f;

typedef Matrix<3, 3, double>	mat3x3d;
typedef Matrix<3, 3, float>		mat3x3f;

typedef Matrix<4, 4, double>	mat4x4d;
typedef Matrix<4, 4, float>		mat4x4f;


template< size_t M, size_t N, typename T>
Matrix<N, M, T> Matrix<M, N, T>::Transpose( void )
{
	Matrix<N, M, T> mt;
	for ( size_t c = 0; c < N; ++c ) {
		for ( size_t r = 0; r < M; ++r )
		{
			mt[ c ][ r ] = data[ r ][ c ];
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
			mt[ c ][ r ] = data[ r ][ c ];
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
MatrixRow<N, T>& Matrix<M, N, T>::operator[]( const size_t i )
{
	return data[ i ];
}


template< size_t M, size_t N, typename T>
const MatrixRow<N, T>& Matrix<M, N, T>::operator[]( const size_t i ) const
{
	return data[ i ];
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


template< typename T>
T Det( Matrix<2, 2, T> m )
{
	return m[ 0 ][ 0 ] * m[ 1 ][ 1 ] - m[ 1 ][ 0 ] * m[ 0 ][ 1 ];
}


template< typename T>
T Det( Matrix<3, 3, T> m )
{
	T cof00[] = { m[ 1 ][ 1 ], m[ 1 ][ 2 ], m[ 2 ][ 1 ], m[ 2 ][ 2 ] };
	T cof01[] = { m[ 1 ][ 0 ], m[ 1 ][ 2 ], m[ 2 ][ 0 ], m[ 2 ][ 2 ] };
	T cof02[] = { m[ 1 ][ 0 ], m[ 1 ][ 1 ], m[ 2 ][ 0 ], m[ 2 ][ 1 ] };

	return m[ 0 ][ 0 ] * Det( Matrix< 2, 2, T >( cof00 ) ) - m[ 0 ][ 1 ] * Det( Matrix< 2, 2, T >( cof01 ) ) + m[ 0 ][ 2 ] * Det( Matrix< 2, 2, T >( cof02 ) );
}


template< typename T>
T Det( Matrix<4, 4, T> m )
{
	T cof00[] = { m[ 1 ][ 1 ], m[ 1 ][ 2 ], m[ 1 ][ 3 ],  m[ 2 ][ 1 ], m[ 2 ][ 2 ], m[ 2 ][ 3 ],  m[ 3 ][ 1 ], m[ 3 ][ 2 ], m[ 3 ][ 3 ] };
	T cof01[] = { m[ 1 ][ 0 ], m[ 1 ][ 2 ], m[ 1 ][ 3 ],  m[ 2 ][ 0 ], m[ 2 ][ 2 ], m[ 2 ][ 3 ],  m[ 3 ][ 0 ], m[ 3 ][ 2 ], m[ 3 ][ 3 ] };
	T cof02[] = { m[ 1 ][ 0 ], m[ 1 ][ 1 ], m[ 1 ][ 3 ],  m[ 2 ][ 0 ], m[ 2 ][ 1 ], m[ 2 ][ 3 ],  m[ 3 ][ 0 ], m[ 3 ][ 1 ], m[ 3 ][ 3 ] };
	T cof03[] = { m[ 1 ][ 0 ], m[ 1 ][ 1 ], m[ 1 ][ 2 ],  m[ 2 ][ 0 ], m[ 2 ][ 1 ], m[ 2 ][ 2 ],  m[ 3 ][ 0 ], m[ 3 ][ 1 ], m[ 3 ][ 2 ] };

	return m[ 0 ][ 0 ] * Det( Matrix< 3, 3, T >( cof00 ) ) - m[ 0 ][ 1 ] * Det( Matrix< 3, 3, T >( cof01 ) ) + m[ 0 ][ 2 ] * Det( Matrix< 3, 3, T >( cof02 ) ) - m[ 0 ][ 3 ] * Det( Matrix< 3, 3, T >( cof03 ) );
}


template< typename T>
Matrix<4, 4, T> CofactorMatrix( Matrix<4, 4, T> m )
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
Matrix<3, 3, T> Inverse( const Matrix<3, 3, T>& m, bool& invertible )
{
	T det_val = Det( m );

	if ( det_val == 0 ) { invertible = false; return Matrix<3, 3, T>(); }
	invertible = true;
	assert( false ); // TODO: transpose of cofactor matrix not transpose of matrix
	return m.Transpose() * ( 1. / det_val );
}

template< typename T>
Matrix<4, 4, T> Inverse( const Matrix<4, 4, T>& m, bool& invertible )
{
	T det_val = Det( m );

	if ( det_val == 0 ) { invertible = false; return Matrix<4, 4, T>(); }
	invertible = true;

	return ( CofactorMatrix( m ).transpose() ) * ( 1. / det_val ); // TODO: transpose of cofactor matrix not transpose of matrix
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


template< size_t M, size_t N, typename T >
std::ostream& operator<<( std::ostream& stream, const Matrix<M, N, T>& m )
{
	for ( size_t r( 0 ); r < M; ++r ) {
		stream << "[";
		for ( size_t c( 0 ); c < N; ++c ) {
			stream << " " << m[ r ][ c ];
		}
		stream << "]" << std::endl;
	}

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
#endif