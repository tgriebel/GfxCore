#pragma once

#include <unordered_map>
#include <iterator>
#include <string>

template< class AssetType >
class LoadHandler
{
public:
	void Load( AssetType& asset );
};

template< class AssetType >
class Asset
{
protected:
	std::string				name;
	std::string				json;
	AssetType				asset;
	LoadHandler<AssetType>	loader;
	bool					loaded;
	bool					isDefault;

public:
	Asset() : name( "" ), loaded( false ) {}
	Asset( const AssetType& _asset, const std::string _name, const bool _loaded = true ) : 
		name( _name ), asset( _asset ), loaded( _loaded ), isDefault( false ) {}

	inline const std::string& GetName() const
	{
		return name;
	}

	inline bool IsLoaded() const
	{
		return loaded;
	}

	inline void SetLoaded()
	{
		loaded = true;
	}

	inline bool IsDefault() const
	{
		return isDefault;
	}

	inline void SetDefault()
	{
		isDefault = true;
	}

	inline const AssetType& Get() const
	{
		return asset;
	}

	inline AssetType& Get()
	{
		return asset;
	}

	inline LoadHandler<AssetType>& GetLoader()
	{
		return loader;
	}
};