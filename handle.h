#pragma once

#include <assert.h>
#include <cstdint>

class hdl_t
{
private:
	static const uint64_t InvalidValue = ~0ull;
public:
	hdl_t()
	{
		this->value = InvalidValue;
	};

	hdl_t( const uint64_t handle )
	{
		this->value = handle;
	}

	hdl_t( const hdl_t& handle )
	{
		if ( handle.IsValid() ) {
			this->value = handle.value;
		} else {
			this->value = InvalidValue;
		}
	}

	~hdl_t()
	{
		value = InvalidValue;
	}

	hdl_t& operator=( const hdl_t& handle )
	{
		if ( this != &handle )
		{
			this->~hdl_t();
			this->value = handle.value;
		}
		return *this;
	}

	bool operator==( const hdl_t& rhs ) const {
		return ( value == rhs.value );
	}

	bool operator!=( const hdl_t& rhs ) const {
		return ( value != rhs.value );
	}

	bool operator<( const hdl_t& rhs )  const {
		return ( value < rhs.value );
	}

	bool operator<=( const hdl_t& rhs ) const {
		return ( value <= rhs.value );
	}

	bool operator>( const hdl_t& rhs )  const {
		return ( value > rhs.value );
	}

	bool operator>=( const hdl_t& rhs ) const {
		return ( value >= rhs.value );
	}

	void Reset() {
		this->~hdl_t();
	}

	bool IsValid() const {
		return ( value != InvalidValue );
	}

	uint64_t Get() const {
		return IsValid() ? value : InvalidValue;
	}
private:
	uint64_t	value;
};

#define INVALID_HDL hdl_t()