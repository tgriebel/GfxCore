#include "camera.h"


float Camera::GetAspectRatio() const
{
	return aspect;
}


void Camera::SetFov( const float fieldOfView, const float aspectRatio )
{
	fov = Clamp( fieldOfView, MinFov, MaxFov );
	aspect = aspectRatio;

	halfFovX = tan( 0.5f * fov );
	halfFovY = tan( 0.5f * fov ) / aspectRatio;
	viewportWidth = 2.0f * halfFovX;
	viewportHeight = 2.0f * halfFovY;
}


void Camera::SetClip( const float nearDistance, const float farDistance )
{
	SetNearClip( nearDistance );
	SetFarClip( farDistance );
}


void Camera::SetNearClip( const float nearDistance )
{
	clipRegion.near = nearDistance;
}


void Camera::SetFarClip( const float farDistance )
{
	clipRegion.far = farDistance;
	focalLength = farDistance;
}


float Camera::GetFov() const
{
	return fov;
}


float Camera::GetNearClip() const
{
	return clipRegion.near;
}


float Camera::GetFarClip() const
{
	return clipRegion.far;
}


Camera::plane_t Camera::GetFocalPlane() const
{
	Camera::plane_t plane;
	plane.origin = origin + focalLength * GetForward();
	plane.halfWidth = 0.5f * focalLength * viewportWidth * GetRight();
	plane.halfHeight = 0.5f * focalLength * viewportHeight * GetUp();
	return plane;
}


Ray Camera::GetViewRay( const vec2f& uv ) const
{	// TODO: clip by near plane
	plane_t plane = GetFocalPlane();
	vec4f corner = plane.origin - plane.halfWidth - plane.halfHeight;
	vec4f viewPt = corner + vec4f( 2.0f * uv[ 0 ] * plane.halfWidth + 2.0f * uv[ 1 ] * plane.halfHeight );

	vec3f rayOrigin = vec3f( origin[ 0 ], origin[ 1 ], origin[ 2 ] );
	vec3f rayTarget = vec3f( viewPt[ 0 ], viewPt[ 1 ], viewPt[ 2 ] );
	return Ray( rayOrigin, rayTarget );
}


vec4f Camera::GetOrigin() const
{
	return origin;
}


mat4x4f Camera::GetAxis() const
{
	if( isAxisCacheValid ) {
		return cachedViewAxis;
	}

	cachedViewAxis = ( ComputeRotationY( Degrees( -yaw ) ) * axis ); // TODO: check if rotation mat is right direction. Just flipped the sign for now
	cachedViewAxis = ( ComputeRotationX( Degrees( -pitch ) ) * cachedViewAxis );
	cachedViewAxis = ( ComputeRotationZ( Degrees( -roll ) ) * cachedViewAxis );

	isAxisCacheValid = forceFlushAxisCache ? false : true;

	return cachedViewAxis;
}


mat4x4f Camera::GetViewMatrix() const
{
	const mat4x4f& view = GetAxis();
	const vec4f& X = view[ 0 ];
	const vec4f& Y = view[ 1 ];
	const vec4f& Z = view[ 2 ];
	const vec4f& localOrigin = vec4f( -Dot( X, origin ), -Dot( Y, origin ), -Dot( Z, origin ), 0.0f );

	// Column-major
	float values[ 16 ] = { view[ 0 ][ 0 ], view[ 1 ][ 0 ], view[ 2 ][ 0 ], 0.0f,	// X
							view[ 0 ][ 1 ], view[ 1 ][ 1 ], view[ 2 ][ 1 ], 0.0f,	// Y
							view[ 0 ][ 2 ], view[ 1 ][ 2 ], view[ 2 ][ 2 ], 0.0f,	// Z
							localOrigin[ 0 ], localOrigin[ 1 ], localOrigin[ 2 ], 1.0f };
	return mat4x4f( values );
}


mat4x4f Camera::GetPerspectiveMatrix( const bool inverseZ ) const
{
	mat4x4f proj = mat4x4f( 0.0f );
	proj[ 0 ][ 0 ] = 1.0f / halfFovX;
	proj[ 1 ][ 1 ] = 1.0f / halfFovY;
	proj[ 2 ][ 3 ] = -1.0f;

	const float n = clipRegion.near;
	const float f = clipRegion.far;

	if ( inverseZ )
	{
		proj[ 2 ][ 2 ] = -n / ( f - n );
		proj[ 3 ][ 2 ] = ( f * n ) / ( f - n );
	}
	else
	{
		proj[ 2 ][ 2 ] = f / ( n - f );
		proj[ 3 ][ 2 ] = -( f * n ) / ( f - n );
	}
	return proj;
}


mat4x4f Camera::GetOrthographicMatrix( const float left, const float right, const float top, const float bottom ) const
{
	const float n = clipRegion.near;
	const float f = clipRegion.far;

	mat4x4f proj = mat4x4f( 1.0f );
	proj[ 0 ][ 0 ] = 2.0f / ( right - left );
	proj[ 1 ][ 1 ] = 2.0f / ( top - bottom );
	proj[ 2 ][ 2 ] = -2.0f / ( f - n );
	proj[ 3 ][ 3 ] = 1.0f;
	proj[ 3 ][ 0 ] = -( right + left ) / ( right - left );
	proj[ 3 ][ 1 ] = -( top + bottom ) / ( top - bottom );
	proj[ 3 ][ 2 ] = -( f + n ) / ( f - n );
	return proj;
}


vec4f Camera::GetForward() const
{
	const mat4x4f& view = GetAxis();
	return view[ 2 ].Reverse();
}


vec4f Camera::GetRight() const
{
	const mat4x4f& view = GetAxis();
	return view[ 0 ];
}


vec4f Camera::GetUp() const
{
	const mat4x4f& view = GetAxis();
	return view[ 1 ];
}


void Camera::GetAxisVectors( vec4f& forward, vec4f& right, vec4f& up ) const
{
	const mat4x4f& view = GetAxis();

	forward = view[ 2 ].Reverse();
	right = view[ 0 ];
	up = view[ 1 ];
}


void Camera::SetAngles( const vec3f& angles )
{
	yaw = angles.x;
	pitch = angles.y;
	roll = angles.z;

	isAxisCacheValid = false;
}


void Camera::SetPosition( const vec3f& position )
{
	origin = vec4f( position, 0.0f );
}


void Camera::Translate( vec4f offset )
{
	origin += offset;
}


void Camera::Pan( const float delta )
{
	yaw += delta;

	isAxisCacheValid = false;
}


void Camera::Tilt( const float delta )
{
	pitch += delta;

	isAxisCacheValid = false;
}


void Camera::Roll( const float delta )
{
	roll += delta;

	isAxisCacheValid = false;
}


void Camera::Dolly( const float delta )
{
	origin += delta * GetForward();
}


void Camera::Truck( const float delta )
{
	origin += delta * GetRight();
}


void Camera::Pedestal( const float delta )
{
	origin += delta * GetUp();
}