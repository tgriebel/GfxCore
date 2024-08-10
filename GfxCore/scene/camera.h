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

#include "../core/common.h"
#include "../core/util.h"
#include "../math/vector.h"
#include "../primitives/geom.h"

struct viewport_t
{
	int32_t		x;
	int32_t		y;
	uint32_t	width;
	uint32_t	height;
	float		near;
	float		far;

	viewport_t() :
		x( 0 ),
		y( 0 ),
		width( 1 ),
		height( 1 ),
		near( 0 ),
		far( 1 )
	{}

	viewport_t( const int32_t x_, const int32_t y_, const uint32_t width_, const uint32_t height_, const float near_, const float far_ ) :
		x( x_ ),
		y( y_ ),
		width( width_ ),
		height( height_ ),
		near( near_ ),
		far( far_ )
	{}
};


class Camera
{
private:
	struct plane_t
	{
		vec4f halfWidth;
		vec4f halfHeight;
		vec4f origin;
	};

	static constexpr float MaxFov = Radians( 120.0f );
	static constexpr float MinFov = Radians( 30.0f );

	mat4x4f		axis;
	vec4f		origin;
	float		yaw;
	float		pitch;
	float		roll;
	float		aspect;
	viewport_t	clipRegion;
	float		viewportWidth;
	float		viewportHeight;
	float		fov;
	float		halfFovX;
	float		halfFovY;
	float		focalLength;

	plane_t		GetFocalPlane() const;

public:
	void Init( const vec4f& _origin, const mat4x4f& _axis, const float _aspect = 1.0f, const float _fov = 90.0f, const float _near = 1.0f, const float _far = 1000.0f )
	{
		float aspectRatio = ( _aspect != 0.0f ) ? _aspect : 1.0f;

		origin = _origin;
		axis = _axis;

		fov = Radians( _fov );
		clipRegion.near = _near;
		clipRegion.far = _far;
		yaw = 0.0f;
		pitch = 0.0f;
		roll = 0.0f;
		focalLength = _far;

		SetAspectRatio( aspectRatio );
	}

	Camera()
	{
		vec4f origin = vec4f( 0.0f, 0.0f, 0.0f, 0.0f );
		float axisValues[ 16 ] = {	0.0f, -1.0f, 0.0f, 0.0f,
									0.0f, 0.0f, -1.0f, 0.0f,
									-1.0f, 0.0f, 0.0f, 0.0f,
									0.0f, 0.0f, 0.0f, 1.0f };
		Init( origin, mat4x4f( axisValues ) );
	}

	Camera( const vec4f& _origin )
	{
		float axisValues[ 16 ] = { 0.0f, -1.0f, 0.0f, 0.0f,
									0.0f, 0.0f, -1.0f, 0.0f,
									-1.0f, 0.0f, 0.0f, 0.0f,
									0.0f, 0.0f, 0.0f, 1.0f };
		Init( _origin, mat4x4f( axisValues ) );
	}

	Camera( const vec4f& _origin, const mat4x4f& _axis )
	{
		Init( _origin, _axis );
	}

	// Viewport
	void		SetAspectRatio( const float aspectRatio );	
	void		SetFov( const float fieldOfView );
	void		SetClip( const float nearDistance, const float farDistance );
	void		SetNearClip( const float nearDistance );
	void		SetFarClip( const float farDistance );
	float		GetFov() const;
	float		GetNearClip() const;
	float		GetFarClip() const;
	float		GetAspectRatio() const;
	Ray			GetViewRay( const vec2f& uv ) const;
	
	// Matrix/vector conversion
	vec4f		GetOrigin() const;
	vec4f		GetForward() const;
	vec4f		GetRight() const;
	vec4f		GetUp() const;
	mat4x4f		GetAxis() const;
	mat4x4f		GetViewMatrix() const;
	mat4x4f		GetPerspectiveMatrix( const bool inverseZ = true ) const;
	mat4x4f		GetOrthogonalMatrix( const float left, const float right, const float top, const float bottom );
	
	// Movement
	void		SetPosition( const vec3f& angles );
	void		SetAngles( const vec3f& position );
	void		Translate( vec4f offset );
	void		Pan( const float delta );
	void		Tilt( const float delta );
	void		Roll( const float delta );
	void		Dolly( const float delta );
	void		Truck( const float delta );
	void		Pedestal( const float delta );
};