#include "assetLib.h"
/*
template< class AssetType >
void AssetLib< AssetType >::Clear()
{
	assets.clear();
	tags.clear();
}

template< class AssetType >
hdl_t AssetLib< AssetType >::Add( const char* name, const AssetType& asset )
{
	const uint64_t hash = Hash( name );
	auto it = assets.find( hash );
	if ( it == assets.end() ) {
		hdl_t handle = hdl_t( hash );
		assets[ hash ] = asset;
		tags[ hash ] = std::string( name );
		return handle;
	}
	else {
		return hdl_t( hash );
	}
}

template< class AssetType >
AssetType* AssetLib< AssetType >::Find( const char* name )
{
	auto it = assets.find( Hash( name ) );
	return ( it != assets.end() ) ? &it->second : nullptr;
}

template< class AssetType >
const AssetType* AssetLib< AssetType >::Find( const char* name ) const
{
	auto it = assets.find( Hash( name ) );
	return ( it != assets.end() ) ? &it->second : nullptr;
}

template< class AssetType >
AssetType* AssetLib< AssetType >::Find( const uint32_t id )
{
	auto it = assets.begin();
	std::advance( it, id );
	return ( it != assets.end() ) ? &it->second : nullptr;
}

template< class AssetType >
const AssetType* AssetLib< AssetType >::Find( const uint32_t id ) const
{
	auto it = assets.begin();
	std::advance( it, id );
	return ( it != assets.end() ) ? &it->second : nullptr;
}

template< class AssetType >
AssetType* AssetLib< AssetType >::Find( const hdl_t& hdl )
{
	auto it = assets.find( hdl.Get() );
	return ( it != assets.end() ) ? &it->second : nullptr;
}

template< class AssetType >
const AssetType* AssetLib< AssetType >::Find( const hdl_t& hdl ) const
{
	auto it = assets.find( hdl.Get() );
	return ( it != assets.end() ) ? &it->second : nullptr;
}

template< class AssetType >
const char* AssetLib< AssetType >::FindName( const hdl_t& hdl ) const
{
	auto it = tags.find( hdl.Get() );
	return ( it != tags.end() ) ? it->second.c_str() : nullptr;
}

template< class AssetType >
hdl_t AssetLib< AssetType >::RetrieveHdl( const char* name ) const
{
	const uint64_t hash = Hash( name );
	auto it = assets.find( hash );
	return ( it != assets.end() ) ? hdl_t( hash ) : INVALID_HDL;
}
*/