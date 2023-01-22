#pragma once

#include <unordered_map>
#include <iterator>
#include <string>

template< class AssetType >
class Asset;

template< class AssetType >
class LoadHandler
{
private:
	virtual bool Load( AssetType& asset ) = 0;

	friend class Asset<AssetType>;
};

template< class AssetType >
class Asset
{
public:
	using loadHandlerPtr_t = std::unique_ptr< LoadHandler<AssetType> >;

protected:
	std::string					name;
	std::string					json;
	AssetType					asset;
	loadHandlerPtr_t			loader;
	bool						loaded;
	bool						isDefault;

public:
	Asset() : name( "" ), loaded( false ), isDefault( false ), loader( nullptr ) {}
	Asset( const AssetType& _asset, const std::string& _name, const bool _loaded = true ) : 
		name( _name ), asset( _asset ), loaded( _loaded ), isDefault( false ), loader( nullptr ) {}

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

	inline void AttachLoader( loadHandlerPtr_t _loader )
	{
		loader = std::move( _loader );
	}

	inline bool HasLoader() const
	{
		return loader ? true : false;
	}

	inline bool Load()
	{
		if( ( loaded == false ) && HasLoader() )
		{
			loaded = loader->Load( asset );
			return loaded;
		}
		return true;
	}

	inline void Unload()
	{
		asset.~AssetType();
		loaded = false;
	}
};