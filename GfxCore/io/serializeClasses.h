#pragma once

#include "../image/image.h"

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
	SerializeArray( s, buffer, length );
}