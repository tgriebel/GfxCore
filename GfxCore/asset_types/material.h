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

#pragma once

#include <cinttypes>
#include "../core/common.h"
#include "../math/vector.h"
#include "../math/matrix.h"
#include "../image/color.h"
#include "../core/handle.h"
#include "../core/asset.h"

class Serializer;

struct lightInput_t
{
	vec3f	viewVector;
	vec3f	normal;
};

enum drawPass_t : uint32_t
{
	DRAWPASS_SHADOW,
	DRAWPASS_DEPTH,
	DRAWPASS_TERRAIN,
	DRAWPASS_OPAQUE,
	DRAWPASS_SKYBOX,
	DRAWPASS_TRANS,
	DRAWPASS_EMISSIVE,
	DRAWPASS_DEBUG_3D,
	DRAWPASS_DEBUG_WIREFRAME,
	DRAWPASS_POST_2D,
	DRAWPASS_DEBUG_2D,
	DRAWPASS_COUNT,

	DRAWPASS_SHADOW_BEGIN = DRAWPASS_SHADOW,
	DRAWPASS_SHADOW_END = DRAWPASS_SHADOW,
	DRAWPASS_MAIN_BEGIN = DRAWPASS_DEPTH,
	DRAWPASS_MAIN_END = DRAWPASS_DEBUG_WIREFRAME,
	DRAWPASS_POST_BEGIN = DRAWPASS_POST_2D,
	DRAWPASS_POST_END = DRAWPASS_DEBUG_2D,
};

enum ggxTextureSlot_t : uint32_t
{
	GGX_COLOR_MAP_SLOT,
	GGX_NORMAL_MAP_SLOT,
	GGX_SPEC_MAP_SLOT,
};

enum cubeTextureSlot_t : uint32_t
{
	CUBE_RIGHT_SLOT,
	CUBE_LEFT_SLOT,
	CUBE_TOP_SLOT,
	CUBE_BOTTOM_SLOT,
	CUBE_FRONT_SLOT,
	CUBE_BACK_SLOT,
};

enum hgtTextureSlot_t : uint32_t
{
	HGT_HEIGHT_MAP_SLOT,
	HGT_COLOR_MAP_SLOT0,
	HGT_COLOR_MAP_SLOT1,
};

enum materialUsage_t : uint32_t
{
	MATERIAL_USAGE_UNKNOWN,
	MATERIAL_USAGE_CODE,
	MATERIAL_USAGE_GGX,
	MATERIAL_USAGE_HEIGHT_MAP,
	MATERIAL_USAGE_CUBE,
};


struct materialParms_t
{
	materialParms_t() : Tr( 0.0f ), Ns( 0.0f ), Ni( 0.0f ), d( 1.0f ), illum( 0.0f ) {}

	rgbTuplef_t	Ka;
	rgbTuplef_t	Ke;
	rgbTuplef_t	Kd;
	rgbTuplef_t	Ks;
	rgbTuplef_t	Tf;
	float		Tr;
	float		Ns;
	float		Ni;
	float		d;
	float		illum;
};

class Material
{
public:
	static const uint32_t Version = 1;
	static const uint32_t MaxMaterialTextures = 8;
	static const uint32_t MaxMaterialShaders = DRAWPASS_COUNT;

	int32_t					uploadId;
	materialUsage_t			usage;

private:
	materialParms_t			p;
	uint16_t				textureBitSet;
	uint16_t				shaderBitSet;

	hdl_t					textures[ MaxMaterialTextures ];
	hdl_t					shaders[ MaxMaterialShaders ];

public:
	Material() :
		textureBitSet( 0 ),
		shaderBitSet( 0 )
	{
		p.Kd = rgbTuplef_t( 1.0f, 1.0f, 1.0f );
		for ( int i = 0; i < MaxMaterialTextures; ++i ) {
			textures[ i ] = INVALID_HDL;
		}
		for ( int i = 0; i < MaxMaterialShaders; ++i ) {
			shaders[ i ] = INVALID_HDL;
		}
		uploadId = -1;
		usage = MATERIAL_USAGE_UNKNOWN;
	}

	inline void SetParms( const materialParms_t& parms )
	{
		p = parms;
	}

	inline const rgbTuplef_t& Kd() const
	{
		return p.Kd;
	}

	inline void Kd( const rgbTuplef_t rgb )
	{
		p.Kd = rgb;
	}

	inline const rgbTuplef_t& Ks() const
	{
		return p.Ks;
	}

	inline void Ks( const rgbTuplef_t rgb )
	{
		p.Ks = rgb;
	}

	inline const rgbTuplef_t& Ke() const
	{
		return p.Ke;
	}

	inline void Ke( const rgbTuplef_t rgb )
	{
		p.Ke = rgb;
	}

	inline const rgbTuplef_t& Ka() const
	{
		return p.Ka;
	}

	inline void Ka( const rgbTuplef_t rgb )
	{
		p.Ka = rgb;
	}

	inline const rgbTuplef_t& Tf() const
	{
		return p.Tf;
	}

	inline void Tf( const rgbTuplef_t rgb )
	{
		p.Tf = rgb;
	}

	inline const float& Tr() const
	{
		return p.Tr;
	}

	inline void Tr( const float rgb )
	{
		p.Tr = rgb;
	}

	inline const float& Ns() const
	{
		return p.Ns;
	}

	inline void Ns( const float rgb )
	{
		p.Ns = rgb;
	}

	inline const float& Ni() const
	{
		return p.Ni;
	}

	inline void Ni( const float rgb )
	{
		p.Ni = rgb;
	}

	inline const float& Illum() const
	{
		return p.illum;
	}

	inline void Illum( const float rgb )
	{
		p.illum = rgb;
	}

	inline bool IsTextured() const
	{
		return ( textureBitSet > 0 );
	}

	bool		AddTexture( const uint32_t slot, const hdl_t hdl );
	hdl_t		GetTexture( const uint32_t slot ) const;
	uint32_t	TextureCount() const;
	bool		AddShader( const drawPass_t pass, const hdl_t hdl );
	hdl_t		GetShader( const drawPass_t pass ) const;
	uint32_t	ShaderCount() const;

	void Serialize( Serializer* serializer );
};

float GGX( float NoH, float roughness );
float SmithGGXCorrelated( float NoV, float NoL, float roughness );
vec3f Schlick( float u, vec3f f0 );
float Lambert();
vec3f BrdfGGX( const vec3f& n, const vec3f& v, const vec3f& l, const Material& m );

class AssetManager;

class BakedMaterialLoader : public LoadHandler<Material>
{
private:
	std::string		m_assetDir;
	std::string		m_textureDir;
	std::string		m_bakedDir;
	std::string		m_fileName;
	std::string		m_ext;
	AssetManager*	m_assets;

	bool Load( Asset<Material>& texture );

public:
	BakedMaterialLoader() {}
	BakedMaterialLoader( AssetManager* assets, const std::string& path, const std::string& ext )
	{
		SetAssetRef( assets );
		SetAssetPath( path );
		SetExtName( ext );
	}

	void SetAssetPath( const std::string& path );
	void SetExtName( const std::string& file );
	void SetAssetRef( AssetManager* assetsPtr );
};

using pMatLoader_t = Asset<Material>::loadHandlerPtr_t;