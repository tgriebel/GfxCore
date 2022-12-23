#include <type_traits>
#include "mathVector.h"
#include "geom.h"
#include "color.h"
#include "serializer.h"
#include "image.h"

#define SERIALIZE_IMPLEMENTATIONS

#ifdef SERIALIZE_IMPLEMENTATIONS
template<size_t D, typename T>
void Vector<D, T>::Serialize( Serializer* serializer )
{
	Serializer* s = reinterpret_cast<Serializer*>( serializer );
	uint32_t length = D;
	s->Next( length );
	if ( length != D ) {
		throw std::runtime_error( "Wrong vector length." );
	}
	for ( size_t i = 0; i < D; ++i ) {
		s->Next( data[i] );
	}
}


void SerializeStruct( Serializer* s, vertex_t& v )
{
	static_assert( sizeof( vertex_t ) == 88, "Serialization out-of-date" );
	v.pos.Serialize( s );
	v.normal.Serialize( s );
	v.tangent.Serialize( s );
	v.bitangent.Serialize( s );
	v.uv.Serialize( s );
	v.color.Serialize( s );
}


void SerializeStruct( Serializer* s, rgbaTuple_t<float>& rgba )
{
	static_assert( sizeof( rgbaTuple_t<float> ) == 16, "Serialization out-of-date" );
	s->Next( rgba.r );
	s->Next( rgba.g );
	s->Next( rgba.b );
	s->Next( rgba.a );
}


void Color::Serialize( Serializer* serializer )
{
	Serializer* s = reinterpret_cast<Serializer*>( serializer );

	uint32_t version = Version;
	s->Next( version );
	if ( version != Version ) {
		throw std::runtime_error( "Wrong version number." );
	}
	SerializeStruct( s, u.rgba );
}

template<typename T>
void Image<T>::Serialize( Serializer* serializer )
{
	Serializer* s = reinterpret_cast<Serializer*>( serializer );
	
	uint32_t version = Version;
	s->Next( version );
	if( version != Version ) {
		throw std::runtime_error( "Wrong version number." );
	}
	s->Next( width );
	s->Next( height );
	s->Next( length );
	s->NextArray( buffer, length );
}
#endif