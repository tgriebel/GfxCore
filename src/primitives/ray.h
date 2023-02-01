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
		return ( _t >= mint ) && ( _t <= t );
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
		return GetPoint( t );
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
