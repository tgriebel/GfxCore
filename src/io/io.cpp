#include "io.h"

#include <map>
#include <deque>
#include <string>
#include <sstream>
#include <unordered_map>

#include <serializer.h>
#include <common.h>

#include "../scene/scene.h"
#include "../resource_types/model.h"
#include "../resource_types/texture.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "../../external/tiny_obj_loader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../../external/stb_image.h"


bool LoadTextureImage( const char* texturePath, texture_t& texture )
{
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load( texturePath, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha );

	if ( !pixels ) {
		stbi_image_free( pixels );
		return false;
	}

	texture.info.width = texWidth;
	texture.info.height = texHeight;
	texture.info.channels = texChannels;
	texture.info.layers = 1;
	texture.info.type = TEXTURE_TYPE_2D;
	texture.uploadId = -1;
	texture.info.mipLevels = static_cast<uint32_t>( std::floor( std::log2( std::max( texture.info.width, texture.info.height ) ) ) ) + 1;
	texture.sizeBytes = ( texWidth * texHeight * 4 );
	texture.bytes = new uint8_t[ texture.sizeBytes ];
	memcpy( texture.bytes, pixels, texture.sizeBytes );
	stbi_image_free( pixels );
	return true;
}


bool LoadTextureCubeMapImage( const char* textureBasePath, const char* ext, texture_t& texture )
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
	texture_t textures2D[ 6 ];
	for ( int i = 0; i < 6; ++i )
	{
		if ( LoadTextureImage( paths[ i ].c_str(), textures2D[ i ] ) == false ) {
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
	texture.info.type = TEXTURE_TYPE_CUBE;
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


hdl_t LoadRawModel( Scene& scene, const std::string& fileName, const std::string& objectName, const std::string& modelPath, const std::string& texturePath )
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
		texture_t texture;
		const std::string supportedTextures[ 3 ] = {
			texturePath + material.diffuse_texname,
			texturePath + material.bump_texname,
			texturePath + material.specular_texname,
		};
		uint32_t texCount = 0;
		for ( int i = 0; i < 3; ++i )
		{
			if ( LoadTextureImage( supportedTextures[ i ].c_str(), texture ) )
			{
				++texCount;
				texture.uploadId = -1;
				texture.info.mipLevels = static_cast<uint32_t>( std::floor( std::log2( std::max( texture.info.width, texture.info.height ) ) ) ) + 1;
				scene.textureLib.Add( supportedTextures[ i ].c_str(), texture );
			}
		}
		
		Material mat;
		mat.shaders[ DRAWPASS_SHADOW ] = scene.gpuPrograms.RetrieveHdl( "Shadow" );
		mat.shaders[ DRAWPASS_DEPTH ] = scene.gpuPrograms.RetrieveHdl( "LitDepth" );
		mat.shaders[ DRAWPASS_OPAQUE ] = scene.gpuPrograms.RetrieveHdl( "LitOpaque" );
		mat.shaders[ DRAWPASS_DEBUG_WIREFRAME ] = scene.gpuPrograms.RetrieveHdl( "Debug" ); // FIXME: do this with an override
		mat.shaders[ DRAWPASS_DEBUG_SOLID ] = scene.gpuPrograms.RetrieveHdl( "Debug_Solid" );
		mat.textures[ 0 ] = scene.textureLib.RetrieveHdl( supportedTextures[ 0 ].c_str() );
		mat.textures[ 1 ] = scene.textureLib.RetrieveHdl( supportedTextures[ 1 ].c_str() );
		mat.textures[ 2 ] = scene.textureLib.RetrieveHdl( supportedTextures[ 2 ].c_str() );
		mat.textured = ( texCount > 0 );

		mat.Kd = rgbTuplef_t( material.diffuse[ 0 ], material.diffuse[ 1 ], material.diffuse[ 2 ] );
		mat.Ks = rgbTuplef_t( material.specular[ 0 ], material.specular[ 1 ], material.specular[ 2 ] );
		mat.Ka = rgbTuplef_t( material.ambient[ 0 ], material.ambient[ 1 ], material.ambient[ 2 ] );
		mat.Ke = rgbTuplef_t( material.emission[ 0 ], material.emission[ 1 ], material.emission[ 2 ] );
		mat.Tf = rgbTuplef_t( material.transmittance[ 0 ], material.transmittance[ 1 ], material.transmittance[ 2 ] );
		mat.Ni = material.ior;
		mat.Ns = material.shininess;
		mat.d = material.dissolve;
		mat.Tr = ( 1.0f - material.dissolve );
		mat.illum = static_cast<float>( material.illum );
		scene.materialLib.Add( material.name.c_str(), mat );
	}

	Model model;

	uint32_t vertexCnt = 0;
	//model.surfs[ 0 ].vertices.reserve( attrib.vertices.size() );
	model.surfCount = 0;

	for ( const auto& shape : shapes )
	{
		std::unordered_map<vertex_t, uint32_t> uniqueVertices{};
		std::unordered_map< uint32_t, uint32_t > indexFaceCount{};
		for ( const auto& index : shape.mesh.indices )
		{
			vertex_t vertex{ };

			vertex.pos[ 0 ] = attrib.vertices[ 3 * index.vertex_index + 0 ];
			vertex.pos[ 1 ] = attrib.vertices[ 3 * index.vertex_index + 1 ];
			vertex.pos[ 2 ] = attrib.vertices[ 3 * index.vertex_index + 2 ];

			model.bounds.Expand( vec3f( vertex.pos[ 0 ], vertex.pos[ 1 ], vertex.pos[ 2 ] ) );

			vertex.uv[ 0 ] = attrib.texcoords[ 2 * index.texcoord_index + 0 ];
			vertex.uv[ 1 ] = 1.0f - attrib.texcoords[ 2 * index.texcoord_index + 1 ];
			vertex.uv2[ 0 ] = 0.0f;
			vertex.uv2[ 1 ] = 0.0f;

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

			model.surfs[ model.surfCount ].indices.push_back( uniqueVertices[ vertex ] );
			indexFaceCount[ uniqueVertices[ vertex ] ]++;
		}

		const int indexCount = static_cast<int>( model.surfs[ model.surfCount ].indices.size() );
		assert( ( indexCount % 3 ) == 0 );

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

			const vec2f uvEdgeDt0 = ( v1.uv - v0.uv );
			const vec2f uvEdgeDt1 = ( v2.uv - v0.uv );
			float uDt = ( v1.uv[ 0 ] - v0.uv[ 0 ] );

			const float r = 1.0f / ( uvEdgeDt0[ 0 ] * uvEdgeDt1[ 1 ] - uvEdgeDt1[ 0 ] * uvEdgeDt0[ 1 ] );

			const vec3f edge0 = Trunc<4, 1>( v1.pos - v0.pos );
			const vec3f edge1 = Trunc<4, 1>( v2.pos - v0.pos );

			const vec3f faceNormal = Cross( edge0, edge1 ).Normalize();
			if ( faceNormal.Length() < 0.001f ) {
				continue; // TODO: remove?
			}

			const vec3f faceTangent = ( edge0 * uvEdgeDt1[ 1 ] - edge1 * uvEdgeDt0[ 1 ] ) * r;
			const vec3f faceBitangent = ( edge1 * uvEdgeDt0[ 0 ] - edge0 * uvEdgeDt1[ 0 ] ) * r;

			v0.tangent += weights[ 0 ] * faceTangent;
			v0.bitangent += weights[ 0 ] * faceBitangent;
			v0.normal += weights[ 0 ] * faceNormal;

			v1.tangent += weights[ 1 ] * faceTangent;
			v1.bitangent += weights[ 1 ] * faceBitangent;
			v1.normal += weights[ 1 ] * faceNormal;

			v2.tangent += weights[ 2 ] * faceTangent;
			v2.bitangent += weights[ 2 ] * faceBitangent;
			v2.normal += weights[ 2 ] * faceNormal;
		}

		const int vertexCount = static_cast<int>( model.surfs[ model.surfCount ].vertices.size() );
		for ( int i = 0; i < vertexCount; ++i ) {
			vertex_t& v = model.surfs[ model.surfCount ].vertices[ i ];
			v.tangent.FlushDenorms();
			v.bitangent.FlushDenorms();
			v.normal.FlushDenorms();
			// Re-orthonormalize
			// TODO: use graham-schmidt?
			v.tangent = v.tangent.Normalize();
			v.bitangent = Cross( v.tangent, v.normal ).Normalize();
			v.normal = Cross( v.tangent, v.bitangent ).Normalize();

			const uint32_t signBit = ( Dot( Cross( v.tangent, v.bitangent ), v.normal ) > 0.0f ) ? 1 : 0;
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
			const hdl_t materialHdl = scene.materialLib.RetrieveHdl( materials[ shapeMaterial ].name.c_str() );
			if ( materialHdl.IsValid() ) {
				model.surfs[ model.surfCount ].materialHdl = materialHdl;
			}
		}
		++model.surfCount;
	}
	return scene.modelLib.Add( objectName.c_str(), model );
}


bool LoadModel( Scene& scene, const hdl_t& hdl, const std::string& bakePath, const std::string& modelPath, const std::string& ext )
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

	hdl_t modelHdl = scene.modelLib.Add( reinterpret_cast<char*>( &name[ 0 ] ), Model() );
	Model* model = scene.modelLib.Find( modelHdl );

	model->Serialize( s );
	return true;
}


bool WriteModel( Scene& scene, const std::string& fileName, hdl_t modelHdl )
{
	Model* model = scene.modelLib.Find( modelHdl );
	if ( model == nullptr ) {
		return false;
	}
	std::string name = scene.modelLib.FindName( modelHdl );
	Serializer* s = new Serializer( MB( 8 ), serializeMode_t::STORE );

	uint8_t buffer[ 256 ];
	assert( name.length() < 256 );
	uint32_t nameLength = static_cast<uint32_t>( name.length() );
	memcpy( buffer, name.c_str(), nameLength );
	s->Next( nameLength );
	s->NextArray( buffer, nameLength );

	model->Serialize( s );
	s->WriteFile( fileName );
	return true;
}