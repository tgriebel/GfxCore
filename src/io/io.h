#pragma once

#include <vector>
#include <string>
#include <fstream>

#include "../core/handle.h"

class Texture;
class Model;
class AssetManager;

template<class T>
class Asset;

std::vector<char> ReadFile( const std::string& filename );
bool LoadTextureImage( const char* texturePath, Texture& texture );
bool LoadTextureCubeMapImage( const char* textureBasePath, const char* ext, Texture& texture );
bool LoadRawModel( AssetManager& assets, const std::string& fileName, const std::string& modelPath, const std::string& texturePath, Model& model );
bool LoadModel( AssetManager& assets, const hdl_t& hdl, const std::string& bakePath, const std::string& modelPath, const std::string& ext );
bool WriteModel( Asset<Model>* model, const std::string& fileName );