#pragma once

#include "../image/image.h"
#include "../core/assetLib.h"
#include <common.h>

template<class T>
bool LoadBaked( T& asset, const std::string& dir, const std::string& fileName, const std::string& ext )
{
	const hdl_t handle = Library::Handle( fileName.c_str() );
	const std::string hash = handle.String();
	const std::string bakedPath = ".\\baked\\" + dir + hash + "." + ext;
	if ( FileExists( bakedPath ) )
	{
		Serializer* s = new Serializer( MB( 32 ), serializeMode_t::LOAD );
		s->ReadFile( bakedPath );
		asset.Serialize( s );
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
	s->Next( length );

	if( s->GetMode() == serializeMode_t::LOAD ) {
		_Init( width, height );
	}
	
	assert( buffer != nullptr );
	SerializeArray( s, buffer, length );
}