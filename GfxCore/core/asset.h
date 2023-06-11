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

template< class AssetType >
class Asset
{
public:
	using loadHandlerPtr_t = std::unique_ptr< LoadHandler<AssetType> >;

protected:
	std::string					name;
	std::string					json;
	hdl_t						handle;
	AssetType					asset;
	loadHandlerPtr_t			loader;
	bool						loaded;
	bool						uploaded;
	bool						isDefault;

public:
	Asset() : name( "" ), loaded( false ), isDefault( false ), uploaded( false ), loader( nullptr ), handle( INVALID_HDL ) {}

	Asset( const AssetType& _asset, const std::string& _name, const bool _loaded = true ) : 
		name( _name ), asset( _asset ), loaded( _loaded ), isDefault( false ), uploaded( false ), loader( nullptr )
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