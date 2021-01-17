
#pragma once

#include <fstream>
#include <sstream>
#include <limits>
#include <iomanip>
#include <algorithm>
#include <assert.h>
#include "meshIO.h"

namespace MeshIO
{
	void CleanLine( std::string& str, std::string& outStr )
	{
		const size_t offset = str.find( "#" );

		if ( offset != std::string::npos )
		{
			outStr = str.substr( 0, offset );
			return;
		}

		outStr = str;

		std::for_each( outStr.begin(), outStr.end(), []( char& c ) {
			c = ::tolower( c );
			} );
	}


	void ReadTetGen( const std::string& baseFileName, Tetgen& mesh )
	{
		std::string nodeFileName = baseFileName + ".node";
		std::fstream nodeStream( nodeFileName.c_str(), std::fstream::in );

		if ( !nodeStream )
			return;

		//First line: <# of points> <dimension (must be 3)> <# of attributes> <# of boundary markers (0 or 1)>
		//Remaining lines list # of points:
		//<point #> <x> <y> <z> [attributes] [boundary marker]

		// TODO: do something with attributes and markers??
		int32_t trash;
		int32_t points;
		nodeStream >> points;
		nodeStream.ignore( std::numeric_limits<std::streamsize>::max(), '\n' );

		// Read vertices
		mesh.InitVertices( points );
		for ( int32_t i = 0; i < points; ++i )
		{
			nodeStream >> trash;
			nodeStream >> mesh.vertices[ i ].pos.x;
			nodeStream >> mesh.vertices[ i ].pos.y;
			nodeStream >> mesh.vertices[ i ].pos.z;
			nodeStream.ignore( std::numeric_limits<std::streamsize>::max(), '\n' );
		}
		nodeStream.close();

		//First line: <# of faces> <boundary marker (0 or 1)>
		//Remaining lines list of # of faces:
		//<face #> <node> <node> <node> [boundary marker]

		std::string face_file_name = baseFileName + ".face";
		std::fstream faceStream( face_file_name.c_str(), std::fstream::in );
		if ( !faceStream ) return;

		int32_t faces;
		faceStream >> faces;
		faceStream.ignore( std::numeric_limits<std::streamsize>::max(), '\n' );

		// Read faces
		mesh.InitFaces( faces );
		for ( int32_t i( 0 ); i < faces; ++i )
		{
			faceStream >> trash;
			mesh.faces[ i ].init( 3, true );
			faceStream >> mesh.faces[ i ].points[ 0 ];
			faceStream >> mesh.faces[ i ].points[ 1 ];
			faceStream >> mesh.faces[ i ].points[ 2 ];
			faceStream.ignore( std::numeric_limits<std::streamsize>::max(), '\n' );
		}
		faceStream.close();

		//First line: <# of tetrahedra> <nodes per tetrahedron> <# of attributes>
		//Remaining lines list of # of tetrahedra:
		//<tetrahedron #> <node> <node> <node> <node> ... [attributes]

		std::string elementFileName = baseFileName + ".ele";
		std::fstream elementStream = std::fstream( elementFileName.c_str(), std::fstream::in );
		if ( !elementStream )
			return;

		int32_t tets, nodes_per;
		elementStream >> tets;
		elementStream >> nodes_per;
		elementStream.ignore( std::numeric_limits<std::streamsize>::max(), '\n' );
		//read tets

		mesh.InitTets( tets );
		for ( int32_t i = 0; i < tets; ++i )
		{
			elementStream >> trash;

			mesh.tets[ i ].init( nodes_per, false );
			for ( int32_t j = 0; j < nodes_per; ++j )
			{
				elementStream >> mesh.tets[ i ].points[ j ];
			}
			elementStream.ignore( std::numeric_limits<std::streamsize>::max(), '\n' );
		}
		elementStream.close();
	}


