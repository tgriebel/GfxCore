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

// Dummy types for template specialization
template<size_t M, size_t N>
class CofactorMatrixStorage {};

//------------------------------------------------------------------------//
// Matrix class and matrix functions
//------------------------------------------------------------------------//

// Generic Matrix template. Can be specialized for plain-old-data, views, and sparse matrix types
template<size_t M, size_t N, typename T, typename StorageType>
class Matrix
{
public:
	Matrix<N, M, T, StorageType> Transpose( void ) const
	{
		return ::Transpose( *this );
	}

	Vector<N, T, StorageType>& operator[]( const size_t rowIndex )
	{
		static_assert( sizeof(T) == 0, "Abstract Template" );
		throw;
	}

	const Vector<N, T, StorageType>& operator[]( const size_t rowIndex ) const
	{
		static_assert( sizeof(T) == 0, "Abstract Template" );
		throw;
	}
};


template< size_t M, size_t N, typename T>
class Matrix<M, N, T, PodStorage>
{
public:

	static const size_t RowCount = M;
	static const size_t ColumnCount = N;
	using Type = Matrix<M, N, T, PodStorage>;
	using TransposeType = Matrix<N, M, T, PodStorage>;
	using RowType = Vector<N, T, PodStorage>;

private:
	RowType rows[ M ];
	static constexpr T Epsilon = std::numeric_limits< T >::epsilon() * ( (T)2.0 );

	static_assert( sizeof( rows ) == ( M * N * sizeof( T ) ), "Must be plain-old-data." );

public:

	Matrix( T diagonalValue = static_cast<T>( 0.0 ) )
	{
		for ( size_t j = 0; j < RowCount; ++j )
		{
			for ( size_t i = 0; i < ColumnCount; ++i )
			{
				rows[ j ][ i ] = ( i == j ) ? diagonalValue : static_cast<T>( 0.0 );
			}
		}
	}

	Matrix( const T values[] )
	{
		for ( size_t j = 0; j < RowCount; ++j )
		{
			for ( size_t i = 0; i < ColumnCount; ++i )
			{
				rows[ j ][ i ] = values[ i + ( j * N ) ];
			}
		}
	}

	static constexpr Type Identity()
	{
		return Type( 1.0f );
	}

	static constexpr Type Zero()
	{
		return Type( 0.0f );
	}

	inline TransposeType Transpose( void ) const
	{
		return ::Transpose( *this );
	}

	inline RowType& operator[]( const size_t rowIndex )
	{
		return rows[ rowIndex ];
	}

	inline const RowType& operator[]( const size_t rowIndex ) const
	{
		return rows[ rowIndex ];
	}
};


template<size_t SourceM, size_t SourceN, size_t M, size_t N, typename T>
class Matrix< M, N, T, ViewStorage<SourceM, SourceN> >
{
public:

	static const size_t RowCount = N;
	static const size_t ColumnCount = M;
	using SourceType = Matrix<SourceM, SourceN, T, PodStorage>;
	using Type = Matrix<M, N, T, ViewStorage<SourceM, SourceN> >;
	using TransposeType = Matrix<N, M, T, PodStorage>;
	using RowType = Vector<N, T, PodStorage >;

private:
	size_t sourceRow;
	size_t sourceCol;
	RowType* rows[ M ]; // Allows for discontinuous memory

	Matrix();

public:

	Matrix( SourceType& A, const size_t rowOffset = 0, const size_t colOffset = 0 )
	{
		// Clamp so last element doesn't exceed max
		sourceRow = ( SourceM - M );
		sourceRow = ( sourceRow > rowOffset ) ? rowOffset : sourceRow;

		sourceCol = ( SourceN - N );
		sourceCol = ( sourceCol > colOffset ) ? colOffset : sourceCol;

		size_t viewRowIndex = 0;
		for ( size_t r = sourceRow; r < ( sourceRow + M ); ++r )
		{
			rows[ viewRowIndex ] = reinterpret_cast<RowType*>( &( A[ r ][ sourceCol ] ) );
			++viewRowIndex;
		}
	}

	inline TransposeType Transpose( void ) const
	{
		return ::Transpose( *this );
	}

	inline RowType& operator[]( const size_t rowIndex )
	{
		return *rows[ rowIndex ];
	}

	inline const RowType& operator[]( const size_t rowIndex ) const
	{
		return *rows[ rowIndex ];
	}
};


template<size_t M, size_t N, typename T>
using MatrixP = Matrix<M, N, T, PodStorage>;

template<size_t SourceM, size_t SourceN, size_t M, size_t N, typename T>
using MatrixV = Matrix< M, N, T, ViewStorage<SourceM, SourceN> >;

