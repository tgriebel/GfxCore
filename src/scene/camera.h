#pragma once

#include "../core/common.h"
#include "../core/util.h"
#include "../math/vector.h"
#include "../primitives/geom.h"

struct viewport_t
{
	float x;
	float y;
	float width;
	float height;
	float near;
	float far;
};

class Camera
{
private:
	static constexpr float MaxFov = Radians( 120.0f );
	static constexpr float MinFov = Radians( 30.0f );

	mat4x4f	axis;
	vec4f	origin;
	float	yaw;
	float	pitch;
	float	aspect;
	float	viewportWidth;
	float	viewportHeight;
	float	fov;
	float	halfFovX;
	float	halfFovY;

public:
	// FIXME: make private
	float	near;
	float	far;
	float	focalLength;

	void Init( const vec4f& _origin, const mat4x4f& _axis, const float _aspect = 1.0f, const float _fov = 90.0f, const float _near = 1.0f, const float _far = 1000.0f )
	{
		float aspectRatio = ( _aspect != 0.0f ) ? _aspect : 1.0f;

		origin = _origin;
		axis = _axis;

		fov = Radians( _fov );
		near = _near;
		far = _far;
		yaw = 0.0f;
		pitch = 0.0f;
		focalLength = _far;

		SetAspectRatio( aspectRatio );
	}

