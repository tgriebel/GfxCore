#pragma once

#include <cinttypes>
#include "../core/common.h"
#include "../math/mathVector.h"
#include "../math/matrix.h"
#include "../image/color.h"

struct lightInput_t
{
	vec3f	viewVector;
	vec3f	normal;
};

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
	static const uint32_t MaxMaterialShaders = 16;

	hdl_t					textures[ MaxMaterialTextures ];
	hdl_t					shaders[ MaxMaterialShaders ];
	int32_t					uploadId;

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

	bool					textured;

	Material() :
		Tr( 0.0f ),
		Ns( 0.0f ),
		Ni( 0.0f ),
		d( 1.0f ),
		illum( 0.0f )
	{
		for ( int i = 0; i < MaxMaterialTextures; ++i ) {
			textures[ i ] = INVALID_HDL;
		}
		for ( int i = 0; i < MaxMaterialShaders; ++i ) {
			shaders[ i ] = INVALID_HDL;
		}
		uploadId = -1;
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


static inline vec3f BrdfGGX( const vec3f& n, const vec3f& v, const vec3f& l, const material_t& m )
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