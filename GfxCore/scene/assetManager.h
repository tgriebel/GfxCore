/*
* MIT License
*
* Copyright( c ) 2023 Thomas Griebel
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

#include "../core/assetLib.h"
#include "../core/asset.h"
#include <cassert>

class AssetManager
{
private:
	std::vector<Library*>	libraries;

	static inline uint32_t NextTypeId()
	{
		static uint32_t counter = 0;
		return counter++;
	}

	template< class T >
	static uint32_t TypeId()
	{
		static uint32_t id = NextTypeId();
		return id;
	}

public:
	~AssetManager()
	{
		for ( Library* lib : libraries )
		{
			delete lib;
		}
		libraries.clear();
	}

	template< class T >
	AssetLib<T>& RegisterLib( const char* name = "" )
	{
		const uint32_t id = TypeId<T>();
		if ( id >= libraries.size() ) {
			libraries.resize( id + 1, nullptr );
		}
		assert( libraries[ id ] == nullptr );
		AssetLib<T>* lib = new AssetLib<T>( name );
		libraries[ id ] = lib;
		return *lib;
	}

	template< class T >
	AssetLib<T>* GetLib()
	{
		const uint32_t id = TypeId<T>();
		assert( id < libraries.size() && libraries[ id ] != nullptr );
		return static_cast< AssetLib<T>* >( libraries[ id ] );
	}

	template< class T >
	const AssetLib<T>* GetLib() const
	{
		const uint32_t id = TypeId<T>();
		assert( id < libraries.size() && libraries[ id ] != nullptr );
		return static_cast< const AssetLib<T>* >( libraries[ id ] );
	}

	Library* FindLibrary( const uint32_t index )
	{
		return ( index < libraries.size() ) ? libraries[ index ] : nullptr;
	}

	uint32_t LibraryCount() const
	{
		return static_cast<uint32_t>( libraries.size() );
	}

	void Clear()
	{
		for ( Library* lib : libraries )
		{
			if ( lib != nullptr ) {
				lib->Clear();
			}
		}
	}

	inline bool HasPendingLoads()
	{
		for ( Library* lib : libraries )
		{
			if ( lib != nullptr && lib->HasPendingLoads() ) {
				return true;
			}
		}
		return false;
	}

	void RunLoadLoop( const uint32_t limit = 12 )
	{
		uint32_t i = 0;
		while ( i < limit )
		{
			bool hasLoads = false;
			for ( Library* lib : libraries )
			{
				if ( lib != nullptr && lib->HasPendingLoads() )
				{
					lib->LoadAll();
					hasLoads = true;
				}
			}
			if ( hasLoads == false ) {
				break;
			}
			++i;
		}
	}
};