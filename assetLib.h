#pragma once
#include "common.h"
#include <map>

template< class Asset >
class AssetLib {
private:
	std::vector< Asset >		assets;
	std::vector< std::string >	tags;
	std::vector< hdl_t >		handles;
public:
	void					Create();
	void					Destroy();
	const Asset*			GetDefault() const { return ( assets.size() > 0 ) ? &assets[ 0 ] : nullptr; };
	uint32_t				Count() const { return static_cast<uint32_t>( assets.size() ); }
	int						Add( const char* name, const Asset& asset );
	Asset*					Find( const char* name );
	const Asset*			Find( const char* name ) const;
	hdl_t					RetrieveHdl( const char* name ) const;
	inline Asset*			Find( const int id ) { return ( id < assets.size() && id >= 0 ) ? &assets[ id ] : nullptr; }
	inline const Asset*		Find( const int id ) const { return ( id < assets.size() && id >= 0 ) ? &assets[ id ] : nullptr; }
	int						FindId( const char* name ) const;
	const char*				FindName( const int id ) const { return ( id < tags.size() && id >= 0 ) ? tags[ id ].c_str() : ""; }
};

template< class Asset >
void AssetLib< Asset >::Destroy() {
	assets.clear();
	tags.clear();
	handles.clear();
}

template< class Asset >
int AssetLib< Asset >::Add( const char* name, const Asset& asset )
{
	const int searchId = FindId( name );
	if ( searchId < 0 ) {
		const int id = static_cast<int>( assets.size() );
		handles.push_back( hdl_t( id ) );
		assets.push_back( asset );
		tags.push_back( name );
		return id;
	} else {
		return searchId;
	}
}

template< class Asset >
int AssetLib< Asset >::FindId( const char* name ) const
{
	auto it = find( tags.begin(), tags.end(), name );
	const int idx = static_cast<int>( std::distance( tags.begin(), it ) );
	return ( it != tags.end() ) ? idx : -1;
}

template< class Asset >
Asset* AssetLib< Asset >::Find( const char* name )
{
	const int id = FindId( name );
	return ( id >= 0 ) ? &assets[ id ] : nullptr;
}

template< class Asset >
const Asset* AssetLib< Asset >::Find( const char* name ) const
{
	const int id = FindId( name );
	return ( id >= 0 ) ? &assets[ id ] : nullptr;
}

template< class Asset >
hdl_t AssetLib< Asset >::RetrieveHdl( const char* name ) const
{
	const int id = FindId( name );
	return ( id >= 0 ) ? handles[ id ] : INVALID_HDL;
}