//template<size_t M, size_t N, typename T>
//using MatrixC = Matrix< M, N, T, CofactorMatrixStorage<M, N> >;

//template<size_t M, size_t N, typename T>
//using MatrixS = Matrix<M, N, T, SparseStorage>;

template<typename T>
using Matrix2 = MatrixP<2, 2, T>;

template<typename T>
using Matrix3 = MatrixP<3, 3, T>;

template<typename T>
using Matrix4 = MatrixP<4, 4, T>;

using mat2x2f = MatrixP<2, 2, float>;
using mat2x2d = MatrixP<2, 2, double>;
using mat3x3f = MatrixP<3, 3, float>;
using mat3x3d = MatrixP<3, 3, double>;
using mat4x4f = MatrixP<4, 4, float>;
using mat4x4d = MatrixP<4, 4, double>;


template<typename T, typename S>
T Det( const Matrix<2, 2, T, S>& A );
template< typename T>
T Det( const MatrixP<3, 3, T>& A );
template< typename T>
T Det( const MatrixP<4, 4, T>& A );


template< size_t M, size_t N, typename T, typename S>
[[nodiscard]]
Matrix<M, N, T, S> operator+( const Matrix<M, N, T, S>& A, const Matrix<M, N, T, S>& B )
{
	Matrix<M, N, T, S> C;
	for ( size_t c = 0; c < N; ++c )
	{
		for ( size_t r = 0; r < M; ++r )
		{
			C[ c ][ r ] = A[ c ][ r ] + B[ c ][ r ];
		}
	}
	return	C;
}


template< size_t M, size_t N, typename T, typename S>
[[nodiscard]]
Matrix<M, N, T, S> operator-( const Matrix<M, N, T, S>& A, const Matrix<M, N, T, S>& B )
{
	Matrix<M, N, T, S> C;
	for ( size_t c = 0; c < N; ++c )
	{
		for ( size_t r = 0; r < M; ++r )
		{
			C[ c ][ r ] = A[ c ][ r ] - B[ c ][ r ];
		}
	}
	return	C;
}


template< size_t M, size_t N, typename T, typename S>
[[nodiscard]]
Matrix<M, N, T, S> operator/( const Matrix<M, N, T, S>& A, T s )
{
	Matrix<M, N, T, S> B;
	for ( size_t c = 0; c < M; ++c )
	{
		for ( size_t r = 0; r < N; ++r )
		{
			B[ c ][ r ] = A[ c ][ r ] / s;
		}
	}
	return	B;
}


