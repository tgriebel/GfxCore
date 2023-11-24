/*
* MIT License
*
* Copyright( c ) 2022-2023 Thomas Griebel
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

#include <vector>
#include <chrono>

#include "camera.h"
#include "../core/common.h"
#include "../core/assetLib.h"
#include "../math/vector.h"
#include "../image/color.h"
#include "../asset_types/texture.h"
#include "../asset_types/material.h"
#include "../asset_types/gpuProgram.h"
#include "../asset_types/model.h"
#include "entity.h"
#include "assetManager.h"


enum lightFlags_t
{
	LIGHT_FLAGS_NONE = 0,
	LIGHT_FLAGS_HIDDEN = ( 1 << 0 ),
	LIGHT_FLAGS_SHADOW = ( 1 << 1 ),
	LIGHT_FLAGS_ALL = 0XFF,
};


struct light_t
{
	vec4f			pos;
	vec4f			dir;
	Color			color;
	float			intensity;
	lightFlags_t	flags;
};

struct Ray;

extern AssetManager g_assets;

class Scene
{
private:
	using chronoClock_t = std::chrono::high_resolution_clock;
	chronoClock_t::time_point	prevTime;
	std::chrono::nanoseconds	dt;
	std::chrono::nanoseconds	totalTime;
	uint64_t					frameNumber;
public:
	Camera*						mainCamera;
	Camera						cameras[ 7 ];
	std::vector<Entity*>		entities;
	std::vector<light_t>		lights;
	float						defaultNear = 0.1f;
	float						defaultFar = 1000.0f;
	Entity*						selectedEntity = nullptr;

	virtual void Update() {}
	virtual void Init() {}
	virtual void Shutdown() {}

	inline void AdvanceFrame()
	{
		const chronoClock_t::time_point currentTime = chronoClock_t::now();
		dt = ( currentTime - prevTime );
		prevTime = currentTime;

		totalTime += dt;

		++frameNumber;
	}

	inline float DeltaTime() const
	{
		return std::chrono::duration<float, std::chrono::seconds::period>( dt ).count();
	}

	inline std::chrono::nanoseconds DeltaNano() const
	{
		return dt;
	}

	inline float TotalTimeSeconds() const
	{
		return std::chrono::duration<float, std::chrono::seconds::period>( totalTime ).count();
	}

	inline uint64_t CurrentFrame() const
	{
		return frameNumber;
	}

	Scene()
	{
		for( uint32_t i = 0; i < 6; ++i )
		{
			cameras[ i ] = Camera( vec4f( 0.0f, 1.66f, 1.0f, 0.0f ) );
			cameras[ i ].SetClip( defaultNear, defaultFar );
			cameras[ i ].SetFov( Radians( 90.0f ) );
			cameras[ i ].SetAspectRatio( 1.0f );
		}
		mainCamera = &cameras[ 0 ];

		prevTime = chronoClock_t::now();
		dt = std::chrono::nanoseconds( 0 );
		totalTime = std::chrono::nanoseconds( 0 );
	}

	void			CreateEntityBounds( const hdl_t modelHdl, Entity& entity );
	Entity*			GetTracedEntity( const Ray& ray );

	uint32_t		EntityCount() const;
	Entity*			FindEntity( const uint32_t entityIx );
	const Entity*	FindEntity( const uint32_t entityIx ) const ;
	Entity*			FindEntity( const char* name );
	const Entity*	FindEntity( const char* name ) const;
};