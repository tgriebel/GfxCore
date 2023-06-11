/*
* MIT License
*
* Copyright( c ) 2022-2023 Thomas Griebel
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
#include "util.h"
#include "handle.h"
#include <list>
#include <unordered_map>
#include <iterator>
#include <sstream>

#include "asset.h"

template< class AssetType >
class AssetLib {
private:
	using assetMap_t = std::unordered_map< uint64_t, Asset<AssetType> >;
	using loadList_t = std::list<uint64_t>;
	assetMap_t assets;
	loadList_t pendingLoad;
public:
	static inline hdl_t		Handle( const char* name ) { return Hash( name ); }
	void					Clear();
	Asset<AssetType>*		GetDefault() { return ( assets.size() > 0 ) ? &assets.begin()->second : nullptr; };
	const Asset<AssetType>*	GetDefault() const { return ( assets.size() > 0 ) ? &assets.begin()->second : nullptr; };
	void					LoadAll();
	void					UnloadAll();
	bool					HasPendingLoads() const { return ( pendingLoad.size() > 0 ); }
	uint32_t				Count() const { return static_cast<uint32_t>( assets.size() ); }
	hdl_t					Add( const char* name, const AssetType& asset, const bool replaceIfFound = false );
	hdl_t					AddDeferred( const char* name, std::unique_ptr< LoadHandler<AssetType> > loader = std::unique_ptr< LoadHandler<AssetType> >() );
	void					Remove( const uint32_t id );
	void					Remove( const hdl_t& hdl );
	Asset<AssetType>*		Find( const char* name );
	const Asset<AssetType>* Find( const char* name ) const;
	Asset<AssetType>*		Find( const uint32_t id );
	const Asset<AssetType>*	Find( const uint32_t id ) const;
	Asset<AssetType>*		Find( const hdl_t& hdl );
	const Asset<AssetType>*	Find( const hdl_t& hdl ) const;
	const char*				FindName( const hdl_t& hdl ) const;
	const char*				FindName( const uint32_t id ) const;
	hdl_t					RetrieveHdl( const char* name ) const;
};

template< class AssetType >
void AssetLib< AssetType >::Clear()
{
	UnloadAll();
	assets.clear();
	pendingLoad.clear();
}

template< class AssetType >
void AssetLib< AssetType >::LoadAll()
{
	for ( auto it = pendingLoad.begin(); it != pendingLoad.end(); ) {
		hdl_t handle = *it;
		Asset<AssetType>* asset = Find( handle );
		if( asset->Load() == false )
		{
			// Assume this is a bad asset, path, or loader
			Remove( handle );
			it = pendingLoad.erase( it );
		}
		else 
		{
			if ( asset->IsLoaded() ) {
				it = pendingLoad.erase( it );
			} else {
				++it;
			}
			asset->QueueUpload();
		}
	}
}

template< class AssetType >
void AssetLib< AssetType >::UnloadAll()
{
	for ( auto it = assets.begin(); it != assets.end(); ++it )
	{
		Asset<AssetType>& asset = it->second;
		if( asset.HasLoader() )
		{
			asset.Unload();
			pendingLoad.push_back( it->first );
		}
	}
}

template< class AssetType >
hdl_t AssetLib< AssetType >::Add( const char* name, const AssetType& asset, const bool replaceIfFound )
{
	std::string assetName = name;
	if( assetName.length() == 0 ) {
		return INVALID_HDL;
	}
	
	uint64_t hash = Hash( assetName.c_str() );
	
	if( replaceIfFound == false )
	{
		uint64_t instance = 0;
		auto it = assets.find( hash );
		while( it != assets.end() )
		{
			++instance;
			std::stringstream ss;
			ss << name << "_" << instance;
			assetName = ss.str();
		
			hash = Hash( assetName.c_str() );
			it = assets.find( hash );
		}
	}

	assets[ hash ] = Asset<AssetType>( asset, assetName );
	return Handle( name );
}

template< class AssetType >
hdl_t AssetLib< AssetType >::AddDeferred( const char* name, std::unique_ptr< LoadHandler<AssetType> > loader )
{
	std::string assetName = name;
	if ( assetName.length() == 0 ) {
		return INVALID_HDL;
	}

	const uint64_t hash = Hash( name );
	auto it = assets.find( hash );
	if ( it == assets.end() ) {
		assets[ hash ] = Asset<AssetType>( AssetType(), assetName, false );
		pendingLoad.push_back( hash );
	}

	Asset<AssetType>& asset = assets[ hash ];
	if( loader ) {
		asset.AttachLoader( std::move( loader ) );
	}

	return Handle( name );
}

template< class AssetType >
void AssetLib<AssetType>::Remove( const uint32_t id )
{
	auto it = assets.begin();
	std::advance( it, id );
	assets.erase( it );
}

template< class AssetType >
void AssetLib<AssetType>::Remove( const hdl_t& hdl )
{
	assets.erase( hdl.Get() );
}

template< class AssetType >
Asset<AssetType>* AssetLib< AssetType >::Find( const char* name )
{
	auto it = assets.find( Hash( name ) );
	return ( it != assets.end() ) ? &it->second : GetDefault();
}

template< class AssetType >
const Asset<AssetType>* AssetLib< AssetType >::Find( const char* name ) const
{
	auto it = assets.find( Hash( name ) );
	return ( it != assets.end() ) ? &it->second : GetDefault();
}

template< class AssetType >
Asset<AssetType>* AssetLib< AssetType >::Find( const uint32_t id )
{
	auto it = assets.begin();
	std::advance( it, id );
	return ( it != assets.end() ) ? &it->second : GetDefault();
}

template< class AssetType >
const Asset<AssetType>* AssetLib< AssetType >::Find( const uint32_t id ) const
{
	auto it = assets.begin();
	std::advance( it, id );
	return ( it != assets.end() ) ? &it->second : GetDefault();
}

template< class AssetType >
Asset<AssetType>* AssetLib< AssetType >::Find( const hdl_t& hdl )
{
	auto it = assets.find( hdl.Get() );
	return ( it != assets.end() ) ? &it->second : GetDefault();
}

template< class AssetType >
const Asset<AssetType>* AssetLib< AssetType >::Find( const hdl_t& hdl ) const
{
	auto it = assets.find( hdl.Get() );
	return ( it != assets.end() ) ? &it->second : GetDefault();
}

template< class AssetType >
const char* AssetLib< AssetType >::FindName( const hdl_t& hdl ) const
{
	auto it = assets.find( hdl.Get() );
	return ( it != assets.end() ) ? it->second.GetName().c_str() : "<missing-asset>";
}

template< class AssetType >
const char* AssetLib< AssetType >::FindName( const uint32_t id ) const
{
	auto it = assets.begin();
	std::advance( it, id );
	return ( it != assets.end() ) ? it->second.GetName().c_str() : "<missing-asset>";
}

template< class AssetType >
hdl_t AssetLib< AssetType >::RetrieveHdl( const char* name ) const
{
	const uint64_t hash = Hash( name );
	auto it = assets.find( hash );
	return ( it != assets.end() ) ? hdl_t( hash ) : INVALID_HDL;
}