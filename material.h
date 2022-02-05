#pragma once

#include <cinttypes>
#include "common.h"
#include "mathVector.h"
#include "matrix.h"
#include "color.h"

struct lightInput_t
{
	vec3d	viewVector;
	vec3d	normal;
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
	double		Tr;
	double		Ns;
	double		Ni;
	double		d;
	double		illum;

	bool		textured;
	int32_t		colorMapId;
	int32_t		normalMapId;
};


// BRDF functions are adapted from "https://google.github.io/filament/Filament.md.html#overview/physicallybasedrendering"
static inline float GGX( double NoH, double roughness )
{
	double a = NoH * roughness;
	double k = roughness / ( 1.0 - NoH * NoH + a * a );
	return k * k * ( 1.0 / PI );
}


static inline float SmithGGXCorrelated( double NoV, double NoL, double roughness )
{
	double a2 = roughness * roughness;
	double GGXV = NoL * sqrt( NoV * NoV * ( 1.0 - a2 ) + a2 );
	double GGXL = NoV * sqrt( NoL * NoL * ( 1.0 - a2 ) + a2 );
	return 0.5 / ( GGXV + GGXL );
}


static inline vec3d Schlick( double u, vec3d f0 ) {
	return f0 + ( vec3d( 1.0 ) - f0 ) * pow( 1.0 - u, 5.0 );
}


static inline double Lambert()
{
	return 1.0 / PI;
}


static inline vec3d BrdfGGX( const vec3d& n, const vec3d& v, const vec3d& l, const material_t& m )
{
	double perceptualRoughness = 1.0;
	double f0 = 0.1;

	vec3d h = ( v + l ).Normalize();

	double NoV = abs( Dot( n, v ) ) + 1e-5;
	double NoL = Clamp( Dot( n, l ), 0.0, 1.0 );
	double NoH = Clamp( Dot( n, h ), 0.0, 1.0 );
	double LoH = Clamp( Dot( l, h ), 0.0, 1.0 );

	// perceptually linear roughness to roughness (see parameterization)
	double roughness = perceptualRoughness * perceptualRoughness;

	double D = GGX( NoH, roughness );
	vec3d  F = Schlick( LoH, f0 );
	double V = SmithGGXCorrelated( NoV, NoL, roughness );

	// specular BRDF
	vec3d Fr = ( D * V ) * F;

	// diffuse BRDF
	vec3d Fd = vec3d( 1.0f, 0.0f, 0.0f ) * Lambert();

	return ( Fr + Fd ) * std::max( 0.0, Dot( n, l ) ); // TODO:
}