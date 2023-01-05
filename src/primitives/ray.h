#pragma once

#include <utility>
#include "../math/vector.h"
#include "../math/matrix.h"
#include "../core/common.h"

struct Ray
{
	Ray() : mint( 1e7 ), maxt( FLT_MAX ) {}

	Ray( const vec3f& origin, const vec3f& target, const float _minT = 1e-7 )
	{
		o = origin;
		d = target - origin;
		t = d.Length();
		d = d.Normalize();

		mint = std::max( 0.0f, _minT );
		maxt = std::max( 0.0f, t - mint );
	}

	bool Inside( const float _t ) const
	{
		return ( _t >= mint ) && ( _t <= maxt );
	}

	vec3f GetPoint( const float _t ) const
	{
		float t = Clamp( _t, mint, maxt );
		return ( o + t * d );
	}

	vec3f GetOrigin() const
	{
		return o;
	}

	vec3f GetEndPoint() const
	{
		return ( o + d );
	}

	vec3f GetVector() const
	{
		return d.Normalize();
	}

	vec3f d;
	vec3f o;
	// private:
	float t;
	float mint;
	float maxt;
};