template<size_t M1, size_t N1, size_t N2, typename T, typename S>
[[nodiscard]]
Matrix<M1, N2, T, S> operator*( const Matrix<M1, N1, T, S>& A, const Matrix<N1, N2, T, S>& B )
{
	Matrix<M1, N2, T, S> C;
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


template<size_t M, size_t N, typename T, typename S>
[[nodiscard]]
Matrix<M, N, T, S> operator*( const Matrix<M, N, T, S>& A, T s )
{
	Matrix<M, N, T, S> B;
	for ( size_t r = 0; r < M; ++r ) {
		for ( size_t c = 0; c < N; ++c ) {
			B[ r ][ c ] = A[ r ][ c ] * s;
		}
	}
	return	B;
}


template<size_t M, size_t N, typename T, typename S>
[[nodiscard]]
Matrix<M, N, T, S> operator*( T s, const Matrix<M, N, T, S>& A )
{
	return A * s;
}


template<size_t M, size_t N, typename T, typename S>
[[nodiscard]]
Vector<N, T, S> operator*( const Vector<N, T, S>& u, const Matrix<M, N, T, S>& A )
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


template<size_t M, size_t N, typename T, typename S>
[[nodiscard]]
Vector<N, T, S> operator*( const Matrix<M, N, T, S>& A, const Vector<M, T, S>& u )
{
	Vector<N, T, S> v;
	for ( size_t r = 0; r < M; ++r )
	{
		for ( size_t c = 0; c < N; ++c )
		{
			v[ r ] += A[ r ][ c ] * u[ c ];
		}
	}
	return v;
}


template<size_t M, size_t N, typename T, typename S>
Matrix<N, M, T, S> Transpose( const Matrix<M, N, T, S>& A )
{
	Matrix<N, M, T, S> AT;
	for ( size_t c = 0; c < N; ++c )
	{
		for ( size_t r = 0; r < M; ++r )
		{
			AT[ c ][ r ] = A[ r ][ c ];
		}
	}
	return AT;
}


template<size_t M, size_t N, typename T, typename S>
void Flush( Matrix<M, N, T, S>& A, const T tolerance )
{
	for ( uint32_t r = 0; r < M; ++r ) {
		Flush( A[ r ], tolerance );
	}
}


template<size_t M, size_t N, typename T, typename S>
void Fill( Matrix<M, N, T, S>& A, const T& value )
{
	for ( uint32_t r = 0; r < M; ++r ) {
		Fill( A[ r ], value );
	}
}


template<size_t M, size_t N, typename T, typename S>
void FillRandom( Matrix<M, N, T, S>& A )
{
	for ( uint32_t r = 0; r < M; ++r ) {
		FillRandom( A[ r ] );
	}
}


template<size_t N, typename T, typename S>
[[nodiscard]]
T Trace( const Matrix<N, N, T, S>& A )
{
	T sum( 0 );
	for ( uint32_t i = 0; i < N; ++i )
	{
		sum += A[ i ][ i ];
	}
	return sum;
}


template<size_t M, size_t N, typename T, typename S>
[[nodiscard]]
T GrandSum( const Matrix<M, N, T, S>& A )
{
	T sum( 0 );
	for ( uint32_t r = 0; r < M; ++r )
	{
		for ( uint32_t c = 0; c < N; ++c )
		{
			sum += A[ r ][ c ];
		}
	}
	return sum;
}


template<typename T, typename S>
[[nodiscard]]
T Det( const Matrix<2, 2, T, S>& A )
{
	return A[ 0 ][ 0 ] * A[ 1 ][ 1 ] - A[ 1 ][ 0 ] * A[ 0 ][ 1 ];
}


template< typename T>
[[nodiscard]]
T Det( const MatrixP<3, 3, T>& A )
{
	T cof00[] = { A[ 1 ][ 1 ], A[ 1 ][ 2 ], A[ 2 ][ 1 ], A[ 2 ][ 2 ] };
	T cof01[] = { A[ 1 ][ 0 ], A[ 1 ][ 2 ], A[ 2 ][ 0 ], A[ 2 ][ 2 ] };
	T cof02[] = { A[ 1 ][ 0 ], A[ 1 ][ 1 ], A[ 2 ][ 0 ], A[ 2 ][ 1 ] };

	return A[ 0 ][ 0 ] * Det( MatrixP< 2, 2, T >( cof00 ) ) - A[ 0 ][ 1 ] * Det( MatrixP<2, 2, T>( cof01 ) ) + A[ 0 ][ 2 ] * Det( MatrixP<2, 2, T>( cof02 ) );
}


template< typename T>
[[nodiscard]]
T Det( const MatrixP<4, 4, T>& A )
{
	T cof00[] = { A[ 1 ][ 1 ], A[ 1 ][ 2 ], A[ 1 ][ 3 ],  A[ 2 ][ 1 ], A[ 2 ][ 2 ], A[ 2 ][ 3 ],  A[ 3 ][ 1 ], A[ 3 ][ 2 ], A[ 3 ][ 3 ] };
	T cof01[] = { A[ 1 ][ 0 ], A[ 1 ][ 2 ], A[ 1 ][ 3 ],  A[ 2 ][ 0 ], A[ 2 ][ 2 ], A[ 2 ][ 3 ],  A[ 3 ][ 0 ], A[ 3 ][ 2 ], A[ 3 ][ 3 ] };
	T cof02[] = { A[ 1 ][ 0 ], A[ 1 ][ 1 ], A[ 1 ][ 3 ],  A[ 2 ][ 0 ], A[ 2 ][ 1 ], A[ 2 ][ 3 ],  A[ 3 ][ 0 ], A[ 3 ][ 1 ], A[ 3 ][ 3 ] };
	T cof03[] = { A[ 1 ][ 0 ], A[ 1 ][ 1 ], A[ 1 ][ 2 ],  A[ 2 ][ 0 ], A[ 2 ][ 1 ], A[ 2 ][ 2 ],  A[ 3 ][ 0 ], A[ 3 ][ 1 ], A[ 3 ][ 2 ] };

	return A[ 0 ][ 0 ] * Det( MatrixP<3, 3, T>( cof00 ) ) - A[ 0 ][ 1 ] * Det( MatrixP<3, 3, T>( cof01 ) ) + A[ 0 ][ 2 ] * Det( MatrixP<3, 3, T>( cof02 ) ) - A[ 0 ][ 3 ] * Det( MatrixP<3, 3, T>( cof03 ) );
}


template< typename T>
[[nodiscard]]
MatrixP<4, 4, T> CofactorMatrix( const MatrixP<4, 4, T>& A )
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

	T values[] = {	Det( MatrixP< 3, 3, T >( minor00 ) ), -Det( MatrixP< 3, 3, T >( minor01 ) ), Det( MatrixP< 3, 3, T >( minor02 ) ), -Det( MatrixP< 3, 3, T >( minor03 ) ),
					-Det( MatrixP< 3, 3, T >( minor10 ) ), Det( MatrixP< 3, 3, T >( minor11 ) ), -Det( MatrixP< 3, 3, T >( minor12 ) ), Det( MatrixP< 3, 3, T >( minor13 ) ),
					Det( MatrixP< 3, 3, T >( minor20 ) ), -Det( MatrixP< 3, 3, T >( minor21 ) ), Det( MatrixP< 3, 3, T >( minor22 ) ), -Det( MatrixP< 3, 3, T >( minor23 ) ),
					-Det( MatrixP< 3, 3, T >( minor30 ) ), Det( MatrixP< 3, 3, T >( minor31 ) ), -Det( MatrixP< 3, 3, T >( minor32 ) ), Det( MatrixP< 3, 3, T >( minor33 ) ) };

	return MatrixP<4, 4, T>( values );
}