	void WriteTetGen( const std::string& baseFileName, const Tetgen& mesh )
	{
		std::fstream nodeFileStream = std::fstream( ( baseFileName + ".node" ).c_str(), std::fstream::out );
		std::fstream faceFileStream = std::fstream( ( baseFileName + ".face" ).c_str(), std::fstream::out );
		std::fstream elementFileStream = std::fstream( ( baseFileName + ".ele" ).c_str(), std::fstream::out );

		//First line: <# of points> <dimension (must be 3)> <# of attributes> <# of boundary markers (0 or 1)>
		//Remaining lines list # of points:
		//<point #> <x> <y> <z> [attributes] [boundary marker]

		nodeFileStream << mesh.numVertices << " " << 3 << " " << 0 << " " << 0 << "\n";

		// Write vertices
		for ( int32_t i = 0; i < mesh.numVertices; ++i )
		{
			nodeFileStream << ( i + 1 ) << " " << mesh.vertices[ i ].pos.x << " " << mesh.vertices[ i ].pos.y << " " << mesh.vertices[ i ].pos.z << " \n";
		}
		nodeFileStream << "#finished" << std::endl;
		nodeFileStream.close();
		//First line: <# of faces> <boundary marker (0 or 1)>
		//Remaining lines list of # of faces:
		//<face #> <node> <node> <node> [boundary marker]

		// Write faces
		faceFileStream << mesh.numFaces << " " << 1 << "\n";

		for ( int32_t i = 0; i < mesh.numFaces; ++i )
		{
			faceFileStream << ( i + 1 ) << " " << mesh.faces[ i ].points[ 0 ] << " " << mesh.faces[ i ].points[ 1 ] << " " << mesh.faces[ i ].points[ 2 ] << "\n";
		}
		faceFileStream << "#finished" << std::endl;
		faceFileStream.close();

		//First line: <# of tetrahedra> <nodes per tetrahedron> <# of attributes>
		//Remaining lines list of # of tetrahedra:
		//<tetrahedron #> <node> <node> <node> <node> ... [attributes]

		// Write tets
		elementFileStream << mesh.numTets << " " << 4 << " " << 0 << "\n";

		for ( int32_t i = 0; i < mesh.numTets; ++i )
		{
			elementFileStream << ( i + 1 ) << " ";

			for ( int32_t j = 0; j < 4; ++j )
			{
				elementFileStream << mesh.tets[ i ].points[ j ] << " ";
			}
			elementFileStream << "\n";
		}
		elementFileStream << "#finished" << std::endl;
		elementFileStream.close();
	}


	void ReadOFF( const std::string& file_name, Off& mesh )
	{
		// TODO: close if not exist!!

		std::fstream inputStream = std::fstream( file_name.c_str(), std::fstream::in );

		//stringstream input_stream;
		//_removeUnnecessaryCharacters(filestr, input_stream);

		std::string line( "" );
		getline( inputStream, line );

		getline( inputStream, line );

		std::stringstream ss;
		ss.clear();
		ss.str( line );

		int32_t vertices, edges, faces;
		ss >> vertices;
		ss >> faces;
		ss >> edges;

		mesh.Init( vertices, edges, faces );

		// Read vertices
		for ( int32_t i = 0; i < mesh.verticesCnt; ++i )
		{
			getline( inputStream, line );
			ss.clear();
			ss.str( line );

			ss >> mesh.vertices[ i ].pos.x;
			ss >> mesh.vertices[ i ].pos.y;
			ss >> mesh.vertices[ i ].pos.z;

			ss >> mesh.vertices[ i ].r;
			ss >> mesh.vertices[ i ].g;
			ss >> mesh.vertices[ i ].b;
			ss >> mesh.vertices[ i ].a;

			mesh.vertices[ i ].colored = true;
		}

		// Read polygons
		for ( int32_t i = 0; i < mesh.facesCnt; ++i )
		{
			getline( inputStream, line );
			ss.clear();
			ss.str( line );

			int32_t pts;
			ss >> pts;
			mesh.faces[ i ].init( pts, true );

			for ( int32_t j = 0; j < mesh.faces[ i ].npoints; ++j )
			{
				int32_t buffer;
				ss >> buffer;
				mesh.faces[ i ].points[ j ] = buffer;
			}
		}
		/*
		//read face colors
		if(!input_stream.eof()){
			for(SIZE i(0); i < mesh.nvertices; ++i){

				getline(input_stream, line);
				ss.clear();
				ss.str(line);

				SIZE trash;
				for(SIZE j(0); j <= mesh.faces[i].npoints; ++j){
					ss >> trash;
				}

				ss >> mesh.faces[i].r;
				ss >> mesh.faces[i].g;
				ss >> mesh.faces[i].b;
				ss >> mesh.faces[i].a;
			}

			mesh.faces_colored = true;
		}*/

		inputStream.close();
	}


