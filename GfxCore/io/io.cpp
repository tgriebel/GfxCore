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

#include <syscore/serializer.h>
#include <syscore/common.h>

#include "../scene/scene.h"
#include "../scene/assetManager.h"
#include "../asset_types/model.h"
#include "../asset_types/texture.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "../../external/tiny_obj_loader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../../external/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#pragma warning(push)
#pragma warning(disable : 4996) // sprintf
#include "../../external/stb_image_write.h"
#pragma warning(pop)


bool LoadImage( const char* texturePath, const bool isLinearColor, Image& texture )
{
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load( texturePath, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha );

	if ( !pixels ) {
		stbi_image_free( pixels );
		return false;
	}

	//assert( texChannels == 4 );
	imageInfo_t info = DefaultImage2dInfo( texWidth, texHeight );
	if ( isLinearColor ) {
		info.fmt = imageFmt_t::IMAGE_FMT_RGBA_8_UNORM;
	}
	texture.Create( info, pixels, info.width * info.height * 4 );

	stbi_image_free( pixels );
	return true;
}


bool LoadImageHDR( const char* texturePath, Image& texture )
{
	int texWidth, texHeight, texChannels;
	float* elements = stbi_loadf( texturePath, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha );

	if ( !elements ) {
		stbi_image_free( elements );
		return false;
	}

	//assert( texChannels == 4 );
	imageInfo_t info = DefaultImage2dInfo( texWidth, texHeight );
	info.fmt = IMAGE_FMT_RGBA_16;

	assert( texture.cpuImage == nullptr );

	ImageBuffer<rgba16_t>* imageBuffer = new ImageBuffer<rgba16_t>( info.width, info.height, info.layers );
	
	const uint32_t elementCount = 4 * imageBuffer->GetPixelCount();

	uint16_t* buffer = reinterpret_cast<uint16_t*>( imageBuffer->Ptr() );
	for ( uint32_t i = 0; i < elementCount; ++i ) {
		buffer[ i ] = PackFloat32( elements[ i ] );
	}

	texture.Create( info, imageBuffer, nullptr );

	stbi_image_free( elements );
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
		if ( LoadImage( paths[ i ].c_str(), false, textures2D[ i ] ) == false ) {
			sizeBytes = 0;
			break;
		}
		assert( textures2D[ i ].cpuImage != nullptr );
		assert( textures2D[ i ].cpuImage->GetByteCount() > 0 );
		sizeBytes += textures2D[ i ].cpuImage->GetByteCount();
	}

	if ( sizeBytes == 0 ) {
		for ( int i = 0; i < 6; ++i )
		{
			delete textures2D[ i ].cpuImage;
			textures2D[ i ].cpuImage = nullptr;
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
			for ( int j = 0; j < 6; ++j )
			{
				delete textures2D[ i ].cpuImage;
				textures2D[ i ].cpuImage = nullptr;
			}
			return false;
		}

		memcpy( bytes + byteOffset, textures2D[ i ].cpuImage->Ptr(), textures2D[ i ].cpuImage->GetByteCount() );
		byteOffset += textures2D[ i ].cpuImage->GetByteCount();
	}

	assert( sizeBytes == byteOffset );
	imageInfo_t info = DefaultImage2dInfo( texWidth, texHeight );
	info.channels = texChannels;
	info.layers = 6;
	info.type = IMAGE_TYPE_CUBE;

	texture.Create( info, bytes, sizeBytes );

	delete[] bytes;

	return true;
}


