#pragma once

#include <assert.h>
#include <vector>
#include <stack>
#include "geom.h"
#include "image.h"

struct vertexBuffer_t
{
	std::vector<vertex_t> buffer;
};


struct indexBuffer_t
{
	std::vector<uint32_t> buffer;
};

using matHdl_t = int32_t;
using imgHdl_t = int32_t;
using vertHdl_t = int32_t;
using idxHdl_t = int32_t;
using mdlHdl_t = int32_t;

static const int32_t InvalidHdl = -1;

class ResourceManager
{
private:
	std::vector<vertexBuffer_t>	vertexBuffers;
	std::vector<indexBuffer_t>	indexBuffers;
	std::vector<Model>			modelBuffer;
	std::vector<material_t>		materialBuffer;
	std::vector< Image<Color> >	imageBuffer;
	std::stack<int32_t>			currentVB;
	std::stack<int32_t>			currentIB;
public:

	ResourceManager()
	{
		vertexBuffers.reserve( 10 );
		indexBuffers.reserve( 10 );
		modelBuffer.reserve( 10 );
		imageBuffer.reserve( 10 );

		currentVB.push( -1 );
		currentIB.push( -1 );
	}

	static const uint32_t InvalidModelIx = ~0x00;

	uint32_t AllocVB( const uint32_t reservation = 1000 )
	{
		vertexBuffers.resize( vertexBuffers.size() + 1 );
		vertexBuffers.back().buffer.reserve( reservation );
		return static_cast<uint32_t>( vertexBuffers.size() - 1 );
	}

	uint32_t AllocIB( const uint32_t reservation = 1000 )
	{
		indexBuffers.resize( indexBuffers.size() + 1 );
		indexBuffers.back().buffer.reserve( reservation );
		return static_cast<uint32_t>( indexBuffers.size() - 1 );
	}

	uint32_t AllocModel()
	{
		modelBuffer.push_back( Model() );
		return static_cast<uint32_t>( modelBuffer.size() - 1 );
	}

	uint32_t StoreImageCopy( const Image<Color>& image )
	{
		imageBuffer.push_back( image );
		return static_cast<uint32_t>( imageBuffer.size() - 1 );
	}

	uint32_t StoreMaterialCopy( const material_t& material )
	{
		materialBuffer.push_back( material );
		return static_cast<uint32_t>( materialBuffer.size() - 1 );
	}

	bool PushVB( const uint32_t vbIx )
	{
		if ( vbIx >= vertexBuffers.size() )
		{
			assert( false );
			return false;
		}

		currentVB.push( vbIx );
		return true;
	}

	void PopVB()
	{
		if ( currentVB.size() > 1 )
		{
			currentVB.pop();
		}
	}

	bool PushIB( const uint32_t ibIx )
	{
		if ( ibIx >= indexBuffers.size() )
		{
			assert( false );
			return false;
		}

		currentIB.push( ibIx );
		return true;
	}

	void PopIB()
	{
		if( currentIB.size() > 1 )
		{
			currentIB.pop();
		}
	}

	uint32_t GetVB() const
	{
		const uint32_t vb = currentVB.top();
		assert( vb >= 0 );
		return vb;
	}

	uint32_t GetIB() const
	{
		const uint32_t ib = currentIB.top();
		assert( ib >= 0 );
		return ib;
	}

	void AddVertex( const vertex_t& vertex )
	{
		vertexBuffers[ GetVB() ].buffer.push_back( vertex );
	}

	void AddIndex( const uint32_t index )
	{
		indexBuffers[ GetIB() ].buffer.push_back( index );
	}

	uint32_t GetVbOffset() const
	{
		return vertexBuffers[ GetVB() ].buffer.size();
	}

	uint32_t GetIbOffset() const
	{
		return indexBuffers[ GetIB() ].buffer.size();
	}

	vertex_t* GetVertex( const uint32_t i )
	{
		vertexBuffer_t& vb = vertexBuffers[ GetVB() ];
		if ( i >= vb.buffer.size() )
		{
			assert( false );
			return nullptr;
		}

		return &vb.buffer[ i ];
	}

	uint32_t* GetIndexPtr( const uint32_t i )
	{
		indexBuffer_t& ib = indexBuffers[ GetIB() ];
		if ( i >= ib.buffer.size() )
		{
			assert( false );
			return nullptr;
		}

		return &ib.buffer[ i ];
	}

	uint32_t GetIndex( const uint32_t i )
	{
		return *GetIndexPtr( i );
	}

	Model* GetModel( const uint32_t modelIx )
	{
		if ( modelIx >= modelBuffer.size() )
		{
			assert( false );
			return nullptr;
		}

		return &modelBuffer[ modelIx ];
	}

	uint32_t GetModelCount()
	{
		return modelBuffer.size();
	}

	const Image<Color>* GetImageRef( const int32_t imageIx ) const
	{
		if ( imageIx >= imageBuffer.size() )
		{
			assert( false );
			return nullptr;
		}

		return &imageBuffer[ imageIx ];
	}

	uint32_t GetImageCount()
	{
		return imageBuffer.size();
	}

	const material_t* GetMaterialRef( const int32_t index ) const
	{
		if ( index >= materialBuffer.size() )
		{
			assert( false );
			assert( materialBuffer.size() > 0 );
			return &materialBuffer[ 0 ];
		}

		return &materialBuffer[ index ];
	}

	uint32_t GetMaterialCount()
	{
		return materialBuffer.size();
	}
};