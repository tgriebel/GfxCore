#pragma once

#include "common.h"
#include "mathVector.h"
#include "geom.h"

class Camera
{
public:
	Camera() {}

	Camera( const vec4f& _origin, const vec4f& _X, const vec4f& _Y, const vec4f& _Z, const float _fovDegrees, const float _aspect, const float _near, const float _far )
	{
		float aspectRatio = ( _aspect != 0.0 ) ? _aspect : 1.0;

		halfFovX = tan( 0.5 * Radians( _fovDegrees ) );
		halfFovY = tan( 0.5 * Radians( _fovDegrees ) ) / aspectRatio;
		near = _near;
		far = _far;
		aspect = aspectRatio;
		focalLength = far;
		viewportWidth = 2.0 * halfFovX;
		viewportHeight = 2.0 * halfFovY;
		origin = _origin;
		xAxis = _X;
		yAxis = _Y;
		zAxis = _Z;
	}

	plane_t GetFocalPlane() const
	{
		plane_t plane;
		plane.origin = origin - focalLength * zAxis;
		plane.halfWidth = 0.5f * focalLength * viewportWidth * xAxis;
		plane.halfHeight = 0.5f * focalLength * viewportHeight * yAxis;
		return plane;
	}

	Ray GetViewRay( const vec2f& uv ) const
	{
		// TODO: clip by near plane
		plane_t plane = GetFocalPlane();
		vec4f corner = plane.origin - plane.halfWidth - plane.halfHeight;
		vec4f viewPt = corner + vec4f( 2.0f * uv[ 0 ] * plane.halfWidth + 2.0f * uv[ 1 ] * plane.halfHeight );

		Ray ray = Ray( Trunc<4, 1>( origin ), Trunc<4, 1>( viewPt ), 0.0 );

		return ray;
	}

	mat4x4f ToViewMatrix() const
	{
		return CreateMatrix4x4(	xAxis[ 0 ],	xAxis[ 1 ],	xAxis[ 2 ],	-Dot( xAxis, origin ),
								yAxis[ 0 ],	yAxis[ 1 ],	yAxis[ 2 ],	-Dot( yAxis, origin ),
								zAxis[ 0 ],	zAxis[ 1 ],	zAxis[ 2 ],	-Dot( zAxis, origin ),
								0.0f,		0.0f,		0.0f,		1.0f );
	}

	mat4x4f ToOrthographicProjMatrix( float w, float h ) const
	{
		const float width = ( w != 0.0f ) ? w : 1.0f;
		const float height = ( h != 0.0f ) ? h : 1.0f;

		mat4x4f m;
		m[ 0 ][ 0 ] = 2.0f / width;
		m[ 0 ][ 3 ] = -1.0f;
		m[ 1 ][ 1 ] = 2.0f / height;
		m[ 1 ][ 3 ] = -1.0f;
		m[ 2 ][ 2 ] = -2.0f / ( far - near );
		m[ 2 ][ 3 ] = -( far + near ) / ( far - near );
		m[ 3 ][ 3 ] = 1.0f;

		return m;
	}

	mat4x4f ToPerspectiveProjMatrix() const
	{
		mat4x4f m;
		m[ 0 ][ 0 ] = 1.0f / halfFovX;
		m[ 1 ][ 1 ] = 1.0f / halfFovY;
		m[ 2 ][ 2 ] = -far / ( far - near );
		m[ 2 ][ 3 ] = -( far * near ) / ( far - near );
		m[ 3 ][ 2 ] = -1.0f;
		return m;
	}

	float aspect;
	float halfFovX;
	float halfFovY;
	float focalLength;
	float near;
	float far;
	float viewportWidth;
	float viewportHeight;

	vec4f localOrigin;
	vec4f origin;
	vec4f xAxis;
	vec4f yAxis;
	vec4f zAxis;
};
