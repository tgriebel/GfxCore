#include "mathVector.h"
#include "geom.h"
#include "color.h"
#include "serializer.h"
#include "image.h"

#define SERIALIZE_IMPLEMENTATIONS

#ifdef SERIALIZE_IMPLEMENTATIONS
template<size_t D, typename T>
bool Vector<D, T>::Serialize( void* serializer )
{
	Serializer* s = reinterpret_cast<Serializer*>( serializer );
	for ( size_t i = 0; i < D; ++i ) {
		bool ret = s->Next( Ref( data[i] ) );
		if( ret == false ) {
			return false;
		}
	}
	return true;
}

bool SerializeVertex( Serializer* s, vertex_t& v ) {
	bool ret = true;
	ret = ret && v.pos.Serialize( s );
	ret = ret && v.normal.Serialize( s );
	ret = ret && v.tangent.Serialize( s );
	ret = ret && v.bitangent.Serialize( s );
	ret = ret && v.uv.Serialize( s );
	ret = ret && v.color.Serialize( s );
	return ret;
}

bool SerializeRGBA( Serializer* s, rgbaTuple_t<float>& rgba ) {
	bool ret = true;
	ret = ret && s->Next( Ref( rgba.r ) );
	ret = ret && s->Next( Ref( rgba.g ) );
	ret = ret && s->Next( Ref( rgba.b ) );
	ret = ret && s->Next( Ref( rgba.a ) );
	return ret;
}

bool Color::Serialize( void* serializer )
{
	Serializer* s = reinterpret_cast<Serializer*>( serializer );

	uint32_t version = 0;
	bool ret = s->Next( Ref( version ) );
	if ( version != Color::Version ) {
		return false;
	}
	ret = ret && SerializeRGBA( s, u.rgba );
	return ret;
}

template<typename T>
bool Image<T>::Serialize( void* serializer )
{
	Serializer* s = reinterpret_cast<Serializer*>( serializer );
	
	uint32_t version = 0;
	bool ret = s->Next( Ref( version ) );
	if( version != Image<T>::Version ) {
		return false;
	}
	ret = ret && s->Next( Ref( width ) );
	ret = ret && s->Next( Ref( height ) );
	ret = ret && s->Next( Ref( length ) );
	ret = ret && s->NextArray( buffer, length );
	
	return ret;
}
#endif