bool WriteImage( const char* path, const Image& image )
{
	std::string fileName;
	std::string ext;
	SplitFileName( path, fileName, ext );

	if( ext == "png" )
	{
		const int ret = stbi_write_png( path, image.info.width, image.info.height, image.info.channels, image.cpuImage->Ptr(), image.info.width * image.cpuImage->GetBpp() );
		return ret == 1;
	}
	else if( ext == "jpg" )
	{
		const int ret = stbi_write_jpg( path, image.info.width, image.info.height, image.info.channels, image.cpuImage->Ptr(), 100 );
		return ret == 1;
	}
	else if ( ext == "bmp" )
	{
		const int ret = stbi_write_bmp( path, image.info.width, image.info.height, image.info.channels, image.cpuImage->Ptr() );
		return ret == 1;
	}
	else if ( ext == "hdr" )
	{
		const int ret = stbi_write_hdr( path, image.info.width, image.info.height, image.info.channels, (float*)image.cpuImage->Ptr() );
		return ret == 1;
	}
	return false;
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
		const bool isPbr = material.roughness || material.metallic || !material.roughness_texname.empty() || !material.metallic_texname.empty();

		struct loadInfo_t
		{
			const std::string& name;
			bool isLinear;
		};

		std::vector<loadInfo_t> supportedTextures;
		
		if( isPbr )
		{
			supportedTextures.push_back( loadInfo_t{ material.diffuse_texname, false } );
			supportedTextures.push_back( loadInfo_t{ material.normal_texname, true } );
			supportedTextures.push_back( loadInfo_t{ material.roughness_texname, true } );
			supportedTextures.push_back( loadInfo_t{ material.metallic_texname, true } );
		}
		else
		{
			supportedTextures.push_back( loadInfo_t{ material.diffuse_texname, false } );
			supportedTextures.push_back( loadInfo_t{ material.bump_texname, true } );
			supportedTextures.push_back( loadInfo_t{ material.specular_texname, true } );
		}

		const uint32_t textureCount = static_cast<uint32_t>( supportedTextures.size() );
		for ( uint32_t i = 0; i < textureCount; ++i )
		{
			const std::string& name = supportedTextures[ i ].name;
			if( name.length() == 0 ) {
				continue;
			}
			assets.textureLib.AddDeferred( name.c_str(), pImgLoader_t( new ImageLoader( texturePath, name, supportedTextures[ i ].isLinear ) ) );
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
		mat.AddShader( DRAWPASS_DEBUG_3D, AssetLibGpuProgram::Handle( "DebugSolid" ) );

		if( isPbr  )
		{
			mat.usage = materialUsage_t::MATERIAL_USAGE_GGX;

			mat.AddTexture( GGX_COLOR_MAP_SLOT, assets.textureLib.RetrieveHdl( supportedTextures[ 0 ].name.c_str() ) );
			mat.AddTexture( GGX_NORMAL_MAP_SLOT, assets.textureLib.RetrieveHdl( supportedTextures[ 1 ].name.c_str() ) );
			mat.AddTexture( GGX_SPEC_MAP_SLOT, assets.textureLib.RetrieveHdl( supportedTextures[ 2 ].name.c_str() ) );
			mat.AddTexture( GGX_METALLIC_MAP_SLOT, assets.textureLib.RetrieveHdl( supportedTextures[ 3 ].name.c_str() ) );
		}
		else
		{
			// FIXME: Keep this as PBR for now as materials need to be updated
			// mat.usage = materialUsage_t::MATERIAL_USAGE_BLINN_PHONG;
			mat.usage = materialUsage_t::MATERIAL_USAGE_GGX;

			mat.AddTexture( GGX_COLOR_MAP_SLOT, assets.textureLib.RetrieveHdl( supportedTextures[ 0 ].name.c_str() ) );
			mat.AddTexture( GGX_NORMAL_MAP_SLOT, assets.textureLib.RetrieveHdl( supportedTextures[ 1 ].name.c_str() ) );
			mat.AddTexture( GGX_SPEC_MAP_SLOT, assets.textureLib.RetrieveHdl( supportedTextures[ 2 ].name.c_str() ) );
		}

		mat.Kd( rgb32_t( material.diffuse[ 0 ], material.diffuse[ 1 ], material.diffuse[ 2 ] ) );
		mat.Ks( rgb32_t( material.specular[ 0 ], material.specular[ 1 ], material.specular[ 2 ] ) );
		mat.Ka( rgb32_t( material.ambient[ 0 ], material.ambient[ 1 ], material.ambient[ 2 ] ) );
		mat.Ke( rgb32_t( material.emission[ 0 ], material.emission[ 1 ], material.emission[ 2 ] ) );
		mat.Tf( rgb32_t( material.transmittance[ 0 ], material.transmittance[ 1 ], material.transmittance[ 2 ] ) );
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