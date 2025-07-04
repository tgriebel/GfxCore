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

enum loadHandlerFlags_t
{
	LOAD_HANDLER_FLAGS_NONE		= 0,
	LOAD_HANDLER_FLAGS_REBAKE	= ( 1 << 0 ),
};

template< class AssetType >
class LoadHandler
{
private:
	uint32_t m_flags = 0;

protected:
	inline void SetFlags( const uint32_t flags )
	{
		m_flags |= flags;
	}

	inline uint32_t GetFlags() const
	{
		return m_flags;
	}

	inline void ClearFlags( const uint32_t flags )
	{
		m_flags &= ~flags;
	}

	inline bool HasFlags( const uint32_t flags ) const
	{
		return ( m_flags & flags ) != 0;
	}

private:
	virtual bool Load( Asset<AssetType>& asset ) = 0;

	friend class Asset<AssetType>;
};

class AssetInterface
{
protected:
	std::string					m_name;
	std::string					m_json;
	hdl_t						m_handle;

	bool						m_loaded;
	bool						m_uploaded;
	bool						m_isDefault;
	bool						m_canBake;

public:
	AssetInterface() : m_name( "" ), m_loaded( false ), m_isDefault( false ), m_uploaded( false ), m_canBake( true ), m_handle( INVALID_HDL ) {}

	AssetInterface( const hdl_t hdl ) : m_handle( hdl ), m_loaded( false ), m_isDefault( false ), m_canBake( true ), m_uploaded( false )
	{}

	AssetInterface( const std::string& _name, const bool _loaded ) :
		m_name( _name ), m_loaded( _loaded ), m_isDefault( false ), m_uploaded( false )
	{
		m_handle = Hash( m_name );
	}

	virtual bool Load( const bool rebake = false ) = 0;
	virtual void Unload() = 0;
	virtual void Reload( const bool rebake = false ) = 0;
	virtual bool HasLoader() const = 0;
	virtual void Serialize( Serializer* s ) = 0;

	inline const std::string& GetName() const
	{
		return m_name;
	}

	inline const bool SetName( std::string name )
	{
		const uint64_t hash = Hash( name );
		if( hash == m_handle.Get() )
		{
			m_name = name;
			return true;
		}
		return false;
	}

	inline hdl_t Handle() const
	{
		return m_handle;
	}

	inline bool IsLoaded() const
	{
		return m_loaded;
	}

	inline void SetLoaded()
	{
		m_loaded = true;
	}

	inline void QueueUpload()
	{
		m_uploaded = false;
	}

	inline void CompleteUpload()
	{
		m_uploaded = true;
	}

	inline bool IsUploaded() const
	{
		return m_uploaded;
	}

	inline bool IsDefault() const
	{
		return m_isDefault;
	}

	inline void SetDefault()
	{
		m_isDefault = true;
	}

	inline bool CanBake() const
	{
		return m_canBake;
	}

	inline void SetBakeable( const bool isBakeable)
	{
		m_canBake = isBakeable;
	}
};


template< class AssetType >
class Asset : public AssetInterface
{
public:
	friend class LoadHandler<AssetType>;
	using loadHandlerPtr_t = std::unique_ptr< LoadHandler<AssetType> >;	

protected:
	loadHandlerPtr_t			m_loader;
	AssetType					m_asset;

public:
	Asset() : AssetInterface(), m_loader( nullptr ) {}

	Asset( const hdl_t hdl ) : AssetInterface( hdl ), m_loader( nullptr ) {}

	Asset( const std::string& _name  ) : AssetInterface( _name, false ), m_loader( nullptr ) {}

	Asset( const AssetType& _asset, const std::string& _name, const bool _loaded = true ) :
		AssetInterface( _name, _loaded ), m_asset( _asset ), m_loader( nullptr ) {}

	inline void AttachLoader( loadHandlerPtr_t _loader )
	{
		m_loader = std::move( _loader );
	}

	inline const AssetType& Get() const
	{
		return m_asset;
	}

	inline AssetType& Get()
	{
		return m_asset;
	}

	bool HasLoader() const override
	{
		return m_loader ? true : false;
	}

	bool Load( const bool rebake = false ) override
	{
		if ( ( m_loaded == false ) && HasLoader() )
		{
			const uint32_t flags = m_loader->GetFlags();
			if( rebake ) {
				m_loader->SetFlags( LOAD_HANDLER_FLAGS_REBAKE );
			}

			m_loaded = m_loader->Load( *this );

			if ( rebake ) {
				m_loader->SetFlags( flags );
			}
			return m_loaded;
		}
		return true;
	}

	void Unload() override
	{
		m_asset.~AssetType();
		m_loaded = false;
	}

	void Reload( const bool rebake = false ) override
	{
		Unload();
		Load( rebake );	
		QueueUpload();
	}

	void Serialize( Serializer* s )
	{
		m_asset.Serialize( s );
	}
};