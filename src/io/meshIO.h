/*
* MIT License
*
* Copyright( c ) 2013-2023 Thomas Griebel
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

#include <string>
#include <vector>
#include <map>

namespace MeshIO
{
	class Polytope;
	class Tetgen;
	class Off;
	class Obj;

	struct vector_t
	{
		vector_t() : x( 0.0 ), y( 0.0 ), z( 0.0 ), w( 0.0 ) {}

		float	x;
		float	y;
		float	z;
		float	w;
	};

	struct node_t
	{
		node_t() : r( 0 ), g( 0 ), b( 0 ), a( 0 ) {}

		vector_t pos;
		float	r;
		float	g;
		float	b;
		float	a;
		bool	colored;
	};

	struct objIndex_t
	{
		int32_t vertexIx;
		int32_t uvIx;
		int32_t normalIx;
	};

	struct objFace_t
	{
		std::vector<objIndex_t> vertices;
	};

	struct objSmoothingGroup_t
	{
		std::vector<objFace_t> faces;
	};

	struct objGroup_t
	{
		std::string material;
		std::map<int32_t, objSmoothingGroup_t> smoothingGroups;
	};

	struct edge_t
	{
		int32_t	n1;
		int32_t	n2;
		edge_t() : n1( 0 ), n2( 0 ) {}
	};

	class Polytope
	{
	public:
		int32_t		npoints;
		int32_t* points;
		float		r, g, b, a;
		bool		planar;

		Polytope() : npoints( 0 ), points( NULL ), r( 0 ), g( 0 ), b( 0 ), a( 0 ), planar( true ) {}

		Polytope( int32_t _npoints, bool _planar ) : npoints( _npoints ), r( 0 ), g( 0 ), b( 0 ), a( 0 ), planar( _planar )
		{
			points = new int32_t[ npoints ];
		}

		Polytope( const Polytope& source ) :
			npoints( source.npoints ),
			r( source.r ),
			g( source.g ),
			b( source.b ),
			a( source.a ),
			planar( source.planar )
		{
			points = new int32_t[ npoints ];
			for ( int32_t i( 0 ); i < npoints; ++i )
				points[ i ] = source.points[ i ];
		}

		void init( int32_t _npoints, bool _planar )
		{
			r = g = b = a = 0;
			planar = _planar;
			npoints = _npoints;
			points = new int32_t[ _npoints ];
		}

		~Polytope()
		{
			if ( points != NULL )
			{
				delete[] points;
			}
		}
	};

	class Off
	{
	public:
		int32_t		verticesCnt;
		int32_t		edgesCnt;
		int32_t		facesCnt;
		node_t*		vertices;
		Polytope*	faces;
		bool		vertices_colored;
		bool		faces_colored;

		Off() :verticesCnt( 0 ), edgesCnt( 0 ), facesCnt( 0 ), vertices( NULL ), faces( NULL ), vertices_colored( false ), faces_colored( false ) {}

		Off( int32_t _vertices, int32_t _edges, int32_t _faces ) : verticesCnt( _vertices ), edgesCnt( _edges ), facesCnt( _faces ), vertices_colored( false ), faces_colored( false )
		{
			vertices = new node_t[ verticesCnt ];
			faces = new Polytope[ facesCnt ];
		}

		Off( const Off& source ) :
			verticesCnt( source.verticesCnt ),
			edgesCnt( source.edgesCnt ),
			facesCnt( source.facesCnt ),
			vertices_colored( source.vertices_colored ),
			faces_colored( source.faces_colored )
		{
			vertices = new node_t[ verticesCnt ];
			faces = new Polytope[ facesCnt ];

			for ( int32_t i( 0 ); i < verticesCnt; ++i )	vertices[ i ] = source.vertices[ i ];
			for ( int32_t i( 0 ); i < facesCnt; ++i )		faces[ i ] = source.faces[ i ];
		}

		void Init( int32_t _vertices, int32_t _edges, int32_t _faces )
		{
			verticesCnt = _vertices;
			edgesCnt = _edges;
			facesCnt = _faces;
			vertices_colored = false;
			faces_colored = false;

			vertices = new node_t[ verticesCnt ];
			faces = new Polytope[ facesCnt ];
		}

		~Off()
		{
			delete[] vertices;
			delete[] faces;
		}
	};

	class Obj
	{
	public:
		std::vector<std::string>			materialLibs;
		std::vector<vector_t>				vertices;
		std::vector<vector_t>				normals;
		std::vector<vector_t>				uvs;
		std::map<std::string, objGroup_t>	groups;
	};

	struct objMaterial_t
	{
		float		Ns;
		float		Ni;
		float		d;
		float		Tr;
		vector_t	Tf;
		int			illum;
		vector_t	Ka;
		vector_t	Kd;
		vector_t	Ks;
		vector_t	Ke;
		std::string	map_Ka;
		std::string	map_Kd;
		std::string	map_Disp;
		std::string	name;

		objMaterial_t()
		{
			Ns = 0.0;
			Ni = 0.0;
			d = 0.0;
			Tr = 0.0;
			illum = 0;
		}
	};

	class Tetgen
	{
	public:
		int32_t numVertices, numFaces, numTets;

		node_t* vertices;
		Polytope* faces;
		Polytope* tets;

		Tetgen( int32_t n_vertices_i, int32_t n_faces_i, int32_t n_tets_i ) :
			numVertices( n_vertices_i ),
			numFaces( n_faces_i ),
			numTets( n_tets_i ),
			vertices( NULL ),
			faces( NULL ),
			tets( NULL )
		{
			if ( numVertices > 0 )	vertices = new node_t[ numVertices ];
			if ( numFaces > 0 )		faces = new Polytope[ numFaces ];
			if ( numTets > 0 )		tets = new Polytope[ numFaces ];
		}

		Tetgen() : vertices( NULL ), faces( NULL ), tets( NULL ), numVertices( 0 ), numFaces( 0 ), numTets( 0 ) {}

		~Tetgen()
		{
			if ( vertices != NULL )	delete[] vertices;
			if ( faces != NULL )	delete[] faces;
			if ( tets != NULL )		delete[] tets;
		}

		void Init( int32_t _numVertices, int32_t _numFaces, int32_t _numTets )
		{
			numVertices = _numVertices;
			numFaces = _numFaces;
			numTets = _numTets;

			if ( vertices != NULL )	delete[] vertices;
			if ( faces != NULL )	delete[] faces;
			if ( tets != NULL )		delete[] tets;

			if ( numVertices > 0 )	vertices = new node_t[ numVertices ];
			if ( numFaces > 0 )		faces = new Polytope[ numFaces ];
			if ( numTets > 0 )		tets = new Polytope[ numTets ];
		}

		void InitVertices( int32_t _numVertices )
		{
			if ( _numVertices <= 0 )
				return;

			numVertices = _numVertices;
			if ( vertices != NULL )
			{
				delete[] vertices;
			}
			vertices = new node_t[ numVertices ];
		}

		void InitFaces( int32_t _numFaces )
		{
			if ( _numFaces <= 0 )
				return;

			numFaces = _numFaces;
			if ( faces != NULL )
			{
				delete[] faces;
			}
			faces = new Polytope[ numFaces ];
		}

		void InitTets( int32_t _numTets )
		{
			if ( _numTets <= 0 )
				return;

			numTets = _numTets;
			if ( tets != NULL )
			{
				delete[] tets;
			}
			tets = new Polytope[ numTets ];
		}
	};

	void CleanLine( std::string& str, std::string& outStr );
	void ReadTetGen( const std::string& baseFileName, Tetgen& mesh );
	void WriteTetGen( const std::string& baseFileName, const Tetgen& mesh );
	void ReadOFF( const std::string& file_name, Off& mesh );
	void WriteOFF( const std::string& file_name, const Off& mesh );
	void ReadObj( const std::string& fileName, Obj& mesh );
	void WriteObj( const std::string& fileName, const Obj& mesh );
	void ReadMtl( const std::string& fileName, objMaterial_t& mtl );
	void WriteMtl( const std::string& fileName, objMaterial_t& mtl );
}