	Camera()
	{
		vec4f origin = vec4f( 0.0f, 0.0f, 0.0f, 0.0f );
		float axisValues[ 16 ] = { 0.0f, -1.0f, 0.0f, 0.0f,
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

	void SetAspectRatio( const float aspectRatio )
	{
		aspect = aspectRatio;
		halfFovX = tan( 0.5f * fov );
		halfFovY = tan( 0.5f * fov ) / aspectRatio;
		viewportWidth = 2.0f * halfFovX;
		viewportHeight = 2.0f * halfFovY;
	}

	float GetAspectRatio() const
	{
		return aspect;
	}

	void SetFov( const float fieldOfView )
	{
		fov = Clamp( fieldOfView, MinFov, MaxFov );
		halfFovX = tan( 0.5f * fov );
		halfFovY = tan( 0.5f * fov ) / aspect;
		viewportWidth = 2.0f * halfFovX;
		viewportHeight = 2.0f * halfFovY;
	}

	float GetFov() const
	{
		return fov;
	}

	struct plane_t
	{
		vec4f halfWidth;
		vec4f halfHeight;
		vec4f origin;
	};

	plane_t GetFocalPlane() const
	{
		plane_t plane;
		plane.origin = origin + focalLength * GetForward();
		plane.halfWidth = 0.5f * focalLength * viewportWidth * GetRight();
		plane.halfHeight = 0.5f * focalLength * viewportHeight * GetUp();
		return plane;
	}

	Ray GetViewRay( const vec2f& uv ) const
	{	// TODO: clip by near plane
		plane_t plane = GetFocalPlane();
		vec4f corner = plane.origin - plane.halfWidth - plane.halfHeight;
		vec4f viewPt = corner + vec4f( 2.0f * uv[ 0 ] * plane.halfWidth + 2.0f * uv[ 1 ] * plane.halfHeight );

		vec3f rayOrigin = vec3f( origin[ 0 ], origin[ 1 ], origin[ 2 ] );
		vec3f rayTarget = vec3f( viewPt[ 0 ], viewPt[ 1 ], viewPt[ 2 ] );
		return Ray( rayOrigin, rayTarget );
	}

	vec4f GetOrigin() const
	{
		return origin;
	}

	mat4x4f GetAxis() const
	{
		mat4x4f view = ( ComputeRotationY( Degrees( -yaw ) ) * axis ); // TODO: check if rotation mat is right direction. Just flipped the sign for now
		view = ( ComputeRotationX( Degrees( -pitch ) ) * view );
		return view;
	}

	mat4x4f GetViewMatrix() const
	{
		mat4x4f view = GetAxis();
		vec4f X = vec4f( view[ 0 ][ 0 ], view[ 0 ][ 1 ], view[ 0 ][ 2 ], view[ 0 ][ 3 ] );
		vec4f Y = vec4f( view[ 1 ][ 0 ], view[ 1 ][ 1 ], view[ 1 ][ 2 ], view[ 1 ][ 3 ] );
		vec4f Z = vec4f( view[ 2 ][ 0 ], view[ 2 ][ 1 ], view[ 2 ][ 2 ], view[ 2 ][ 3 ] );
		vec4f localOrigin = vec4f( -Dot( X, origin ), -Dot( Y, origin ), -Dot( Z, origin ), 0.0f );

		// Column-major
		float values[ 16 ] = { view[ 0 ][ 0 ], view[ 1 ][ 0 ], view[ 2 ][ 0 ], 0.0f,	// X
								view[ 0 ][ 1 ], view[ 1 ][ 1 ], view[ 2 ][ 1 ], 0.0f,	// Y
								view[ 0 ][ 2 ], view[ 1 ][ 2 ], view[ 2 ][ 2 ], 0.0f,	// Z
								localOrigin[ 0 ], localOrigin[ 1 ], localOrigin[ 2 ], 1.0f };
		return mat4x4f( values );
	}

	mat4x4f GetPerspectiveMatrix( const bool inverseZ = true ) const
	{
		mat4x4f proj = mat4x4f( 0.0f );
		proj[ 0 ][ 0 ] = 1.0f / halfFovX;
		proj[ 1 ][ 1 ] = 1.0f / halfFovY;
		proj[ 2 ][ 3 ] = -1.0f;

		if( inverseZ )
		{
			proj[ 2 ][ 2 ] = -near / ( far - near );
			proj[ 3 ][ 2 ] = ( far * near ) / ( far - near );
		}
		else
		{
			proj[ 2 ][ 2 ] = far / ( near - far );
			proj[ 3 ][ 2 ] = -( far * near ) / ( far - near );
		}
		return proj;
	}

	mat4x4f GetOrthogonalMatrix( const float left, const float right, const float top, const float bottom )
	{
		mat4x4f proj = mat4x4f( 1.0f );
		proj[ 0 ][ 0 ] = 2.0f / ( right - left );
		proj[ 1 ][ 1 ] = 2.0f / ( top - bottom );
		proj[ 2 ][ 2 ] = -2.0f / ( far - near );
		proj[ 3 ][ 3 ] = 1.0f;
		proj[ 3 ][ 0 ] = -( right + left ) / ( right - left );
		proj[ 3 ][ 1 ] = -( top + bottom ) / ( top - bottom );
		proj[ 3 ][ 2 ] = -( far + near ) / ( far - near );
		return proj;
	}

	vec4f GetForward() const
	{
		mat4x4f view = GetAxis();
		return vec4f( view[ 2 ][ 0 ], view[ 2 ][ 1 ], view[ 2 ][ 2 ], view[ 2 ][ 3 ] ).Reverse();
	}

	vec4f GetRight() const
	{
		mat4x4f view = GetAxis();
		return vec4f( view[ 0 ][ 0 ], view[ 0 ][ 1 ], view[ 0 ][ 2 ], view[ 0 ][ 3 ] );
	}

	vec4f GetUp() const
	{
		mat4x4f view = GetAxis();
		return vec4f( view[ 1 ][ 0 ], view[ 1 ][ 1 ], view[ 1 ][ 2 ], view[ 1 ][ 3 ] );
	}

	void Translate( vec4f offset )
	{
		origin += offset;
	}

	void AdjustYaw( const float delta )
	{
		yaw += delta;
	}

	void AdjustPitch( const float delta )
	{
		pitch += delta;
	}

	void MoveForward( const float delta )
	{
		origin += delta * GetForward();
	}

	void MoveRight( const float delta )
	{
		origin += delta * GetRight();
	}

	void MoveUp( const float delta )
	{
		origin += delta * GetUp();
	}
};