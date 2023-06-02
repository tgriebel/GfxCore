/*
* MIT License
*
* Copyright( c ) 2021-2023 Thomas Griebel
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

#include "io.h"

#include <map>
#include <deque>
#include <string>
#include <sstream>
#include <unordered_map>

#include <serializer.h>
#include <common.h>

#include "../scene/scene.h"
#include "../scene/assetManager.h"
#include "../resource_types/model.h"
#include "../resource_types/texture.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "../../external/tiny_obj_loader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../../external/stb_image.h"


bool LoadImage( const char* texturePath, Image& texture )
{
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load( texturePath, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha );

	if ( !pixels ) {
		stbi_image_free( pixels );
		return false;
	}

	//assert( texChannels == 4 );
	texture.info.width = static_cast<uint32_t>( texWidth );
	texture.info.height = static_cast<uint32_t>( texHeight );
	texture.info.channels = 4; // always loaded in as rgba
	texture.info.layers = 1;
	texture.info.type = IMAGE_TYPE_2D;
	texture.info.fmt = IMAGE_FMT_RGBA_8;
	texture.info.tiling = IMAGE_TILING_MORTON;
	texture.uploadId = -1;
	texture.info.mipLevels = static_cast<uint32_t>( std::floor( std::log2( std::max( texture.info.width, texture.info.height ) ) ) ) + 1;
	texture.sizeBytes = ( texWidth * texHeight * 4 );
	texture.bytes = new uint8_t[ texture.sizeBytes ];
	memcpy( texture.bytes, pixels, texture.sizeBytes );

	stbi_image_free( pixels );
	return true;
}


bool LoadCubeMapImage( const char* textureBasePath, const char* ext, Image& texture )
{
	std::string paths[ 6 ] = {
		( std::string( textureBasePath ) + "_right." + ext ),
		( std::string( textureBasePath ) + "_left." + ext ),
		( std::string( textureBasePath ) + "_top." + ext ),
		( std::string( textureBasePath ) + "_bottom." + ext ),
		( std::string( textureBasePath ) + "_front." + ext ),
		( std::string( textureBasePath ) + "_back." + ext ),
	};

	int sizeBytes = 0;
	Image textures2D[ 6 ];
	for ( int i = 0; i < 6; ++i )
	{
		if ( LoadImage( paths[ i ].c_str(), textures2D[ i ] ) == false ) {
			sizeBytes = 0;
			break;
		}
		assert( textures2D[ i ].sizeBytes > 0 );
		sizeBytes += textures2D[ i ].sizeBytes;
	}

	if ( sizeBytes == 0 ) {
		for ( int i = 0; i < 6; ++i ) {
			if ( textures2D[ i ].bytes == nullptr ) {
				delete[] textures2D[ i ].bytes;
			}
		}
		return false;
	}

	const int texWidth = textures2D[ 0 ].info.width;
	const int texHeight = textures2D[ 0 ].info.height;
	const int texChannels = textures2D[ 0 ].info.channels;

	uint8_t* bytes = new uint8_t[ sizeBytes ];

	int byteOffset = 0;
	for ( int i = 0; i < 6; ++i )
	{
		if ( ( texWidth != textures2D[ i ].info.width ) ||
			( texHeight != textures2D[ i ].info.height ) ||
			( texChannels != textures2D[ i ].info.channels ) )
		{
			if ( bytes != nullptr ) {
				delete[] bytes;
			}
			for ( int j = 0; j < 6; ++j ) {
				if ( textures2D[ j ].bytes == nullptr ) {
					delete[] textures2D[ j ].bytes;
				}
			}
			return false;
		}

		memcpy( bytes + byteOffset, textures2D[ i ].bytes, textures2D[ i ].sizeBytes );
		byteOffset += textures2D[ i ].sizeBytes;
	}

	assert( sizeBytes == byteOffset );
	texture.info.width = texWidth;
	texture.info.height = texHeight;
	texture.info.channels = texChannels;
	texture.info.layers = 6;
	texture.info.type = IMAGE_TYPE_CUBE;
	texture.info.fmt = IMAGE_FMT_RGBA_8;
	texture.info.tiling = IMAGE_TILING_MORTON;
	texture.uploadId = -1;
	texture.info.mipLevels = static_cast<uint32_t>( std::floor( std::log2( std::max( texture.info.width, texture.info.height ) ) ) ) + 1;
	texture.bytes = bytes;
	texture.sizeBytes = sizeBytes;
	return true;
}


std::vector<char> ReadFile( const std::string& filename )
{
	std::ifstream file( filename, std::ios::ate | std::ios::binary );

	if ( !file.is_open() )
	{
		throw std::runtime_error( "Failed to open file!" );
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer( fileSize );

	file.seekg( 0 );
	file.read( buffer.data(), fileSize );
	file.close();

	return buffer;
}


bool LoadRawModel( AssetManager& assets, const std::string& fileName, const std::string& modelPath, const std::string& texturePath, Model& model )
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	if ( !tinyobj::LoadObj( &attrib, &shapes, &materials, &warn, &err, ( modelPath + fileName ).c_str(), modelPath.c_str() ) )
	{
		throw std::runtime_error( warn + err );
	}

	for ( const auto& material : materials )
	{
		const std::string supportedTextures[ 3 ] = {
			material.diffuse_texname,
			material.bump_texname,
			material.specular_texname,
		};

		for ( int i = 0; i < 3; ++i )
		{
			const std::string& name = supportedTextures[ i ];
			if( name.length() == 0 ) {
				continue;
			}
			assets.textureLib.AddDeferred( name.c_str(), pImgLoader_t( new ImageLoader( texturePath, name ) ) );
		}
		
		Material mat;
		if( material.dissolve == 1.0f )
		{
			mat.AddShader( DRAWPASS_SHADOW, AssetLibGpuProgram::Handle( "Shadow" ) );
			mat.AddShader( DRAWPASS_DEPTH, AssetLibGpuProgram::Handle( "LitDepth" ) );
			mat.AddShader( DRAWPASS_OPAQUE, AssetLibGpuProgram::Handle( "LitOpaque" ) );
		} else {
			mat.AddShader( DRAWPASS_TRANS, AssetLibGpuProgram::Handle( "LitTrans" ) );
		}
		mat.AddShader( DRAWPASS_DEBUG_WIREFRAME, AssetLibGpuProgram::Handle( "Debug" ) );
		mat.AddShader( DRAWPASS_DEBUG_SOLID, AssetLibGpuProgram::Handle( "DebugSolid" ) );

		mat.AddTexture( GGX_COLOR_MAP_SLOT, assets.textureLib.RetrieveHdl( supportedTextures[ 0 ].c_str() ) );
		mat.AddTexture( GGX_NORMAL_MAP_SLOT, assets.textureLib.RetrieveHdl( supportedTextures[ 1 ].c_str() ) );
		mat.AddTexture( GGX_SPEC_MAP_SLOT, assets.textureLib.RetrieveHdl( supportedTextures[ 2 ].c_str() ) );

		mat.Kd( rgbTuplef_t( material.diffuse[ 0 ], material.diffuse[ 1 ], material.diffuse[ 2 ] ) );
		mat.Ks( rgbTuplef_t( material.specular[ 0 ], material.specular[ 1 ], material.specular[ 2 ] ) );
		mat.Ka( rgbTuplef_t( material.ambient[ 0 ], material.ambient[ 1 ], material.ambient[ 2 ] ) );
		mat.Ke( rgbTuplef_t( material.emission[ 0 ], material.emission[ 1 ], material.emission[ 2 ] ) );
		mat.Tf( rgbTuplef_t( material.transmittance[ 0 ], material.transmittance[ 1 ], material.transmittance[ 2 ] ) );
		mat.Ni( material.ior );
		mat.Ns( material.shininess );
		mat.Tr( 1.0f - material.dissolve );
		mat.Illum( static_cast<float>( material.illum ) );

		assets.materialLib.Add( material.name.c_str(), mat );
	}

	uint32_t vertexCnt = 0;
	//model.surfs[ 0 ].vertices.reserve( attrib.vertices.size() );
	model.surfCount = 0;
	model.surfs.resize( shapes.size() );
	for ( const auto& shape : shapes )
	{
		bool hasUv = true;

		std::unordered_map<vertex_t, uint32_t> uniqueVertices{};
		std::unordered_map< uint32_t, uint32_t > indexFaceCount{};
		for ( const auto& index : shape.mesh.indices )
		{
			vertex_t vertex{ };

			vertex.pos[ 0 ] = attrib.vertices[ 3 * index.vertex_index + 0 ];
			vertex.pos[ 1 ] = attrib.vertices[ 3 * index.vertex_index + 1 ];
			vertex.pos[ 2 ] = attrib.vertices[ 3 * index.vertex_index + 2 ];

			model.surfs[ model.surfCount ].centroid += vec3f( vertex.pos );

			model.bounds.Expand( vec3f( vertex.pos[ 0 ], vertex.pos[ 1 ], vertex.pos[ 2 ] ) );

			vertex.uv[ 0 ] = 0.0f;
			vertex.uv[ 1 ] = 0.0f;
			vertex.uv2[ 0 ] = 0.0f;
			vertex.uv2[ 1 ] = 0.0f;
			if( index.texcoord_index >= 0 )
			{
				vertex.uv[ 0 ] = attrib.texcoords[ 2 * index.texcoord_index + 0 ];
				vertex.uv[ 1 ] = 1.0f - attrib.texcoords[ 2 * index.texcoord_index + 1 ];
			} else {
				hasUv = false;
			}

			vertex.normal[ 0 ] = attrib.normals[ 3 * index.normal_index + 0 ];
			vertex.normal[ 1 ] = attrib.normals[ 3 * index.normal_index + 1 ];
			vertex.normal[ 2 ] = attrib.normals[ 3 * index.normal_index + 2 ];

			vertex.color[ 0 ] = attrib.colors[ 3 * index.vertex_index + 0 ];
			vertex.color[ 1 ] = attrib.colors[ 3 * index.vertex_index + 1 ];
			vertex.color[ 2 ] = attrib.colors[ 3 * index.vertex_index + 2 ];
			vertex.color[ 3 ] = 1.0f;

			if ( uniqueVertices.count( vertex ) == 0 )
			{
				model.surfs[ model.surfCount ].vertices.push_back( vertex );
				uniqueVertices[ vertex ] = static_cast<uint32_t>( model.surfs[ model.surfCount ].vertices.size() - 1 );
			}

			const uint32_t index = uniqueVertices[ vertex ];
			model.surfs[ model.surfCount ].indices.push_back( index );
			indexFaceCount[ uniqueVertices[ vertex ] ]++;
		}

		const int indexCount = static_cast<int>( model.surfs[ model.surfCount ].indices.size() );
		assert( ( indexCount % 3 ) == 0 );

		// Eric Lengyel "Computing Tangent Basis Vectors for an Arbitrary Mesh"
		for ( int i = 0; i < indexCount; i += 3 ) {
			int indices[ 3 ];
			float weights[ 3 ];
			indices[ 0 ] = model.surfs[ model.surfCount ].indices[ i + 0 ];
			indices[ 1 ] = model.surfs[ model.surfCount ].indices[ i + 1 ];
			indices[ 2 ] = model.surfs[ model.surfCount ].indices[ i + 2 ];

			assert( indexFaceCount[ indices[ 0 ] ] > 0 );
			assert( indexFaceCount[ indices[ 1 ] ] > 0 );
			assert( indexFaceCount[ indices[ 2 ] ] > 0 );

			weights[ 0 ] = ( 1.0f / indexFaceCount[ indices[ 0 ] ] );
			weights[ 1 ] = ( 1.0f / indexFaceCount[ indices[ 1 ] ] );
			weights[ 2 ] = ( 1.0f / indexFaceCount[ indices[ 2 ] ] );

			vertex_t& v0 = model.surfs[ model.surfCount ].vertices[ indices[ 0 ] ];
			vertex_t& v1 = model.surfs[ model.surfCount ].vertices[ indices[ 1 ] ];
			vertex_t& v2 = model.surfs[ model.surfCount ].vertices[ indices[ 2 ] ];

			const vec3f edge0 = vec3f( v1.pos - v0.pos );
			const vec3f edge1 = vec3f( v2.pos - v0.pos );

			const vec3f faceNormal = ( v0.normal + v1.normal + v2.normal ).Normalize();
			vec3f faceTangent;
			vec3f faceBitangent;

			vec2f uvEdgeDt0;
			vec2f uvEdgeDt1;

			if( hasUv )
			{
				uvEdgeDt0 = ( v1.uv - v0.uv );
				uvEdgeDt1 = ( v2.uv - v0.uv );
			}
			else
			{
				uvEdgeDt0 = vec2f( 1.0f, 0.0f );
				uvEdgeDt1 = vec2f( 0.0f, 1.0f );
			}

			const float denom = ( uvEdgeDt0[ 0 ] * uvEdgeDt1[ 1 ] - uvEdgeDt1[ 0 ] * uvEdgeDt0[ 1 ] ) + 0.00001f;
			if( denom != 0.0f )
			{
				const float r = 1.0f / denom;
				faceTangent = ( edge0 * uvEdgeDt1[ 1 ] - edge1 * uvEdgeDt0[ 1 ] ) * r;
				faceBitangent = ( edge1 * uvEdgeDt0[ 0 ] - edge0 * uvEdgeDt1[ 0 ] ) * r;

				v0.tangent += weights[ 0 ] * faceTangent;
				v0.bitangent += weights[ 0 ] * faceBitangent;

				v1.tangent += weights[ 1 ] * faceTangent;
				v1.bitangent += weights[ 1 ] * faceBitangent;

				v2.tangent += weights[ 2 ] * faceTangent;
				v2.bitangent += weights[ 2 ] * faceBitangent;
			}
		}

		const int vertexCount = static_cast<int>( model.surfs[ model.surfCount ].vertices.size() );
		for ( int i = 0; i < vertexCount; ++i ) {
			vertex_t& v = model.surfs[ model.surfCount ].vertices[ i ];
			v.tangent.FlushDenorms();
			v.bitangent.FlushDenorms();
			v.normal.FlushDenorms();

			// Gram-Schmidt orthogonalize
			v.normal = v.normal.Normalize();
			v.tangent = v.tangent.Normalize();
			v.tangent = ( v.tangent - v.normal * Dot( v.normal, v.tangent ) ).Normalize();
			v.bitangent = v.bitangent.Normalize();

			const uint32_t signBit = ( Dot( Cross( v.tangent, v.bitangent ), v.normal ) > 0.0f ) ? 0 : 1;
			union tangentBitPack_t {
				struct {
					uint32_t signBit : 1;
					uint32_t vecBits : 31;
				};
				float value;
			};
			tangentBitPack_t packed;
			packed.value = v.tangent[ 0 ];
			packed.signBit = signBit;
			v.tangent[ 0 ] = packed.value;

			//assert( fabs( v.normal.Length() - 1.0f ) < 0.001f );
			//assert( fabs( v.tangent.Length() - 1.0f ) < 0.001f );
			//assert( fabs( v.bitangent.Length() - 1.0f ) < 0.001f );

			float tsValues[ 9 ] = { v.tangent[ 0 ], v.tangent[ 1 ], v.tangent[ 2 ],
									v.bitangent[ 0 ], v.bitangent[ 1 ], v.bitangent[ 2 ],
									v.normal[ 0 ], v.normal[ 1 ], v.normal[ 2 ] };
			mat3x3f tsMatrix = mat3x3f( tsValues );
			//assert( tsMatrix.IsOrthonormal( 0.01f ) );
		}

		model.surfs[ model.surfCount ].materialHdl = INVALID_HDL;
		if ( ( materials.size() > 0 ) && ( shape.mesh.material_ids.size() > 0 ) ) {
			const int shapeMaterial = shape.mesh.material_ids[ 0 ];
			const hdl_t materialHdl = AssetLibMaterials::Handle( materials[ shapeMaterial ].name.c_str() );
			if ( materialHdl.IsValid() ) {
				model.surfs[ model.surfCount ].materialHdl = materialHdl;
			}
		}
		++model.surfCount;
	}
	return true;
}


bool LoadModel( Model& model, const hdl_t& hdl, const std::string& bakePath, const std::string& modelPath, const std::string& ext )
{
	Serializer* s = new Serializer( MB( 8 ), serializeMode_t::LOAD );
	std::string fileName = bakePath + modelPath + hdl.String() + ext;

	if ( !s->ReadFile( fileName ) ) {
		return false;
	}

	uint8_t name[ 256 ];
	uint32_t nameLength = 0;
	memset( name, 0, 256 );
	s->Next( nameLength );
	assert( nameLength < 256 );
	s->NextArray( name, nameLength );

	name[ nameLength ] = '2'; // FIXME: test

	model.Serialize( s );
	return true;
}


bool WriteModel( Asset<Model>* model, const std::string& fileName )
{
	if ( model == nullptr ) {
		return false;
	}
	std::string name = model->GetName();
	Serializer* s = new Serializer( MB( 8 ), serializeMode_t::STORE );

	uint8_t buffer[ 256 ];
	assert( name.length() < 256 );
	uint32_t nameLength = static_cast<uint32_t>( name.length() );
	memcpy( buffer, name.c_str(), nameLength );
	s->Next( nameLength );
	s->NextArray( buffer, nameLength );

	model->Get().Serialize( s );
	s->WriteFile( fileName );
	return true;
}