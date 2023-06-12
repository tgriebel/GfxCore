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

#include <unordered_map>
#include <iterator>
#include <string>
#include "handle.h"
#include "util.h"

template< class AssetType >
class Asset;

template< class AssetType >
class LoadHandler
{
private:
	virtual bool Load( AssetType& asset ) = 0;

	friend class Asset<AssetType>;
};

class AssetInterface
{
protected:
	std::string					name;
	std::string					json;
	hdl_t						handle;

	bool						loaded;
	bool						uploaded;
	bool						isDefault;

public:
	AssetInterface() : name( "" ), loaded( false ), isDefault( false ), uploaded( false ), handle( INVALID_HDL ) {}

	AssetInterface( const std::string& _name, const bool _loaded ) :
		name( _name ), loaded( _loaded ), isDefault( false ), uploaded( false )
	{
		handle = Hash( name );
	}

	inline const std::string& GetName() const
	{
		return name;
	}

	inline hdl_t Handle() const
	{
		return handle;
	}

	inline bool IsLoaded() const
	{
		return loaded;
	}

	inline void SetLoaded()
	{
		loaded = true;
	}

	inline void QueueUpload()
	{
		uploaded = false;
	}

	inline void CompleteUpload()
	{
		uploaded = true;
	}

	inline bool IsUploaded() const
	{
		return uploaded;
	}

	inline bool IsDefault() const
	{
		return isDefault;
	}

	inline void SetDefault()
	{
		isDefault = true;
	}
};


template< class AssetType >
class Asset : public AssetInterface
{
public:
	using loadHandlerPtr_t = std::unique_ptr< LoadHandler<AssetType> >;

protected:
	loadHandlerPtr_t			loader;
	AssetType					asset;

public:
	Asset() : AssetInterface(), loader( nullptr ) {}

	Asset( const AssetType& _asset, const std::string& _name, const bool _loaded = true ) :
		AssetInterface( _name, _loaded ), asset( _asset ), loader( nullptr ) {}

	inline void AttachLoader( loadHandlerPtr_t _loader )
	{
		loader = std::move( _loader );
	}

	inline const AssetType& Get() const
	{
		return asset;
	}

	inline AssetType& Get()
	{
		return asset;
	}

	inline bool HasLoader() const
	{
		return loader ? true : false;
	}

	inline bool Load()
	{
		if ( ( loaded == false ) && HasLoader() )
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