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

#include "scene.h"

#include "../primitives/ray.h"

void Scene::CreateEntityBounds( const hdl_t modelHdl, Entity& entity )
{
	const Model& model = g_assets.modelLib.Find( modelHdl )->Get();
	entity.modelHdl = modelHdl.Get();
	entity.ExpandBounds( model.bounds );
}


Entity* Scene::GetTracedEntity( const Ray& ray )
{
	Entity* closestEnt = nullptr;
	float closestT = FLT_MAX;
	const uint32_t entityNum = static_cast<uint32_t>( entities.size() );
	for ( uint32_t i = 0; i < entityNum; ++i )
	{
		Entity* ent = entities[ i ];
		if ( !ent->HasFlag( ENT_FLAG_SELECTABLE ) ) {
			continue;
		}
		float t0, t1;
		if ( ent->GetBounds().Intersect( ray, t0, t1 ) ) {
			if ( t0 < closestT ) {
				closestT = t0;
				closestEnt = ent;
			}
		}
	}
	return closestEnt;
}


uint32_t Scene::EntityCount() const
{
	return static_cast<uint32_t>( entities.size() );
}


Entity* Scene::FindEntity( const uint32_t entityIx )
{
	return entities[ entityIx ];
}


const Entity* Scene::FindEntity( const uint32_t entityIx ) const
{
	return entities[ entityIx ];
}


Entity* Scene::FindEntity( const char* name )
{
	const uint32_t entCount = static_cast<uint32_t>( entities.size() );
	for ( uint32_t i = 0; i < entCount; ++i ) {
		if ( entities[ i ]->name == name ) {
			return entities[ i ];
		}
	}
	return nullptr;
}


const Entity* Scene::FindEntity( const char* name ) const
{
	const uint32_t entCount = static_cast<uint32_t>( entities.size() );
	for ( uint32_t i = 0; i < entCount; ++i ) {
		if ( entities[ i ]->name == name ) {
			return entities[ i ];
		}
	}
	return nullptr;
}