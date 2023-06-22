#include "assetBaker.h"

#include <chrono>
#include <ctime>
#include <fstream>

#include <SysCore/serializer.h>
#include <SysCore/systemUtils.h>

#include "../asset_types/model.h"
#include "../asset_types/material.h"
#include "../asset_types/texture.h"
#include "../asset_types/gpuProgram.h"
#include "../core/assetLib.h"
#include "../io/serializeClasses.h"

std::vector<bakedAssetInfo_t> assetInfo;
Serializer* s;

template<class T>
static void BakeLibraryAssets( AssetLib<T>& lib, const std::string& path, const std::string& ext )
{
	assert( s->GetMode() == serializeMode_t::STORE );

	auto time = std::chrono::system_clock::now();
	std::time_t date = std::chrono::system_clock::to_time_t( time );
	char dateCStr[ 128 ];
	ctime_s( dateCStr, 128, &date );

	const uint32_t count = lib.Count();
	for ( uint32_t i = 0; i < count; ++i )
	{
		Asset<T>* asset = lib.Find( i );

		bakedAssetInfo_t info = {};
		info.name = asset->GetName();
		info.hash = asset->Handle().String();
		info.type = lib.AssetTypeName();
		info.date = std::string( dateCStr );
		info.sizeBytes = s->CurrentSize();

		s->Clear( false );
		s->NextString( info.name );
		s->NextString( info.type );
		s->NextString( info.date );
		asset->Get().Serialize( s );
		s->WriteFile( path + asset->Handle().String() + ext );

		assetInfo.push_back( info );
	}
}


void AssetBaker::AddAssetLib( AssetLib<Model>* lib, const std::string path, const std::string ext )
{
	m_modelLib = lib;
	m_modelPath = path;
	m_modelExt = ext;
}


void AssetBaker::AddAssetLib( AssetLib<Material>* lib, const std::string path, const std::string ext )
{
	m_materialLib = lib;
	m_materialPath = path;
	m_materialExt = ext;
}


void AssetBaker::AddAssetLib( AssetLib<Image>* lib, const std::string path, const std::string ext )
{
	m_imageLib = lib;
	m_imagePath = path;
	m_imageExt = ext;
}


void AssetBaker::AddBakeDirectory( const std::string path )
{
	m_bakePath = path;
}


void AssetBaker::Bake()
{
	s = new Serializer( MB( 128 ), serializeMode_t::STORE );
	assetInfo.reserve( m_imageLib->Count() + m_materialLib->Count() + m_modelLib->Count() );

	MakeDirectory( m_bakePath );
	MakeDirectory( m_bakePath + m_imagePath );
	MakeDirectory( m_bakePath + m_materialPath );
	MakeDirectory( m_bakePath + m_modelPath );

	BakeLibraryAssets( *m_imageLib, m_bakePath + m_imagePath, m_imageExt );
	BakeLibraryAssets( *m_materialLib, m_bakePath + m_materialPath, m_materialExt );
	BakeLibraryAssets( *m_modelLib, m_bakePath + m_modelPath, m_modelExt );

	std::ofstream assetFile( m_bakePath + "asset_info.csv", std::ios::out | std::ios::trunc );
	assetFile << "Name,Type,Hash,Data,Size\n";
	for ( auto it = assetInfo.begin(); it != assetInfo.end(); ++it )
	{
		const bakedAssetInfo_t& asset = *it;
		assetFile << asset.name << "," << asset.type << "," << asset.hash << "," << asset.sizeBytes << "," << asset.date; // date has an end-line char
	}
	assetFile.close();

	delete s;
}