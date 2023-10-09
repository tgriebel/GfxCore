/*
* MIT License
*
* Copyright( c ) 2020-2023 Thomas Griebel
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

#include <map>
#include <deque>
#include <string>
#include <sstream>
#include <unordered_map>

#include "geom.h"
#include "../image/bitmap.h"
#include "../image/image.h"
#include "../core/util.h"
#include "../scene/entity.h"
#include "../scene/assetManager.h"
#include "../scene/resourceManager.h"
#include "../asset_types/model.h"
#include "../asset_types/texture.h"


struct mdlHeader_t
{
	uint32_t	info;

	uint32_t	vertexOffset;
	uint32_t	indexOffset;
	uint32_t	surfOffset;
	uint32_t	materialOffset;
	uint32_t	imageOffset;

	uint32_t	numVertices;
	uint32_t	numIndices;
	uint32_t	numMaterials;
	uint32_t	numSurfaces;
	uint32_t	numImages;
};


void CreateRayTraceModel( ResourceManager& rm, const uint32_t modelIx, const mat4x4f& modelMatrix, const bool smoothNormals, const Color& tint, RtModel* outInstance, const matHdl_t materialId )
{
	const ModelSource* model = rm.GetModel( modelIx );

	const uint32_t surfCount = static_cast<uint32_t>( model->surfs.size() );
	const surface_t& baseSurf = model->surfs[ 0 ];

	// These are same for all surfaces
	uint32_t modelVB = baseSurf.vb;
	uint32_t modelIB = baseSurf.ib;

	uint32_t vbOffset	= baseSurf.vbOffset;
	uint32_t vbEnd		= baseSurf.vbEnd;
	uint32_t ibOffset	= baseSurf.ibOffset;
	uint32_t ibEnd		= baseSurf.ibEnd;

	for ( uint32_t surfIx = 1; surfIx < surfCount; ++surfIx )
	{
		const surface_t& surf = model->surfs[ surfIx ];
		ibOffset = std::min( ibOffset, surf.ibOffset );
		ibEnd = std::max( ibEnd, surf.ibEnd );

		assert( surf.vb == baseSurf.vb );
		assert( surf.ib == baseSurf.ib );
	}

	outInstance->transform = modelMatrix;
	const uint32_t vb = rm.AllocVB();
	outInstance->triCache.reserve( ( ibEnd - ibOffset ) / 3 );

	rm.PushVB( modelVB );
	rm.PushIB( modelIB );

	using triIndices = std::tuple<uint32_t, uint32_t, uint32_t>;
	std::map< uint32_t, std::deque<triIndices> > vertToPolyMap;

	vec3f centroid = vec3f( 0.0f, 0.0f, 0.0f );
	for ( uint32_t i = vbOffset; i < vbEnd; ++i )
	{
		vertex_t vertex = *rm.GetVertex( i );

		vertex.pos = outInstance->transform * vec4f( Trunc<4,1>( vertex.pos ), 1.0 );
		vertex.color *= tint;

		rm.PushVB( vb );
		rm.AddVertex( vertex );
		rm.PopVB();

		centroid += Trunc<4, 1>( vertex.pos );
	}

	outInstance->centroid = centroid / (float)( vbEnd - vbOffset );

	for ( uint32_t i = ibOffset; i < ibEnd; i += 3 )
	{
		uint32_t indices[ 3 ];
		for ( uint32_t t = 0; t < 3; ++t )
		{
			indices[ t ] = rm.GetIndex( i + t ) - vbOffset;
		}

		triIndices tIndices = std::make_tuple( indices[ 0 ], indices[ 1 ], indices[ 2 ] );

		vertToPolyMap[ indices[ 0 ] ].push_back( tIndices );
		vertToPolyMap[ indices[ 1 ] ].push_back( tIndices );
		vertToPolyMap[ indices[ 2 ] ].push_back( tIndices );
	}

	rm.PopVB();
	rm.PopIB();

	rm.PushVB( vb );
	rm.PushIB( modelIB );

	if ( smoothNormals )
	{
		using vertMapIter = std::map< uint32_t, std::deque<triIndices> >::iterator;

		for ( vertMapIter iter = vertToPolyMap.begin(); iter != vertToPolyMap.end(); ++iter )
		{
			vec3f interpretedNormal = vec3f( 0.0f, 0.0f, 0.0f );
			vec3f interpretedTangent = vec3f( 0.0f, 0.0f, 0.0f );
			vec3f interpretedBitangent = vec3f( 0.0f, 0.0f, 0.0f );

			vertex_t* vertex = rm.GetVertex( iter->first );

			for ( std::deque<triIndices>::iterator polyListIter = iter->second.begin(); polyListIter != iter->second.end(); ++polyListIter )
			{
				const uint32_t i0 = std::get<0>( *polyListIter );
				const uint32_t i1 = std::get<1>( *polyListIter );
				const uint32_t i2 = std::get<2>( *polyListIter );

				// These are transformed positions; this is critical for proper normals
				const vec3f pt0 = Trunc<4, 1>( rm.GetVertex( i0 )->pos );
				const vec3f pt1 = Trunc<4, 1>( rm.GetVertex( i1 )->pos );
				const vec3f pt2 = Trunc<4, 1>( rm.GetVertex( i2 )->pos );

				const vec3f tangent = ( pt1 - pt0 ).Normalize();
				const vec3f bitangent = ( pt2 - pt0 ).Normalize();
				const vec3f normal = Cross( tangent, bitangent ).Normalize();

				interpretedNormal += normal;
				interpretedTangent += tangent;
				interpretedBitangent += bitangent;
			}

			interpretedNormal.FlushDenorms();
			interpretedTangent.FlushDenorms();
			interpretedBitangent.FlushDenorms();

			vertex->normal = interpretedNormal.Normalize();
			vertex->tangent = interpretedTangent.Normalize();
			vertex->bitangent = interpretedBitangent.Normalize();
		}
	}
	else
	{
		// TODO: fix normals on this path
	}

	for ( uint32_t surfIx = 0; surfIx < surfCount; ++surfIx )
	{
		const surface_t& surf = model->surfs[ surfIx ];

		const uint32_t triMaterialId = ( materialId == InvalidHdl ) ? surf.materialId : materialId;

		for ( uint32_t i = surf.ibOffset; i < surf.ibEnd; i += 3 )
		{
			uint32_t indices[ 3 ];
			for ( uint32_t t = 0; t < 3; ++t )
			{
				indices[ t ] = rm.GetIndex( i + t ) - surf.vbOffset;
			}

			vertex_t& v0 = *rm.GetVertex( indices[ 0 ] );
			vertex_t& v1 = *rm.GetVertex( indices[ 1 ] );
			vertex_t& v2 = *rm.GetVertex( indices[ 2 ] );

			outInstance->triCache.push_back( Triangle( v0, v1, v2, CLOCKWISE, triMaterialId ) );
		}
	}

	rm.PopVB();
	rm.PopIB();

	outInstance->BuildAS();
}


void CreateRayTraceModel( AssetManager& assets, Entity* ent, RtModel* outInstance, const hdl_t overrideMaterial )
{
	vec4f centroid = vec4f( 0.0f, 0.0f, 0.0f, 0.0f );
	outInstance->transform = ent->GetMatrix();

	Model& model = assets.modelLib.Find( ent->modelHdl )->Get();
	for( uint32_t surfId = 0; surfId < model.surfCount; ++surfId )
	{
		Surface& surf = model.surfs[ surfId ];

		const uint32_t vertCount = static_cast<uint32_t>( surf.vertices.size() );
		for ( uint32_t vertIx = 0; vertIx < vertCount; ++vertIx )
		{
			const vertex_t& vertex = surf.vertices[ vertIx ];
			centroid += outInstance->transform * vec4f( vec3f( vertex.pos ), 1.0 );
		}

		outInstance->centroid = vec3f( centroid / (float)( vertCount ) );

		const hdl_t materialId = ( overrideMaterial != INVALID_HDL ) ? overrideMaterial : surf.materialHdl;

		const uint32_t indexCount = static_cast<uint32_t>( surf.indices.size() );
		for ( uint32_t ix = 0; ix < ( indexCount - 2 ); ix += 3 )
		{
			uint32_t indices[ 3 ];
			indices[ 0 ] = surf.indices[ ix + 0 ];
			indices[ 1 ] = surf.indices[ ix + 1 ];
			indices[ 2 ] = surf.indices[ ix + 2 ];

			vertex_t v0 = surf.vertices[ indices[ 0 ] ];
			vertex_t v1 = surf.vertices[ indices[ 1 ] ];
			vertex_t v2 = surf.vertices[ indices[ 2 ] ];

			v0.pos[3] = 1.0f;
			v1.pos[3] = 1.0f;
			v2.pos[3] = 1.0f;

			v0.pos = outInstance->transform * v0.pos;
			v1.pos = outInstance->transform * v1.pos;
			v2.pos = outInstance->transform * v2.pos;

			v0.bitangent = outInstance->transform * vec4f( v0.bitangent, 0.0f );
			v1.bitangent = outInstance->transform * vec4f( v1.bitangent, 0.0f );
			v2.bitangent = outInstance->transform * vec4f( v2.bitangent, 0.0f );

			v0.tangent = outInstance->transform * vec4f( v0.tangent, 0.0f );
			v1.tangent = outInstance->transform * vec4f( v1.tangent, 0.0f );
			v2.tangent = outInstance->transform * vec4f( v2.tangent, 0.0f );

			v0.normal = Cross( v0.tangent, v0.bitangent );
			v1.normal = Cross( v1.tangent, v1.bitangent );
			v2.normal = Cross( v2.tangent, v2.bitangent );

			v0.pos[ 3 ] = 0.0f;
			v1.pos[ 3 ] = 0.0f;
			v2.pos[ 3 ] = 0.0f;

			outInstance->triCache.push_back( Triangle( v0, v1, v2, CLOCKWISE, materialId ) );
		}
	}
	outInstance->BuildAS();
}


uint32_t CreatePlaneModel( ResourceManager& rm, const vec2f& size, const vec2i& cellCnt, const matHdl_t materialId )
{
	uint32_t modelIx = rm.AllocModel();
	ModelSource* model = rm.GetModel( modelIx );
	model->surfs.push_back( surface_t() );

	surface_t& surf = model->surfs.back();

	std::stringstream name;
	name << "_plane" << modelIx;

	model->name = name.str();
	surf.vb = rm.GetVB();
	surf.ib = rm.GetIB();
	surf.vbOffset = rm.GetVbOffset();
	surf.ibOffset = rm.GetIbOffset();

	vec2f gridSize = Divide( size, vec2f( (float)cellCnt[ 0 ], (float)cellCnt[ 1 ] ) );

	const uint32_t verticesPerQuad = 6;

	const uint32_t firstIndex = surf.vbOffset;
	uint32_t indicesCnt = surf.ibOffset;
	uint32_t vbIx = surf.vbOffset;

	for ( int32_t j = 0; j <= cellCnt[ 1 ]; ++j )
	{
		for ( int32_t i = 0; i <= cellCnt[ 0 ]; ++i )
		{
			vertex_t v;

			v.pos = vec4f( i * gridSize[ 0 ] - 0.5f * size[ 0 ], j * gridSize[ 1 ] - 0.5f * size[ 1 ], 0.0f, 1.0f );
			v.color = Color::White;
			v.normal = vec3f( 0.0f, 0.0f, 1.0f );

			rm.AddVertex( v );
		}
	}
	surf.vbEnd = rm.GetVbOffset();

	for ( int32_t j = 0; j < cellCnt[ 1 ]; ++j )
	{
		for ( int32_t i = 0; i < cellCnt[ 0 ]; ++i )
		{
			uint32_t vIx[ 4 ];
			vIx[ 0 ] = static_cast<uint32_t>( firstIndex + ( i + 0 ) + ( j + 0 ) * ( cellCnt[ 1 ] + 1 ) );
			vIx[ 1 ] = static_cast<uint32_t>( firstIndex + ( i + 1 ) + ( j + 0 ) * ( cellCnt[ 1 ] + 1 ) );
			vIx[ 2 ] = static_cast<uint32_t>( firstIndex + ( i + 0 ) + ( j + 1 ) * ( cellCnt[ 1 ] + 1 ) );
			vIx[ 3 ] = static_cast<uint32_t>( firstIndex + ( i + 1 ) + ( j + 1 ) * ( cellCnt[ 1 ] + 1 ) );

			// Clockwise-winding
			rm.AddIndex( vIx[ 0 ] );
			rm.AddIndex( vIx[ 1 ] );
			rm.AddIndex( vIx[ 2 ] );

			rm.AddIndex( vIx[ 2 ] );
			rm.AddIndex( vIx[ 1 ] );
			rm.AddIndex( vIx[ 3 ] );
		}
	}
	surf.ibEnd = rm.GetIbOffset();

	surf.materialId = materialId;

	return modelIx;
}