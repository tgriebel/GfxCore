#pragma once

#include <vector>
#include <string>
#include <fstream>

#include "../core/handle.h"

struct Scene;

std::vector<char> ReadFile( const std::string& filename );
hdl_t LoadRawModel( Scene& scene, const std::string& fileName, const std::string& objectName, const std::string& modelPath, const std::string& texturePath );
bool LoadModel( Scene& scene, const hdl_t& hdl, const std::string& bakePath, const std::string& modelPath, const std::string& ext );
bool WriteModel( Scene& scene, const std::string& fileName, hdl_t modelHdl );