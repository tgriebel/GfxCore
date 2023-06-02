/*
* MIT License
*
* Copyright( c ) 2021-2023 Thomas Griebel
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

#include <assert.h>
#include <cstdint>
#include <sstream>
#include <string>

template<typename T>
class Handle
{
private:
	static const T InvalidValue = ~0ull;
public:
	Handle()
	{
		this->value = InvalidValue;
	};

	Handle( const T handle )
	{
		this->value = handle;
	}

	Handle( const Handle& handle )
	{
		if ( handle.IsValid() ) {
			this->value = handle.value;
		} else {
			this->value = InvalidValue;
		}
	}

	~Handle()
	{
		value = InvalidValue;
	}

	Handle& operator=( const Handle& handle )
	{
		if ( this != &handle )
		{
			this->~Handle();
			this->value = handle.value;
		}
		return *this;
	}

	bool operator==( const Handle& rhs ) const {
		return ( value == rhs.value );
	}

	bool operator!=( const Handle& rhs ) const {
		return ( value != rhs.value );
	}

	bool operator<( const Handle& rhs )  const {
		return ( value < rhs.value );
	}

	bool operator<=( const Handle& rhs ) const {
		return ( value <= rhs.value );
	}

	bool operator>( const Handle& rhs )  const {
		return ( value > rhs.value );
	}

	bool operator>=( const Handle& rhs ) const {
		return ( value >= rhs.value );
	}

	std::string String() const
	{
		std::stringstream ss;
		ss << Get();
		return ss.str();
	}

	void Reset() {
		this->~Handle();
	}

	bool IsValid() const {
		return ( value != InvalidValue );
	}

	T Get() const {
		return IsValid() ? value : InvalidValue;
	}
private:
	T	value;
};

 using hdl8_t = Handle<uint8_t>;
 using hdl16_t = Handle<uint16_t>;
 using hdl32_t = Handle<uint32_t>;
 using hdl64_t = Handle<uint64_t>;
 using hdl_t = hdl64_t;

#define INVALID_HDL hdl_t()