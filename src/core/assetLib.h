#pragma once
#include "util.h"
#include "handle.h"
#include <unordered_map>
#include <iterator>

#include "asset.h"

template< class AssetType >
class AssetLib {
private:
	using assetMap_t = std::unordered_map< uint64_t, Asset<AssetType> >;
	assetMap_t assets;
public:
	void					Clear();
	const AssetType*		GetDefault() const { return ( assets.size() > 0 ) ? &assets.begin()->second.Get() : nullptr; };
	uint32_t				Count() const { return static_cast<uint32_t>( assets.size() ); }
	hdl_t					Add( const char* name, const AssetType& asset );
	hdl_t					AddDeferred( const char* name );
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
void AssetLib< AssetType >::Clear() {
	assets.clear();
}

template< class AssetType >
hdl_t AssetLib< AssetType >::Add( const char* name, const AssetType& asset )
{
	const uint64_t hash = Hash( name );
	auto it = assets.find( hash );
	if ( it == assets.end() )
	{
		hdl_t handle = hdl_t( hash );
		assets[ hash ] = Asset<AssetType>( asset, name );
		return handle;
	} 
	else {
		return hdl_t( hash );
	}
}

template< class AssetType >
hdl_t AssetLib< AssetType >::AddDeferred( const char* name )
{
	const uint64_t hash = Hash( name );
	auto it = assets.find( hash );
	if ( it == assets.end() )
	{
		hdl_t handle = hdl_t( hash );
		assets[ hash ] = Asset<AssetType>( AssetType(), name, false );
		return handle;
	}
	else {
		return hdl_t( hash );
	}
}

template< class AssetType >
Asset<AssetType>* AssetLib< AssetType >::Find( const char* name )
{
	auto it = assets.find( Hash( name ) );
	return ( it != assets.end() ) ? &it->second : nullptr;
}

template< class AssetType >
const Asset<AssetType>* AssetLib< AssetType >::Find( const char* name ) const
{
	auto it = assets.find( Hash( name ) );
	return ( it != assets.end() ) ? &it->second : nullptr;
}

template< class AssetType >
Asset<AssetType>* AssetLib< AssetType >::Find( const uint32_t id )
{
	auto it = assets.begin();
	std::advance( it, id );
	return ( it != assets.end() ) ? &it->second : nullptr;
}

template< class AssetType >
const Asset<AssetType>* AssetLib< AssetType >::Find( const uint32_t id ) const
{
	auto it = assets.begin();
	std::advance( it, id );
	return ( it != assets.end() ) ? &it->second : nullptr;
}

template< class AssetType >
Asset<AssetType>* AssetLib< AssetType >::Find( const hdl_t& hdl )
{
	auto it = assets.find( hdl.Get() );
	return ( it != assets.end() ) ? &it->second : nullptr;
}

template< class AssetType >
const Asset<AssetType>* AssetLib< AssetType >::Find( const hdl_t& hdl ) const
{
	auto it = assets.find( hdl.Get() );
	return ( it != assets.end() ) ? &it->second : nullptr;
}

template< class AssetType >
const char* AssetLib< AssetType >::FindName( const hdl_t& hdl ) const
{
	auto it = assets.find( hdl.Get() );
	return ( it != assets.end() ) ? it->second.GetName().c_str() : nullptr;
}

template< class AssetType >
const char* AssetLib< AssetType >::FindName( const uint32_t id ) const
{
	auto it = assets.begin();
	std::advance( it, id );
	return ( it != assets.end() ) ? it->second.GetName().c_str() : nullptr;
}

template< class AssetType >
hdl_t AssetLib< AssetType >::RetrieveHdl( const char* name ) const
{
	const uint64_t hash = Hash( name );
	auto it = assets.find( hash );
	return ( it != assets.end() ) ? hdl_t( hash ) : INVALID_HDL;
}