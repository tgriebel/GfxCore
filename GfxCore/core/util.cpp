#include "util.h"

// Fowler–Noll–Vo Hash - fnv1a - 32bits
// https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
[[nodiscard]]
uint32_t Hash( const uint8_t* bytes, const uint32_t sizeBytes )
{
	uint32_t result = 2166136261;
	const uint32_t prime = 16777619;
	for ( uint32_t i = 0; i < sizeBytes; ++i ) {
		result = ( result ^ bytes[ i ] ) * prime;
	}
	return result;
}


uint64_t Hash( const char* s, const int length )
{
	const int p = 31;
	const int m = static_cast<int>( 1e9 + 9 );
	uint64_t hash = 0;
	uint64_t pN = 1;
	for ( int i = 0; i < length; ++i )
	{
		hash = ( hash + ( s[ i ] - (uint64_t)'a' + 1ull ) * pN ) % m;
		pN = ( pN * p ) % m;
	}
	return hash;
}


mat4x4f RandomSolveableMatrix()
{
	while ( true )
	{
		mat4x4f M;

		for ( uint32_t r = 0; r < 4; ++r ) {
			for ( uint32_t c = 0; c < 4; ++c ) {
				M[ r ][ c ] = Random();
			}
		}

		float sum = 0.0f;
		for ( uint32_t c = 0; c < 4; ++c ) {
			sum += fabs( M[ 0 ][ c ] );
		}

		for ( uint32_t c = 0; c < 4; ++c ) {
			M[ c ][ c ] += sum;
		}

		bool invertible;
		mat4x4f IA = Invert( M, invertible );

		mat4x4f I = M * IA;
		Flush( I, 0.000001f );

		if ( invertible && IsIdentity( I, 0.000001f ) ) {
			return M;
		}
	}
	return mat4x4f::Identity();
}