	void WriteOFF( const std::string& file_name, const Off& mesh )
	{
		std::fstream output = std::fstream( file_name.c_str(), std::fstream::out );
		output << "OFF" << "\n";
		output << mesh.verticesCnt << " " << mesh.facesCnt << " " << mesh.edgesCnt << "\n";

		// Write vertices
		for ( int32_t i = 0; i < mesh.verticesCnt; ++i )
		{
			output << mesh.vertices[ i ].pos.x << " " << mesh.vertices[ i ].pos.y << " " << mesh.vertices[ i ].pos.z;

			if ( mesh.vertices[ i ].colored )
			{
				output << " " << mesh.vertices[ i ].r << " " << mesh.vertices[ i ].g << " " << mesh.vertices[ i ].b << " " << mesh.vertices[ i ].a << "\n";
			}
			else
			{
				output << " " << 1 << " " << 1 << " " << 1 << " " << 1 << "\n";
			}
		}

		// Write faces
		for ( int32_t i = 0; i < mesh.facesCnt; ++i )
		{
			output << mesh.faces[ i ].npoints << " ";

			for ( int32_t j = 0; j < mesh.faces[ i ].npoints; ++j )
			{
				output << mesh.faces[ i ].points[ j ] << " ";
			}

			output << "\n";
		}

		output.close();
	}


	void ReadObj( const std::string& fileName, Obj& mesh )
	{
		std::fstream inputStream = std::fstream( fileName.c_str(), std::fstream::in );

		if ( inputStream.fail() )
		{
			inputStream.close();
			return;
		}

		struct state_t
		{
			std::string	object;
			std::string	group;
			std::string	material;
			int32_t		smoothGroup;
		};

		state_t state;
		state.object = "";
		state.group = "";
		state.material = "";
		state.smoothGroup = 0;

		mesh.vertices.reserve( 10000 );
		mesh.normals.reserve( 10000 );
		mesh.uvs.reserve( 10000 );

		while ( !inputStream.eof() )
		{
			std::string line = "";
			std::string srcLine = "";

			getline( inputStream, srcLine );

			CleanLine( srcLine, line );

			if ( line.length() == 0 )
				continue;

			std::stringstream ss;
			ss.clear();
			ss.str( line );

			std::string semanticToken;
			ss >> semanticToken;

			if ( semanticToken.compare( "mtllib" ) == 0 )
			{
				std::string mtlFileName;
				ss >> mtlFileName;
				mesh.materialLibs.push_back( mtlFileName );
			}
			else if ( semanticToken.compare( "v" ) == 0 )
			{
				vector_t v;
				ss >> v.x;
				ss >> v.y;
				ss >> v.z;
				mesh.vertices.push_back( v );
			}
			else if ( semanticToken.compare( "vn" ) == 0 )
			{
				vector_t v;
				ss >> v.x;
				ss >> v.y;
				ss >> v.z;
				mesh.normals.push_back( v );
			}
			else if ( semanticToken.compare( "vt" ) == 0 ) // Texture UVs
			{
				vector_t v;
				ss >> v.x;
				if ( !ss.eof() )
				{
					ss >> v.y;
				}
				if ( !ss.eof() )
				{
					ss >> v.z;
				}
				mesh.uvs.push_back( v );
			}
			else if ( semanticToken.compare( "vp" ) == 0 ) // Parameter space vertices
			{
				assert( false ); // Unimplemented
			}
			else if ( semanticToken.compare( "s" ) == 0 ) // Smooth shading
			{
				ss >> state.smoothGroup;
			}
			else if ( semanticToken.compare( "o" ) == 0 ) // Object name
			{
				assert( false ); // Unimplemented
				ss >> state.object;
			}
			else if ( semanticToken.compare( "g" ) == 0 ) // Group name
			{
				ss >> state.group;
			}
			else if ( semanticToken.compare( "usemtl" ) == 0 ) // Material for element
			{
				ss >> state.material;
				mesh.groups[ state.group ].material = state.material;
			}
			else if ( semanticToken.compare( "f" ) == 0 ) // Faces
			{
				objFace_t face;

				while ( !ss.eof() )
				{
					objIndex_t indices;
					indices.vertexIx = -1;
					indices.uvIx = -1;
					indices.normalIx = -1;

					std::string facePt;
					ss >> facePt;
					std::stringstream vertexStream( facePt );

					if ( !vertexStream.rdbuf()->in_avail() )
						break;

					int32_t elementIx = 0;

					while ( !vertexStream.eof() && ( elementIx < 3 ) )
					{
						std::string token;
						std::getline( vertexStream, token, '/' );

						std::stringstream tokenStream;
						tokenStream.clear();
						tokenStream.str( token );

						if ( tokenStream.eof() || ( token.length() == 0 ) )
						{
							++elementIx;
							break;
						}

						int32_t index;
						tokenStream >> index;

						index -= 1;

						if ( elementIx == 0 )
						{
							indices.vertexIx = index;
						}
						else if ( elementIx == 1 )
						{
							indices.uvIx = index;
						}
						else if ( elementIx == 2 )
						{
							indices.normalIx = index;
						}
						++elementIx;
					}
					face.vertices.push_back( indices );
				}

				objGroup_t& group = mesh.groups[ state.group ];
				objSmoothingGroup_t& smoothingGroup = group.smoothingGroups[ state.smoothGroup ];
				smoothingGroup.faces.push_back( face );
			}
			else if ( semanticToken.compare( "l" ) == 0 ) // Lines
			{
				assert( false ); // Unimplemented
			}
		}

		inputStream.close();
	}


