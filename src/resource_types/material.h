#pragma once

#include <cinttypes>
#include "../core/common.h"
#include "../math/vector.h"
#include "../math/matrix.h"
#include "../image/color.h"

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
	DRAWPASS_DEBUG_SOLID,
	DRAWPASS_DEBUG_WIREFRAME,
	DRAWPASS_POST_2D,
	DRAWPASS_COUNT
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
	MATERIAL_USAGE_GGX,
	MATERIAL_USAGE_HEIGHT_MAP,
	MATERIAL_USAGE_CUBE,
};

// FIXME: Deprecated
struct material_t
{
	static const uint32_t BufferSize = 256;
	char		name[ BufferSize ];

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

	bool		textured;
	int32_t		colorMapId;
	int32_t		normalMapId;
};

class Material
{
public:
	static const uint32_t MaxMaterialTextures = 8;
	static const uint32_t MaxMaterialShaders = DRAWPASS_COUNT;

	int32_t					uploadId;
	bool					dirty;
	materialUsage_t			usage;

	rgbTuplef_t				Ka;
	rgbTuplef_t				Ke;
	rgbTuplef_t				Kd;
	rgbTuplef_t				Ks;
	rgbTuplef_t				Tf;
	float					Tr;
	float					Ns;
	float					Ni;
	float					d;
	float					illum;

private:
	uint16_t				textureBitSet;
	uint16_t				shaderBitSet;

	hdl_t					textures[ MaxMaterialTextures ];
	hdl_t					shaders[ MaxMaterialShaders ];

public:
	Material() :
		Tr( 0.0f ),
		Ns( 0.0f ),
		Ni( 0.0f ),
		d( 1.0f ),
		illum( 0.0f ),
		textureBitSet( 0 ),
		shaderBitSet( 0 )
	{
		Kd = rgbTuplef_t( 1.0f, 1.0f, 1.0f );
		for ( int i = 0; i < MaxMaterialTextures; ++i ) {
			textures[ i ] = INVALID_HDL;
		}
		for ( int i = 0; i < MaxMaterialShaders; ++i ) {
			shaders[ i ] = INVALID_HDL;
		}
		dirty = true;
		uploadId = -1;
		usage = MATERIAL_USAGE_UNKNOWN;
	}

	inline bool IsTextured() const
	{
		return ( textureBitSet > 0 );
	}

	inline uint32_t TextureCount() const
	{
		uint32_t count = 0;
		uint32_t bits = textureBitSet;
		while( bits )
		{
			bits &= ( bits - 1 );
			count++;
		}
		return count;
	}

	inline bool AddTexture( const uint32_t slot, const hdl_t hdl )
	{
		if ( slot >= MaxMaterialTextures ) {
			return false;
		}
		textures[ slot ] = hdl;
		textureBitSet |= ( 1 << slot );
		return true;
	}

	inline hdl_t GetTexture( const uint32_t slot ) const
	{
		if ( slot >= MaxMaterialTextures ) {
			return INVALID_HDL;
		}
		return textures[ slot ];
	}

	inline bool AddShader( const uint32_t slot, const hdl_t hdl )
	{
		if ( slot >= MaxMaterialShaders ) {
			return false;
		}
		shaders[ slot ] = hdl;
		shaderBitSet |= ( 1 << slot );
		return true;
	}

	inline hdl_t GetShader( const uint32_t slot ) const
	{
		if ( slot >= MaxMaterialShaders ) {
			return INVALID_HDL;
		}
		return shaders[ slot ];
	}

	inline uint32_t ShaderCount() const
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
};

// BRDF functions are adapted from "https://google.github.io/filament/Filament.md.html#overview/physicallybasedrendering"
static inline float GGX( float NoH, float roughness )
{
	float a = NoH * roughness;
	float k = roughness / ( 1.0f - NoH * NoH + a * a );
	return k * k * ( 1.0f / PI );
}


static inline float SmithGGXCorrelated( float NoV, float NoL, float roughness )
{
	float a2 = roughness * roughness;
	float GGXV = NoL * sqrt( NoV * NoV * ( 1.0f - a2 ) + a2 );
	float GGXL = NoV * sqrt( NoL * NoL * ( 1.0f - a2 ) + a2 );
	return 0.5f / ( GGXV + GGXL );
}


static inline vec3f Schlick( float u, vec3f f0 ) {
	return f0 + ( vec3f( 1.0f ) - f0 ) * pow( 1.0f - u, 5.0f );
}


static inline float Lambert()
{
	return 1.0f / PI;
}


static inline vec3f BrdfGGX( const vec3f& n, const vec3f& v, const vec3f& l, const Material& m )
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