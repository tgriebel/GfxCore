/*
* MIT License
*
* Copyright( c ) 2025 Thomas Griebel
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