	void WriteObj( const std::string& fileName, const Obj& mesh )
	{
		std::fstream file;
		file.open( fileName, std::fstream::out );

		if ( file.fail() )
		{
			file.close();
			return;
		}

		file << std::fixed << std::showpoint << std::setprecision( 4 );

		file << "# MeshIO Export\n\n\n";

		const size_t vertCnt = mesh.vertices.size();
		for ( int i = 0; i < vertCnt; ++i )
		{
			file << "v  " << mesh.vertices[ i ].x << " " << mesh.vertices[ i ].y << " " << mesh.vertices[ i ].z << "\n";
		}
		file << "# " << vertCnt << " vertices\n\n";

		const size_t normalCnt = mesh.normals.size();
		for ( int i = 0; i < normalCnt; ++i )
		{
			file << "vn " << mesh.normals[ i ].x << " " << mesh.normals[ i ].y << " " << mesh.normals[ i ].z << "\n";
		}
		file << "# " << normalCnt << " vertex normals\n\n";

		const size_t uvCnt = mesh.uvs.size();
		for ( int i = 0; i < uvCnt; ++i )
		{
			file << "vt " << mesh.uvs[ i ].x << " " << mesh.uvs[ i ].y << " " << mesh.uvs[ i ].z << "\n";
		}
		file << "# " << uvCnt << " texture coords\n\n";

		auto groupIttEnd = mesh.groups.end();
		for ( auto groupItt = mesh.groups.begin(); groupItt != groupIttEnd; ++groupItt )
		{
			file << "g " << groupItt->first << "\n";
			file << "usemtl " << groupItt->second.material << "\n";

			objGroup_t group = groupItt->second;

			auto smoothGroupIttEnd = group.smoothingGroups.end();
			for ( auto smoothGroupItt = group.smoothingGroups.begin(); smoothGroupItt != smoothGroupIttEnd; ++smoothGroupItt )
			{
				file << "s " << smoothGroupItt->first << "\n";

				objSmoothingGroup_t smoothGroup = smoothGroupItt->second;

				auto faceIttEnd = smoothGroup.faces.end();
				for ( auto faceItt = smoothGroup.faces.begin(); faceItt != faceIttEnd; ++faceItt )
				{
					file << "f ";

					auto vertIttEnd = faceItt->vertices.end();
					for ( auto vertItt = faceItt->vertices.begin(); vertItt != vertIttEnd; ++vertItt )
					{
						file << ( 1 + vertItt->vertexIx );

						if ( vertItt->uvIx != -1 )
						{
							file << "/" << ( 1 + vertItt->uvIx );
						}

						if ( vertItt->normalIx != -1 )
						{
							file << "/" << ( 1 + vertItt->normalIx );
						}

						file << " ";
					}
					file << "\n";
				}
				file << "# " << uvCnt << " faces\n\n";
			}
		}

		file.close();
	}


