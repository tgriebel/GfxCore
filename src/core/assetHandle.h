/*
* MIT License
*
* Copyright( c ) 2023 Thomas Griebel
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

#include "asset.h"
#include "assetLib.h"
#include "handle.h"

template< class AssetType >
class Asset;

template< class AssetType >
class AssetHandle
{
private:
	AssetLib<AssetType>*	lib;
	hdl_t					handle
public:
	AssetHandle()
	{
		lib = nullptr;
		handle = INVALID_HDL;
	}

	AssetHandle( hdl_t _handle, AssetLib<AssetType>& _lib ) : handle( _handle ), lib( &lib )
	{}

	AssetHandle( const AssetHandle<AssetType>& asset )
	{
		lib = asset.lib;
		handle = asset.handle;
	}

	AssetHandle<AssetType>& operator=( const AssetHandle<AssetType>& rhs )
	{
		if ( this != &rhs )
		{
			lib = rhs.lib;
			handle = rhs.handle;
		}
		return *this;
	}

	hdl_t GetHandle() const
	{
		return handle;
	}

	Asset<AssetType>* Resolve()
	{
		return ( lib != nullptr ) ? lib.Find( handle ) : nullptr;
	}
};