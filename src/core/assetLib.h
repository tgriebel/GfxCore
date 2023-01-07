#pragma once
#include "util.h"
#include <string>
#include <unordered_map>
#include <sstream>
#include <iterator>

static std::string HashString( const hdl_t& hdl ) {
	std::stringstream ss;
	ss << hdl.Get();
	return ss.str();
}

class Asset {
	Asset() {}
};

template< class AssetType >
class AssetLib {
private:
	using assetMap_t = std::unordered_map<uint64_t, AssetType>;
	using tagMap_t = std::unordered_map<uint64_t, std::string>;
	assetMap_t	assets;
	tagMap_t	tags;
public:
	void					Clear();
	const AssetType*		GetDefault() const { return ( assets.size() > 0 ) ? &assets.begin()->second : nullptr; };
	uint32_t				Count() const { return static_cast<uint32_t>( assets.size() ); }
	hdl_t					Add( const char* name, const AssetType& asset );
	AssetType*				Find( const char* name );
	const AssetType*		Find( const char* name ) const;
	AssetType*				Find( const uint32_t id );
	const AssetType*		Find( const uint32_t id ) const;
	AssetType*				Find( const hdl_t& hdl );
	const AssetType*		Find( const hdl_t& hdl ) const;
	const char*				FindName( const hdl_t& hdl ) const;
	hdl_t					RetrieveHdl( const char* name ) const;
};

template< class AssetType >
void AssetLib< AssetType >::Clear() {
	assets.clear();
	tags.clear();
}

template< class AssetType >
hdl_t AssetLib< AssetType >::Add( const char* name, const AssetType& asset )
{
	const uint64_t hash = Hash( name );
	assetMap_t::const_iterator it = assets.find( hash );
	if ( it == assets.end() ) {
		hdl_t handle = hdl_t( hash );
		assets[ hash ] = asset;
		tags[ hash ] = std::string( name );
		return handle;
	} else {
		return hdl_t( hash );
	}
}

template< class AssetType >
AssetType* AssetLib< AssetType >::Find( const char* name )
{
	assetMap_t::iterator it = assets.find( Hash( name ) );
	return ( it != assets.end() ) ? &it->second : nullptr;
}

template< class AssetType >
const AssetType* AssetLib< AssetType >::Find( const char* name ) const
{
	assetMap_t::const_iterator it = assets.find( Hash( name ) );
	return ( it != assets.end() ) ? &it->second : nullptr;
}

template< class AssetType >
AssetType* AssetLib< AssetType >::Find( const uint32_t id )
{
	assetMap_t::iterator it = assets.begin();
	std::advance( it, id );
	return ( it != assets.end() ) ? &it->second : nullptr;
}

template< class AssetType >
const AssetType* AssetLib< AssetType >::Find( const uint32_t id ) const
{
	assetMap_t::const_iterator it = assets.begin();
	std::advance( it, id );
	return ( it != assets.end() ) ? &it->second : nullptr;
}

template< class AssetType >
AssetType* AssetLib< AssetType >::Find( const hdl_t& hdl )
{
	assetMap_t::iterator it = assets.find( hdl.Get() );
	return ( it != assets.end() ) ? &it->second : nullptr;
}

template< class AssetType >
const AssetType* AssetLib< AssetType >::Find( const hdl_t& hdl ) const
{
	assetMap_t::const_iterator it = assets.find( hdl.Get() );
	return ( it != assets.end() ) ? &it->second : nullptr;
}

template< class AssetType >
const char* AssetLib< AssetType >::FindName( const hdl_t& hdl ) const
{
	tagMap_t::const_iterator it = tags.find( hdl.Get() );
	return ( it != tags.end() ) ? it->second.c_str() : nullptr;
}

template< class AssetType >
hdl_t AssetLib< AssetType >::RetrieveHdl( const char* name ) const
{
	const uint64_t hash = Hash( name );
	assetMap_t::const_iterator it = assets.find( hash );
	return ( it != assets.end() ) ? hdl_t( hash ) : INVALID_HDL;
}