	void ReadMtl( const std::string& fileName, objMaterial_t& mtl )
	{
		std::fstream inputStream = std::fstream( fileName.c_str(), std::fstream::in );

		if ( inputStream.fail() )
		{
			inputStream.close();
			return;
		}

		while ( !inputStream.eof() )
		{
			std::string line = "";
			std::string srcLine = "";

			getline( inputStream, srcLine );

			CleanLine( srcLine, line );

			if ( line.length() == 0 )
				continue;

			std::stringstream ss;
			ss.clear();
			ss.str( line );

			std::string semanticToken;
			ss >> semanticToken;

			if ( semanticToken.compare( "newmtl" ) == 0 )
			{
				ss >> mtl.name;
			}
			else if ( semanticToken.compare( "ns" ) == 0 )
			{
				ss >> mtl.Ns;
			}
			else if ( semanticToken.compare( "ni" ) == 0 )
			{
				ss >> mtl.Ni;
			}
			else if ( semanticToken.compare( "d" ) == 0 )
			{
				ss >> mtl.d;
			}
			else if ( semanticToken.compare( "tr" ) == 0 )
			{
				ss >> mtl.Tr;
			}
			else if ( semanticToken.compare( "tf" ) == 0 )
			{
				ss >> mtl.Tf.x;
				ss >> mtl.Tf.y;
				ss >> mtl.Tf.z;
				ss >> mtl.Tf.w;
			}
			else if ( semanticToken.compare( "illum" ) == 0 )
			{
				ss >> mtl.illum;
			}
			else if ( semanticToken.compare( "ka" ) == 0 )
			{
				ss >> mtl.Ka.x;
				ss >> mtl.Ka.y;
				ss >> mtl.Ka.z;
				ss >> mtl.Ka.w;
			}
			else if ( semanticToken.compare( "kd" ) == 0 )
			{
				ss >> mtl.Kd.x;
				ss >> mtl.Kd.y;
				ss >> mtl.Kd.z;
				ss >> mtl.Kd.w;
			}
			else if ( semanticToken.compare( "ks" ) == 0 )
			{
				ss >> mtl.Ks.x;
				ss >> mtl.Ks.y;
				ss >> mtl.Ks.z;
				ss >> mtl.Ks.w;
			}
			else if ( semanticToken.compare( "ke" ) == 0 )
			{
				ss >> mtl.Ke.x;
				ss >> mtl.Ke.y;
				ss >> mtl.Ke.z;
				ss >> mtl.Ke.w;
			}
			else if ( semanticToken.compare( "map_ka" ) == 0 )
			{
				ss >> mtl.map_Ka;
			}
			else if ( semanticToken.compare( "map_kd" ) == 0 )
			{
				ss >> mtl.map_Kd;
			}
			else if ( semanticToken.compare( "map_disp" ) == 0 )
			{
				ss >> mtl.map_Disp;
			}
		}

		inputStream.close();
	}


	void WriteMtl( const std::string& fileName, objMaterial_t& mtl )
	{
		std::fstream output = std::fstream( fileName.c_str(), std::fstream::out );

		if ( output.fail() )
		{
			output.close();
			return;
		}

		output << "Obj Material" << "\n\n";

		output << "newmtl " << mtl.name << " " << "\n";
		output << "Ns " << mtl.Ns << " " << "\n";
		output << "Ni " << mtl.Ni << " " << "\n";
		output << "d " << mtl.d << " " << "\n";
		output << "Tr " << mtl.Tr << " " << "\n";
		output << "Tf " << mtl.Tf.x << " " << mtl.Tf.y << " " << mtl.Tf.z << " " << mtl.Tf.w << "\n";
		output << "illum " << mtl.illum << " " << "\n";
		output << "Ka " << mtl.Ka.x << " " << mtl.Ka.y << " " << mtl.Ka.z << " " << mtl.Ka.w << "\n";
		output << "Kd " << mtl.Kd.x << " " << mtl.Kd.y << " " << mtl.Kd.z << " " << mtl.Kd.w << "\n";
		output << "Ks " << mtl.Ks.x << " " << mtl.Ks.y << " " << mtl.Ks.z << " " << mtl.Ks.w << "\n";
		output << "Ke " << mtl.Ke.x << " " << mtl.Ke.y << " " << mtl.Ke.z << " " << mtl.Ke.w << "\n";
		output << "map_Ka " << mtl.map_Ka << " " << "\n";
		output << "map_Kd " << mtl.map_Kd << " " << "\n";

		output.close();
	}
};