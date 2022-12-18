#pragma once

#include <assert.h>

class refCount_t
{
public:
	refCount_t() = delete;

	refCount_t( const int count ) {
		assert( count > 0 );
		this->count = count;
	}
	inline int Add() {
		return ( count > 0 ) ? ++count : 0;
	}
	inline int Release() {
		return ( count > 0 ) ? --count : 0;
	}
	[[nodiscard]]
	inline int IsFree() const {
		return ( count <= 0 );
	}
private:
	int count; // Considered dead at 0
};


class hdl_t
{
public:
	hdl_t()
	{
		this->value = nullptr;
		this->instances = nullptr;
	};

	hdl_t( const int handle )
	{
		this->value = new int( handle );
		this->instances = new refCount_t( 1 );
	}

	hdl_t( const hdl_t& handle )
	{
		if ( handle.IsValid() )
		{
			this->value = handle.value;
			this->instances = handle.instances;
			this->instances->Add();
		}
		else {
			this->value = nullptr;
			this->instances = nullptr;
		}
	}

	~hdl_t()
	{
		if ( IsValid() )
		{
			instances->Release();
			if ( instances->IsFree() )
			{
				delete instances;
				delete value;
			}
			instances = nullptr;
			value = nullptr;
		}
	}

	hdl_t& operator=( const hdl_t& handle )
	{
		if ( this != &handle )
		{
			this->~hdl_t();
			this->value = handle.value;
			this->instances = handle.instances;
			if ( handle.IsValid() ) {
				this->instances->Add();
			}
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
		return ( value != nullptr ) && ( instances != nullptr );
	}

	int Get() const {
		return ( IsValid() && ( instances->IsFree() == false ) ) ? *value : -1;
	}

	void Reassign( const int handle ) {
		if ( IsValid() ) {
			*value = handle;
		}
	}
private:
	int* value;
	refCount_t* instances;
};

#define INVALID_HDL hdl_t()