/*
* MIT License
*
* Copyright( c ) 2023 Thomas Griebel
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this softwareand associated documentation files( the "Software" ), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions :
*
* The above copyright noticeand this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#include <type_traits>
#include "../math/vector.h"
#include "../primitives/geom.h"
#include "../image/color.h"
#include "../image/image.h"
#include "../asset_types/model.h"
#include "../asset_types/gpuProgram.h"
#include <serializer.h>

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


void AABB::Serialize( Serializer* serializer )
{
	Serializer* s = reinterpret_cast<Serializer*>( serializer );

	uint32_t version = Version;
	s->Next( version );
	if ( version != Version ) {
		throw std::runtime_error( "Wrong version number." );
	}
	min.Serialize( s );
	max.Serialize( s );
}

template<typename T>
void ImageBuffer<T>::Serialize( Serializer* serializer )
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


void Surface::Serialize( Serializer* s )
{
	uint32_t vertexCount = 0;
	if ( s->GetMode() == serializeMode_t::LOAD )
	{
		s->Next( vertexCount );
		vertices.resize( vertexCount );
	}
	else if ( s->GetMode() == serializeMode_t::STORE )
	{
		vertexCount = static_cast<uint32_t>( vertices.size() );
		s->Next( vertexCount );
	}

	for ( uint32_t i = 0; i < vertexCount; ++i ) {
		SerializeStruct( s, vertices[ i ] );
	}

	uint32_t indexCount = 0;
	if ( s->GetMode() == serializeMode_t::LOAD )
	{
		s->Next( indexCount );
		indices.resize( indexCount );
	}
	else if ( s->GetMode() == serializeMode_t::STORE )
	{
		indexCount = static_cast<uint32_t>( indices.size() );
		s->Next( indexCount );
	}

	for ( uint32_t i = 0; i < indexCount; ++i ) {
		s->Next( indices[ i ] );
	}
	uint64_t hash = materialHdl.Get();
	s->Next( hash );
	materialHdl = hdl_t( hash );
}


void Model::Serialize( Serializer* s )
{
	uint32_t version = Version;
	s->Next( version );
	if ( version != Version ) {
		throw std::runtime_error( "Wrong version number." );
	}
	bounds.Serialize( s );

	s->Next( surfCount );
	for ( uint32_t i = 0; i < surfCount; ++i ) {
		surfs[ i ].Serialize( s );
	}
}
#endif