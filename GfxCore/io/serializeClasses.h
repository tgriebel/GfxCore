#pragma once

#include "../image/image.h"
#include "../core/assetLib.h"
#include <common.h>

static const bool g_supportBaked = true;

struct bakedAssetInfo_t
{
	std::string		name;
	std::string		hash;
	std::string		type;
	std::string		date;
	uint32_t		sizeBytes;
};

template<class T>
bool LoadBaked( Asset<T>& asset, bakedAssetInfo_t& info, const std::string& dir, const std::string& ext )
{
	if( g_supportBaked == false ) {
		return false;
	}

	const hdl_t handle = asset.Handle();
	const std::string hash = handle.String();
	const std::string bakedPath = ".\\baked\\" + dir + hash + "." + ext;
	if ( FileExists( bakedPath ) )
	{
		Serializer* s = new Serializer( MB( 32 ), serializeMode_t::LOAD );
		s->ReadFile( bakedPath );

		s->NextString( info.name );
		s->NextString( info.type );
		s->NextString( info.date );
		info.sizeBytes = s->CurrentSize();
		info.hash = Library::Handle( info.name.c_str() ).String();

		asset.Get().Serialize( s );
		asset.SetName( info.name.c_str() );

		const bool loaded = ( s->Status() == serializeStatus_t::OK );
		assert( loaded );
		delete s;
		return loaded;
	}
	return false;
}

template<typename T>
void ImageBuffer<T>::Serialize( Serializer* s )
{
	uint32_t version = Version;
	s->Next( version );
	if ( version != Version ) {
		throw std::runtime_error( "Wrong version number." );
	}
	s->Next( width );
	s->Next( height );
	s->Next( layers );
	s->Next( length );

	if( s->GetMode() == serializeMode_t::LOAD ) {
		_Init( width, height, layers );
	}
	
	assert( buffer != nullptr );
	SerializeArray( s, buffer, length );
}