template< typename T, typename S>
[[nodiscard]]
Matrix<3, 3, T, S> Invert( const Matrix<3, 3, T, S>& A, bool& invertible )
{
	T det_val = Det( A );

	if ( det_val == 0 )
	{
		invertible = false;
		return Matrix<3, 3, T, S>();
	}
	invertible = true;
	assert( false ); // TODO: transpose of cofactor matrix not transpose of matrix
	return A.Transpose() * ( 1. / det_val );
}

template< typename T, typename S>
[[nodiscard]]
Matrix<4, 4, T, S> Invert( const Matrix<4, 4, T, S>& A, bool& invertible )
{
	T det_val = Det( A );

	if ( det_val == 0 )
	{
		invertible = false;
		return Matrix<4, 4, T, S>();
	}
	invertible = true;

	return ( CofactorMatrix( A ).Transpose() ) * ( static_cast<T>( 1.0 ) / det_val );
}

template<size_t M, size_t N, typename T, typename S>
T Convolution( const Matrix<M, N, T, S>& A, const Matrix<M, N, T, S>& B )
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


template<size_t M, size_t N, typename T, typename S>
[[nodiscard]]
Matrix<M, N, T, S> Identity()
{
	return Matrix<M, N, T, S>( static_cast<T>( 1.0 ) );
}


template<size_t N, typename T, typename S>
[[nodiscard]]
bool IsInvertible( const Matrix<N, N, T, S>& m )
{
	return ( Det( m ) != 0 );
}


template<size_t N, typename T, typename S>
[[nodiscard]]
bool IsOrthonormal( const Matrix<N, N, T, S>& A, const T tolerance )
{
	Matrix<N, N, T, S> I = A * A.Transpose();
	return IsIdentity( I, tolerance );
}


template<size_t N, typename T, typename S>
[[nodiscard]]
bool IsIdentity( const Matrix<N, N, T, S>& A, const T tolerance )
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


template<size_t M, size_t N, typename T, typename S>
std::ostream& operator<<( std::ostream& stream, const Matrix<M, N, T, S>& A )
{
	stream << "[";
	for ( size_t r = 0; r < M; ++r ) {
		stream << A[ r ] << ( ( r + 1 < M ) ? ",\n" : "" );
	}
	stream << "]";
	return stream;
}

template<typename T>
[[nodiscard]]
MatrixP<2, 2, T> CreateMatrix2x2(	T m00, T m01,
									T m10, T m11 )
{
	MatrixP<2, 2, T> A;

	A[ 0 ][ 0 ] = m00;
	A[ 0 ][ 1 ] = m01;

	A[ 1 ][ 0 ] = m10;
	A[ 1 ][ 1 ] = m11;

	return A;
}

template<typename T>
[[nodiscard]]
MatrixP<3, 3, T> CreateMatrix3x3(	T m00, T m01, T m02,
									T m10, T m11, T m12,
									T m20, T m21, T m22 )
{
	MatrixP<3, 3, T> A;

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


template<typename T, typename S>
[[nodiscard]]
MatrixP<3, 3, T> CreateMatrix3x3( const Vector<3, T, S>& X, const Vector<3, T, S>& Y, const Vector<3, T, S>& Z )
{
	MatrixP<3, 3, T> A;

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


template<typename T, typename S>
[[nodiscard]]
MatrixP<4, 4, T> CreateMatrix4x4( const Vector<4, T, S>& X, const Vector<4, T, S>& Y, const Vector<4, T, S>& Z, const Vector<4, T, S>& W )
{
	MatrixP<4, 4, T> A;

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


template<typename T>
MatrixP<4, 4, T> CreateMatrix4x4(	T m00, T m01, T m02, T m03,
									T m10, T m11, T m12, T m13,
									T m20, T m21, T m22, T m23,
									T m30, T m31, T m32, T m33 )
{
	MatrixP<4, 4, T> A;

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