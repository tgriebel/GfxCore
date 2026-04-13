#pragma once

#include <math.h>
#include <vector>
#include "vector.h"

template<typename T>
class SparseVector
{
private:
	using bitMask_t = uint64_t;

	static const uint32_t		KernelSize		= 4;
	static constexpr uint32_t	BitsPerBitMask	= ( 8 * sizeof( bitMask_t ) );

	bitMask_t* bitMask = nullptr;
	Vector<KernelSize, T>** vectorPool;

public:
	SparseVector( const uint32_t D )
	{
		const uint32_t maxRealVectors = ( D + KernelSize - 1 ) / KernelSize;
		const uint32_t bitMaskCount = ( maxRealVectors + BitsPerBitMask - 1 ) / BitsPerBitMask;
		bitMask = new bitMask_t[ bitMaskCount ];

		vectorPool = new Vector<KernelSize, T>*[ D / 3 ];
	}

	//inline const T& operator[]( const size_t i ) const
	//{
	//	return elements[ i ];
	//}

	~SparseVector()
	{
		if( bitMask != nullptr )
		{
			delete[] bitMask;
			bitMask = nullptr;
		}
	}
};