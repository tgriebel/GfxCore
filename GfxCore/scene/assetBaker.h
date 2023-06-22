#pragma once
#include <vector>
#include <string>

template<class T>
class AssetLib;

class Model;
class Material;
class Image;
class GpuProgram;

class AssetBaker
{
private:
	std::string				m_bakePath;
	std::string				m_modelPath;	
	std::string				m_modelExt;
	std::string				m_materialPath;
	std::string				m_materialExt;
	std::string				m_imagePath;
	std::string				m_imageExt;
	AssetLib<Model>*		m_modelLib;
	AssetLib<Material>*		m_materialLib;
	 AssetLib<Image>*		m_imageLib;
public:
	AssetBaker() {}

	void AddAssetLib( AssetLib<Model>* lib, const std::string path, const std::string ext );
	void AddAssetLib( AssetLib<Material>* lib, const std::string path, const std::string ext );
	void AddAssetLib( AssetLib<Image>* lib, const std::string path, const std::string ext );
	void AddBakeDirectory( const std::string path );
	void Bake();
};