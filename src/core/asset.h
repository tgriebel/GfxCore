#pragma once

#include <unordered_map>
#include <iterator>
#include <string>

template< class AssetType >
class Asset
{
protected:
	std::string	name;
	bool		loaded;
	AssetType	asset;

public:
	Asset() : name( "" ), loaded( false ) {}
	Asset( const AssetType& _asset, const std::string _name, const bool _loaded = true ) : name( _name ), asset( _asset ), loaded( _loaded ) {}

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

	inline const AssetType& Get() const
	{
		return asset;
	}

	inline AssetType& Get()
	{
		return asset;
	}
};