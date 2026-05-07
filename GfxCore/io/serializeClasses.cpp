#include <type_traits>
#include "../math/vector.h"
#include "../primitives/geom.h"
#include "../image/color.h"
#include "../image/image.h"
#include <syscore/serializer.h>

#define SERIALIZE_IMPLEMENTATIONS

#ifdef SERIALIZE_IMPLEMENTATIONS

template<size_t D, typename T, typename S>
void Serialize( Serializer* serializer, Vector<D, T, S>& v )
{
	Serializer* s = reinterpret_cast<Serializer*>( serializer );
	uint32_t length = D;
	s->Next( length );
	if ( length != D ) {
		throw std::runtime_error( "Wrong vector length." );
	}
	for ( size_t i = 0; i < D; ++i ) {
		s->Next( v[ i ] );
	}
}


void SerializeStruct( Serializer* s, vertex_t& v )
{
	static_assert( sizeof( vertex_t ) == 84, "Serialization out-of-date" );
	v.pos.Serialize( s );
	v.normal.Serialize( s );
	v.tangent.Serialize( s );
	v.bitangent.Serialize( s );
	v.uv0.Serialize( s );
	v.uv1.Serialize( s );
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


void SerializeStruct( Serializer* s, rgbTuple_t<float>& rgb )
{
	static_assert( sizeof( rgbTuple_t<float> ) == 12, "Serialization out-of-date" );
	s->Next( rgb.r );
	s->Next( rgb.g );
	s->Next( rgb.b );
}


void Color::Serialize( Serializer* s )
{
	uint32_t version = Version;
	s->Next( version );
	if ( version != Version ) {
		throw std::runtime_error( "Wrong version number." );
	}
	SerializeStruct( s, rgba );
}


void AABB::Serialize( Serializer* s )
{
	uint32_t version = Version;
	s->Next( version );
	if ( version != Version ) {
		throw std::runtime_error( "Wrong version number." );
	}
	min.Serialize( s );
	max.Serialize( s );
}


void ImageBufferInterface::Serialize( Serializer* s )
{
	uint32_t version = Version;
	s->Next( version );
	s->Next( width );
	s->Next( height );
	s->Next( layers );	
	s->Next( length );
	s->Next( bpp );
	s->Next( mipCount );

	if ( version == 5 )
	{
		s->Next( byteCount );
	}

	if ( s->GetMode() == serializeMode_t::LOAD )
	{
		imageBufferInfo_t info{};
		info.width = width;
		info.height = height;
		info.layers = layers;
		info.mipCount = mipCount > 0 ? mipCount : 1;
		info.bpp = bpp;

		const uint32_t storedLength = length; // TODO: replace with byteCount
		_Init( info );
		assert( storedLength == length );
	}

	if( version == 5 )
	{
		assert( buffer != nullptr );
		SerializeArray( s, buffer, byteCount );
	}
	else
	{
		assert( buffer != nullptr );
		SerializeArray( s, buffer, bpp * length );
	}
}
#endif
