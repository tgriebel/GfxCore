#pragma once

#include <SysCore/common.h>
#include "vector.h"
#include "matrix.h"

enum class luStateBits_t : uint8_t
{
    LU_UNKNOWN                  = 0,
    LU_COMPUTED                 = ( 1 << 0 ),
    LU_DETERMINENT_COMPUTED     = ( 1 << 1 ),
    LU_INVERSE_COMPUTED         = ( 1 << 2 ),
    LU_FAILED                   = ( 1 << 3 ),
};
DEFINE_ENUM_OPERATORS( luStateBits_t, uint8_t )

/*
* LU Decomposition adapted from https://en.wikipedia.org/wiki/LU_decomposition
* See "LU factorization with partial pivoting (LUP)" and the C implementation
*/
template< size_t N, typename T>
class LUPSolver
{
private:
    const Matrix<N, N, T>&  m_A;
    Matrix<N, N, T>&        m_LU;
    Matrix<N, N, T>         m_I;
    Matrix<N, N, T>         m_P;
    T                       m_det;
    uint32_t                m_pivotIndices[ N + 1 ]; // Implicitly stored to avoid explicit multiplications
    uint32_t                m_rowExchanges;
    luStateBits_t           m_state;
    T                       m_tolerance;

    void    ComputeDecomposition();
    void    ComputeInverse();
    void    ComputeDeterminant();

public:

    inline bool IsValid() const
    {
        return ( m_state != luStateBits_t::LU_UNKNOWN ) && !HasFlags( m_state, luStateBits_t::LU_FAILED );
    }

    LUPSolver( const Matrix<N, N, T>& A, Matrix<N, N, T>& LU, const T tolerance ) : m_A( A ), m_LU( LU ), m_tolerance( tolerance )
    {
        m_I = Matrix<N, N, T>( 1.0 );
        m_P = Matrix<N, N, T>( 0.0 );

        m_rowExchanges = 0;
        m_state = luStateBits_t::LU_UNKNOWN;
        memset( m_pivotIndices, 0, sizeof( m_pivotIndices[ 0 ] * N ) );

        ComputeDecomposition();
        ComputeInverse();
        ComputeDeterminant();
    }

    const Matrix<N, N, T>&  LU() const;
    const Matrix<N, N, T>&  P() const;
    const Matrix<N, N, T>&  Inverse() const;
    T                       Determinant() const;
    bool                    Solve( const Vector<N, T>& b, Vector<N, T>& x ) const;
};


template< size_t N, typename T>
const Matrix<N, N, T>& LUPSolver<N, T>::LU() const
{
    return m_LU;
}


template< size_t N, typename T>
const Matrix<N, N, T>& LUPSolver<N, T>::P() const
{
    return m_P;
}


template< size_t N, typename T>
const Matrix<N, N, T>& LUPSolver<N, T>::Inverse() const
{
    return m_I;
}


template< size_t N, typename T>
T LUPSolver<N, T>::Determinant() const
{
    return m_det;
}


template< size_t N, typename T>
void LUPSolver<N, T>::ComputeDecomposition()
{
    if ( HasFlags( m_state, luStateBits_t::LU_COMPUTED ) )
    {
        return;
    }

    for ( uint32_t i = 0; i <= N; i++ ) {
        m_pivotIndices[ i ] = i;
    }

    T maxA = 0.0;
    uint32_t iMax = 0;

    uint32_t j = 0;
    uint32_t k = 0;

    for ( uint32_t i = 0; i < N; i++ )
    {
        maxA = 0.0;
        iMax = i;

        for ( k = i; k < N; k++ )
        {
            T absA = fabs( m_LU[ k ][ i ] );
            if ( absA > maxA )
            {
                maxA = absA;
                iMax = k;
            }
        }

        if ( maxA < m_tolerance )
        {
            m_state = luStateBits_t::LU_FAILED;
            return;
        }

        if ( iMax != i )
        {
            // pivoting P
            j = m_pivotIndices[ i ];
            m_pivotIndices[ i ] = m_pivotIndices[ iMax ];
            m_pivotIndices[ iMax ] = j;

            // Pivot rows, original implementation used pointer to row
            Vector<N, T> row = m_LU[ i ];
            m_LU[ i ] = m_LU[ iMax ];
            m_LU[ iMax ] = row;

            // Counting pivots starting from N (for determinant)
            ++m_pivotIndices[ N ];
        }

        for ( j = i + 1; j < N; ++j )
        {
            m_LU[ j ][ i ] /= m_LU[ i ][ i ];

            for ( k = i + 1; k < N; ++k )
            {
                m_LU[ j ][ k ] -= m_LU[ j ][ i ] * m_LU[ i ][ k ];
            }
        }
    }

    for ( uint32_t i = 0; i < N; ++i )
    {
        uint32_t index = m_pivotIndices[ i ];
        m_P[ index ][ index ] = (T)1.0;
    }

    m_state = luStateBits_t::LU_COMPUTED;
}

/* Solves for `x` in `A * x = b`
 */
template< size_t N, typename T>
bool LUPSolver<N, T>::Solve( const Vector<N, T>& b, Vector<N, T>& x ) const
{
    if( IsValid() == false )
    {
        return false;
    }

    for ( int32_t i = 0; i < N; ++i )
    {
        x[ i ] = b[ m_pivotIndices[ i ] ];

        for ( int32_t k = 0; k < i; ++k ) {
            x[ i ] -= m_LU[ i ][ k ] * x[ k ];
        }
    }

    for ( int32_t i = N - 1; i >= 0; i-- )
    {
        for ( int32_t k = i + 1; k < N; ++k ) {
            x[ i ] -= m_LU[ i ][ k ] * x[ k ];
        }
        x[ i ] /= m_LU[ i ][ i ];
    }

    return true;
}

/*
 * Calculates A^-1
 */
template< size_t N, typename T>
void LUPSolver<N, T>::ComputeInverse()
{
    if ( IsValid() == false )
    {
        return;
    }

    for ( int32_t j = 0; j < N; j++ )
    {
        for ( int32_t i = 0; i < N; i++ )
        {
            m_I[ i ][ j ] = ( m_pivotIndices[ i ] == j ) ? (T)1.0 : (T)0.0;

            for ( int32_t k = 0; k < i; k++ ) {
                m_I[ i ][ j ] -= m_LU[ i ][ k ] * m_I[ k ][ j ];
            }
        }

        for ( int32_t i = N - 1; i >= 0; i-- )
        {
            for ( int32_t k = i + 1; k < N; k++ ) {
                m_I[ i ][ j ] -= m_LU[ i ][ k ] * m_I[ k ][ j ];
            }
            m_I[ i ][ j ] /= m_LU[ i ][ i ];
        }
    }

    m_state |= luStateBits_t::LU_INVERSE_COMPUTED;
}

/*
 * Calculates determinant of A
 */
template< size_t N, typename T>
void LUPSolver<N, T>::ComputeDeterminant()
{
    if ( IsValid() == false )
    {
        return; // Valid failure case b/c it implies a degenerate  matrix
    }

    T det = m_LU[ 0 ][ 0 ];

    for ( uint32_t i = 1; i < N; i++ ) {
        det *= m_LU[ i ][ i ];
    }

    m_state |= luStateBits_t::LU_DETERMINENT_COMPUTED;
    m_det = ( m_rowExchanges - N ) % 2 == 0 ? det : -det;
    m_det = ( fabs( m_det ) < m_tolerance ) ? (T)0.0 : m_det;
}