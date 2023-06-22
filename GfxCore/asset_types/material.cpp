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

#include "material.h"
#include <serializer.h>
#include <systemUtils.h>
#include "../io/serializeClasses.h"
#include "../scene/assetManager.h"

bool Material::AddTexture( const uint32_t slot, const hdl_t hdl )
{
	if ( slot >= MaxMaterialTextures ) {
		return false;
	}
	if ( hdl.IsValid() == false ) {
		return false;
	}
	textures[ slot ] = hdl;
	textureBitSet |= ( 1 << slot );
	return true;
}


hdl_t Material::GetTexture( const uint32_t slot ) const
{
	if ( slot >= MaxMaterialTextures ) {
		return INVALID_HDL;
	}
	return textures[ slot ];
}


uint32_t Material::TextureCount() const
{
	uint32_t count = 0;
	uint32_t bits = textureBitSet;
	while ( bits )
	{
		bits &= ( bits - 1 );
		count++;
	}
	return count;
}


bool Material::AddShader( const drawPass_t pass, const hdl_t hdl )
{
	const uint32_t slot = uint32_t( pass );
	if ( slot >= MaxMaterialShaders ) {
		return false;
	}
	if ( hdl.IsValid() == false ) {
		return false;
	}
	shaders[ slot ] = hdl;
	shaderBitSet |= ( 1 << slot );
	return true;
}


hdl_t Material::GetShader( const drawPass_t pass ) const
{
	const uint32_t slot = uint32_t( pass );
	if ( slot >= MaxMaterialShaders ) {
		return INVALID_HDL;
	}
	return shaders[ slot ];
}


uint32_t Material::ShaderCount() const
{
	uint32_t count = 0;
	uint32_t bits = shaderBitSet;
	while ( bits )
	{
		bits &= ( bits - 1 );
		count++;
	}
	return count;
}


bool BakedMaterialLoader::Load( Asset<Material>& materialAsset )
{
	Material& material = materialAsset.Get();

	bakedAssetInfo_t info = {};
	const bool loadedBaked = LoadBaked( materialAsset, info, m_basePath, "mtl.bin" );
	if ( loadedBaked )
	{
		assert( m_assets != nullptr );
		const uint32_t imgCount = material.TextureCount();
		for ( uint32_t imageIx = 0; imageIx < imgCount; ++imageIx )
		{
			const hdl_t imgHandle = material.GetTexture( imageIx );
			m_assets->textureLib.AddDeferred( imgHandle, pImgLoader_t( new BakedImageLoader( ".\\textures\\", "img.bin" ) ) );
		}
		return true;
	}
	return false;
}


void BakedMaterialLoader::SetBasePath( const std::string& path )
{
	m_basePath = path;
}


void BakedMaterialLoader::SetExtName( const std::string& ext )
{
	m_ext = ext;
}


void BakedMaterialLoader::SetAssetRef( AssetManager* assetsPtr )
{
	m_assets = assetsPtr;
}

// BRDF functions are adapted from "https://google.github.io/filament/Filament.md.html#overview/physicallybasedrendering"
float GGX( float NoH, float roughness )
{
	float a = NoH * roughness;
	float k = roughness / ( 1.0f - NoH * NoH + a * a );
	return k * k * ( 1.0f / PI );
}


float SmithGGXCorrelated( float NoV, float NoL, float roughness )
{
	float a2 = roughness * roughness;
	float GGXV = NoL * sqrt( NoV * NoV * ( 1.0f - a2 ) + a2 );
	float GGXL = NoV * sqrt( NoL * NoL * ( 1.0f - a2 ) + a2 );
	return 0.5f / ( GGXV + GGXL );
}


vec3f Schlick( float u, vec3f f0 )
{
	return f0 + ( vec3f( 1.0f ) - f0 ) * pow( 1.0f - u, 5.0f );
}


float Lambert()
{
	return 1.0f / PI;
}


vec3f BrdfGGX( const vec3f& n, const vec3f& v, const vec3f& l, const Material& m )
{
	float perceptualRoughness = 1.0f;
	float f0 = 0.1f;

	vec3f h = ( v + l ).Normalize();

	float NoV = abs( Dot( n, v ) ) + 1e-5f;
	float NoL = Clamp( Dot( n, l ), 0.0f, 1.0f );
	float NoH = Clamp( Dot( n, h ), 0.0f, 1.0f );
	float LoH = Clamp( Dot( l, h ), 0.0f, 1.0f );

	// perceptually linear roughness to roughness (see parameterization)
	float roughness = perceptualRoughness * perceptualRoughness;

	float D = GGX( NoH, roughness );
	vec3f  F = Schlick( LoH, f0 );
	float V = SmithGGXCorrelated( NoV, NoL, roughness );

	// specular BRDF
	vec3f Fr = ( D * V ) * F;

	// diffuse BRDF
	vec3f Fd = vec3f( 1.0f, 0.0f, 0.0f ) * Lambert();

	return ( Fr + Fd ) * std::max( 0.0f, Dot( n, l ) ); // TODO:
}