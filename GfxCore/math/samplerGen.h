#pragma once

#include "../core/common.h"
#include "../core/util.h"
#include "vector.h"
#include <random>


class SamplerGenerator
{
protected:

	float ScaleFloat( const float t ) const
	{
		return m_min + t * ( m_max - m_min );
	}

	float m_min   = 0.0f;
	float m_max   = 1.0f;
	float m_range = 1.0f;

public:

    virtual ~SamplerGenerator() = default;

    [[nodiscard]]
    virtual float Sample() = 0;

    [[nodiscard]]
    virtual vec2f Sample2D() = 0;

    [[nodiscard]]
    virtual vec3f Sample3D() = 0;

    void AddRange( const float min, const float max )
    {
        m_min   = min;
        m_max   = max;
        m_range = ( max - min );
    }
};


// Returns uniform random samples (defaults within [0, 1])
class UniformSamplerGen : public SamplerGenerator
{
private:

	std::mt19937 m_rng { std::random_device{}( ) };
	std::uniform_real_distribution<float> m_dist { 0.0f, 1.0f };

public:

    [[nodiscard]]
    float Sample() override
    {
        return ScaleFloat( m_dist( m_rng ) );
    }

    [[nodiscard]]
    vec2f Sample2D() override
    {
        return vec2f( ScaleFloat( m_dist( m_rng ) ), ScaleFloat( m_dist( m_rng ) ) );
    }

    [[nodiscard]]
    vec3f Sample3D() override
    {
        return vec3f( ScaleFloat( m_dist( m_rng ) ), ScaleFloat( m_dist( m_rng ) ), ScaleFloat( m_dist( m_rng ) ) );
    }
};


// Samples uniformly distributed points on a circle (Sample2D) or sphere surface (Sample3D).
class SphericalSamplerGen : public SamplerGenerator
{
private:

	float Sample() override { return 0.0f; }

	std::mt19937 m_rng { std::random_device{}( ) };
	std::uniform_real_distribution<float> m_dist { 0.0f, 1.0f };
	float m_radius;

public:

    SphericalSamplerGen( const float radius = 1.0f ) : m_radius( radius ) {}

    void SetRadius( const float radius )
    {
        m_radius = radius;
    }

    [[nodiscard]]
    vec2f Sample2D() override
    {
        return PointOnCircle( m_dist( m_rng ), m_radius );
    }

    [[nodiscard]]
    vec3f Sample3D() override
    {
        return PointOnSphere( m_dist( m_rng ), m_dist( m_rng ), m_radius );
    }
};


// Shirley concentric disk mapping
// https://victorpoughon.fr/non-random-uniform-disk-sampling/
// https://www.adriancourreges.com/blog/2018/12/02/ue4-optimized-post-effects/
// Warping: https://www.slideshare.net/slideshow/graphics-gems-from-cryengine-3-siggraph-2013/25052583#33
class ShirleyConcentricSamplerGen : public SamplerGenerator
{
private:

	static float NgonBoundaryRadius( const float theta, const int N )
	{
		const float piOverN = PI / static_cast< float >( N );
		const float twoPiOverN = 2.0f * piOverN;
		const float t = theta - twoPiOverN * std::floor( theta / twoPiOverN ) - piOverN;
		return std::cos( piOverN ) / std::cos( t );
	}

	float Sample() override { return 0.0f; }
	vec3f Sample3D() override { return vec3f( 0.0f, 0.0f, 0.0f ); }

	int32_t	m_sides;
	float	m_angle;
	int32_t	m_index = 0;

public:

    ShirleyConcentricSamplerGen( const int sides = 0, const float angle = 0.0f )
        : m_sides( sides )
        , m_angle( angle )
    {}

    void SetBlades( const int sides ) { m_sides = sides; }
    void SetApertureAngle( const float angle ) { m_angle = angle; }

    [[nodiscard]]
    vec2f Sample2D() override
    {
        const int gridN = Max( static_cast<int>( m_range ), 1 );

        const int i = m_index % gridN;
        const int j = m_index / gridN;
        m_index = ( m_index + 1 ) % ( gridN * gridN );

        const float step = ( gridN > 1 ) ? 1.0f / static_cast<float>( gridN - 1 ) : 0.0f;
        const float a = 2.0f * ( i * step ) - 1.0f;
        const float b = 2.0f * ( j * step ) - 1.0f;

        float r = 0.0f;
        float phi = 0.0f;

        if ( a != 0.0f || b != 0.0f )
        {
            if ( std::abs( a ) > std::abs( b ) )
            {
                r   = a;
                phi = ( PI / 4.0f ) * ( b / a );
            }
            else
            {
                r   = b;
                phi = ( PI / 2.0f ) - ( PI / 4.0f ) * ( a / b );
            }
        }

        float x = r * std::cos( phi );
        float y = r * std::sin( phi );

        if ( m_sides >= 3 )
        {
            const float theta = std::atan2( y, x ) + m_angle;
            r = std::sqrt( x * x + y * y ) * NgonBoundaryRadius( theta, m_sides );
            x = r * std::cos( theta );
            y = r * std::sin( theta );
        }

        return vec2f( x, y );
    }
};


// Deterministic Halton Sequence
class HaltonSamplerGen : public SamplerGenerator
{
private:

	// https ://en.wikipedia.org/wiki/Halton_sequence
	float Halton( const int index, const int base ) const
	{
		float result = 0.0f;
		float f = 1.0f;
		int32_t i = index;
		while( i > 0 ) {
			f /= static_cast< float >( base );
			result += f * static_cast< float >( i % base );
			i /= base;
		}
		return result;
	}

	int32_t m_index = 1;

public:

    [[nodiscard]]
    float Sample() override
    {
		// Use prime number as base
        return ScaleFloat( Halton( m_index++, 2 ) );
    }

    [[nodiscard]]
    vec2f Sample2D() override
    {
		// Use first two prime numbers as base
        const vec2f result( ScaleFloat( Halton( m_index, 2 ) ), ScaleFloat( Halton( m_index, 3 ) ) );
        ++m_index;
        return result;
    }

    [[nodiscard]]
    vec3f Sample3D() override
    {
		// Use first three prime numbers as base
        const vec3f result( ScaleFloat( Halton( m_index, 2 ) ), ScaleFloat( Halton( m_index, 3 ) ), ScaleFloat( Halton( m_index, 5 ) ) );
        ++m_index;
        return result;
    }
};

