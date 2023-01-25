#include "scene.h"

#include "../primitives/ray.h"

void Scene::CreateEntityBounds( const hdl_t modelHdl, Entity& entity )
{
	const Model& model = gAssets.modelLib.Find( modelHdl )->Get();
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