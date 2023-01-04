#pragma once

#include <assert.h>
#include <cstdint>

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