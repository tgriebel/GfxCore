#include <map>
#include <deque>
#include <string>
#include <sstream>

#include "geom.h"
#include "bitmap.h"
#include "image.h"
#include "util.h"
#include "resourceManager.h"


struct mdlHeader_t
{
	uint32_t	info;

	uint32_t	vertexOffset;
	uint32_t	indexOffset;
	uint32_t	surfOffset;
	uint32_t	materialOffset;
	uint32_t	imageOffset;

	uint32_t	numVertices;
	uint32_t	numIndices;
	uint32_t	numMaterials;
	uint32_t	numSurfaces;
	uint32_t	numImages;
};


void LoadMaterialObj( const std::string& path, ResourceManager& rm, material_t& material )
{
	if( path.size() <= 0 )
	{
		return;
	}

	MeshIO::objMaterial_t objMaterial;
	MeshIO::ReadMtl( path, objMaterial );

	material.Ni = objMaterial.Ni;
	material.Ns = objMaterial.Ns;
	material.Ka = Color( (float)objMaterial.Ka.x ).AsRGBf();
	material.Ke = Color( (float)objMaterial.Ke.x ).AsRGBf();
	material.Kd = Color( (float)objMaterial.Kd.x ).AsRGBf();
	material.Ks = Color( (float)objMaterial.Ks.x ).AsRGBf();
	material.Tf = Color( (float)objMaterial.Tf.x ).AsRGBf();


	material.Tr = objMaterial.Tr;
	material.illum = static_cast<float>( objMaterial.illum );
	material.textured = false;

	std::string name = path.substr( path.find_last_of( '/' ) + 1, path.size() );

	memset( material.name, 0, material_t::BufferSize );
	strcpy_s( material.name, material_t::BufferSize, name.c_str() );

	if ( objMaterial.map_Kd.size() > 0 )
	{
		Bitmap texture = Bitmap( std::string( "textures/" + objMaterial.map_Kd ) );

		Image<Color> image = Image<Color>( texture.GetWidth(), texture.GetHeight(), Color( 0.0f ), objMaterial.map_Kd.c_str() );
		BitmapToImage( texture, image );
		material.colorMapId = rm.StoreImageCopy( image );
		material.textured = true;
	}
}


uint32_t LoadModelObj( const std::string& path, ResourceManager& rm )
{
	/////////////////////////////////////////////
	//                                         //
	// Load model and clean data			   //
	//                                         //
	/////////////////////////////////////////////

	MeshIO::Obj objMesh;
	MeshIO::ReadObj( path, objMesh );

	auto& groups = objMesh.groups;

	// Format obj into vertices and indices
	std::vector<vertex_t> uniqueVertices; // TODO: replace with set or map
	std::vector<int32_t> indices;

	for ( auto group = groups.begin(); group != groups.end(); ++group )
	{
		auto& smoothingGroup = group->second.smoothingGroups;
		for ( auto smoothGroup = smoothingGroup.begin(); smoothGroup != smoothingGroup.end(); ++smoothGroup )
		{
			std::vector<MeshIO::objFace_t>& faces = smoothGroup->second.faces;

			faces.reserve( 10000 );

			const uint32_t faceCnt = static_cast<uint32_t>( faces.size() );
			for ( uint32_t faceIx = 0; faceIx < faceCnt; ++faceIx )
			{
				const uint32_t vertexCnt = static_cast<uint32_t>( faces[ faceIx ].vertices.size() );
				assert( ( vertexCnt == 3 ) || ( vertexCnt == 4 ) );
				for ( uint32_t i = 0; i < vertexCnt; ++i )
				{
					vertex_t vert;
					const int32_t vertIx = faces[ faceIx ].vertices[ i ].vertexIx;
					const int32_t uvIx = faces[ faceIx ].vertices[ i ].uvIx;
					const int32_t normalIx = faces[ faceIx ].vertices[ i ].normalIx;

					MeshIO::vector_t srcVertex = objMesh.vertices[ vertIx ];
					
					vert.pos = vec4f( srcVertex.x, srcVertex.y, srcVertex.z, srcVertex.w );
					vert.color = Color::White;

					// TODO: default in MeshIO
					if( uvIx != -1 )
					{
						MeshIO::vector_t srcUv = objMesh.uvs[ uvIx ];
						vert.uv = vec2f( srcUv.x, srcUv.y );
					}
					else
					{
						vert.uv = vec2f( 0.0f, 0.0f );
					}

					if ( normalIx != -1 )
					{
						MeshIO::vector_t srcNormal = objMesh.normals[ normalIx ];
						vert.normal = vec3f( srcNormal.x, srcNormal.y, srcNormal.z ).Normalize();
					}
					else
					{
						vert.normal = vec3f( 1.0f, 1.0f, 1.0f ).Normalize();
					}

					auto it = std::find( uniqueVertices.begin(), uniqueVertices.end(), vert );

					if ( it == uniqueVertices.end() )
					{
						indices.push_back( static_cast<int32_t>( uniqueVertices.size() ) );
						uniqueVertices.push_back( vert );
					}
					else
					{
						indices.push_back( static_cast<int32_t>( std::distance( uniqueVertices.begin(), it ) ) );
					}
				}

				if( vertexCnt == 4 )
				{					
					// For quads do a very simple triangulation by appending the two
					// adjacent indices to make the second triangle in the quad.
					// This totals 6 indices, 3 for each triangle
					// TODO: winding order
					assert( indices.size() >= 4 );
					const uint32_t v0 = *( indices.end() - 4 );
					const uint32_t v2 = *( indices.end() - 2 );

					indices.push_back( v0 );
					indices.push_back( v2 );
				}
			}
		}
	}

	// Normalize UVs to [0, 1]
	// TODO: leave as-is and let texture wrap mode deal with it?
	{
		auto vertEnd = uniqueVertices.end();
		for( auto it = uniqueVertices.begin(); it != vertEnd; ++it )
		{
			it->uv[ 0 ] = ( it->uv[ 0 ] > 1.0 ) ? ( it->uv[ 0 ] - floor( it->uv[ 0 ] ) ) : it->uv[ 0 ];
			it->uv[ 1 ] = ( it->uv[ 1 ] > 1.0 ) ? ( it->uv[ 1 ] - floor( it->uv[ 1 ] ) ) : it->uv[ 1 ];

			it->uv[ 0 ] = Saturate( it->uv[ 0 ] );
			it->uv[ 1 ] = Saturate( it->uv[ 1 ] );

			it->uv[ 1 ] = 1.0f - it->uv[ 1 ];
		}
	}

	/////////////////////////////////////////////
	//                                         //
	// Construct final object representation   //
	//                                         //
	/////////////////////////////////////////////

	const uint32_t modelIx = rm.AllocModel();
	ModelSource* model = rm.GetModel( modelIx );
	model->surfs.push_back( surface_t() );

	surface_t& surf = model->surfs.back();

	// Build VB and IB
	{
		model->name = path;
		surf.vb = rm.GetVB();
		surf.ib = rm.GetIB();
		surf.vbOffset = rm.GetVbOffset();
		surf.ibOffset = rm.GetIbOffset();

		const uint32_t vertexCnt = static_cast<uint32_t>( uniqueVertices.size() );
		for ( uint32_t i = 0; i < vertexCnt; ++i )
		{
			rm.AddVertex( uniqueVertices[ i ] );
		}
		surf.vbEnd = rm.GetVbOffset();

		const size_t indexCnt = indices.size();
		assert( ( indexCnt % 3 ) == 0 );
		for ( size_t i = 0; i < indexCnt; i++ )
		{
			rm.AddIndex( surf.vbOffset + indices[ i ] );
		}
		surf.ibEnd = rm.GetIbOffset();
	}

	// MeshIO::WriteObj( std::string( "outtest.obj" ), objMesh );
	return modelIx;
}


void StoreModelObj( const std::string& path, ResourceManager& rm, const uint32_t modelIx )
{
	const ModelSource* model = rm.GetModel( modelIx );

	const surface_t& surf = model->surfs[ 0 ];

	rm.PushVB( surf.vb );
	rm.PushIB( surf.ib );

	MeshIO::Obj meshObj;
	for ( uint32_t i = surf.vbOffset; i < surf.vbEnd; ++i )
	{
		const vertex_t* v = rm.GetVertex( i );

		MeshIO::vector_t vert;
		vert.x = v->pos[ 0 ];
		vert.y = v->pos[ 1 ];
		vert.z = v->pos[ 2 ];
		vert.w = v->pos[ 3 ];
		meshObj.vertices.push_back( vert );

		MeshIO::vector_t normal;
		normal.x = v->normal[ 0 ];
		normal.y = v->normal[ 1 ];
		normal.z = v->normal[ 2 ];
		normal.w = 0.0;
		meshObj.normals.push_back( normal );

		MeshIO::vector_t uv;
		uv.x = v->uv[ 0 ];
		uv.y = v->uv[ 1 ];
		uv.z = 0.0;
		uv.w = 0.0;
		meshObj.uvs.push_back( uv );
	}

	MeshIO::objGroup_t group;
	MeshIO::objSmoothingGroup_t smoothingGroup;

	group.material = "default";

	for ( uint32_t i = surf.ibOffset; i < surf.ibEnd; i += 3 )
	{
		MeshIO::objFace_t face;
		for ( uint32_t j = 0; j < 3; ++j )
		{
			const uint32_t index = rm.GetIndex( i + j ) - surf.vbOffset;

			MeshIO::objIndex_t indexTuple;
			indexTuple.vertexIx = index;
			indexTuple.uvIx = index;
			indexTuple.normalIx = index;

			// TODO: deduplicate
			face.vertices.push_back( indexTuple );
		}

		smoothingGroup.faces.push_back( face );
	}

	group.smoothingGroups[ 0 ] = smoothingGroup;
	meshObj.groups[ model->name ] = group;

	MeshIO::WriteObj( path, meshObj );

	rm.PopVB();
	rm.PopIB();
}


uint32_t LoadModelBin( const std::string& path, ResourceManager& rm )
{
	mdlHeader_t inHeader;
	uint32_t byteChkSize = 0;
	uint32_t inFileSize = 0;

	std::fstream file;

	file.open( path, std::fstream::in | std::ios::binary );
	file.read( (char*)&inHeader, sizeof( mdlHeader_t ) );

	inFileSize = inHeader.imageOffset;

	auto inVert = std::unique_ptr<vertex_t[]>( new vertex_t[ inHeader.numVertices ] );
	auto inIndices = std::unique_ptr<uint32_t[]>( new uint32_t[ inHeader.numIndices ] );
	auto inSurfaces = std::unique_ptr<surface_t[]>( new surface_t[ inHeader.numSurfaces ] );
	auto inMaterials = std::unique_ptr<material_t[]>( new material_t[ inHeader.numMaterials ] );

	file.read( (char*)inVert.get(), sizeof( vertex_t ) * inHeader.numVertices );
	file.read( (char*)inIndices.get(), sizeof( uint32_t ) * inHeader.numIndices );
	file.read( (char*)inSurfaces.get(), sizeof( surface_t ) * inHeader.numSurfaces );
	file.read( (char*)inMaterials.get(), sizeof( material_t ) * inHeader.numMaterials );

	const uint32_t baseImageId = rm.GetImageCount();

	for ( uint32_t i = 0; i < inHeader.numImages; ++i )
	{
		uint32_t width;
		uint32_t height;

		file.read( (char*)&width, sizeof( uint32_t ) );
		file.read( (char*)&height, sizeof( uint32_t ) );

		Image<Color> image = Image<Color>( width, height );
		for ( uint32_t y = 0; y < height; ++y )
		{
			for ( uint32_t x = 0; x < width; ++x )
			{
				uint32_t pixel;
				file.read( (char*)&pixel, sizeof( uint32_t ) );
				image.SetPixel( x, y, Color( pixel ) );
			}
		}

		rm.StoreImageCopy( image );

		//Bitmap bitmap = Bitmap( width, height );
		//ImageToBitmap( image, bitmap );
		//bitmap.Write( "test.bmp" );
	}

	file.close();

	uint32_t modelIx = rm.AllocModel();
	ModelSource* model = rm.GetModel( modelIx );
	
	// Vertex buffers
	const uint32_t vbOffset = rm.GetVbOffset();
	for ( uint32_t i = 0; i < inHeader.numVertices; ++i )
	{
		rm.AddVertex( inVert[ i ] );
	}
	const uint32_t vbEnd = rm.GetVbOffset();

	// Index buffers
	const uint32_t ibOffset = rm.GetIbOffset();
	for ( uint32_t i = 0; i < inHeader.numIndices; ++i )
	{
		rm.AddIndex( inIndices[ i ] + vbOffset );
	}
	const uint32_t ibEnd = rm.GetIbOffset();

	// Surfaces
	const uint32_t surfaceCount = inHeader.numSurfaces;
	model->surfs.resize( surfaceCount );
	for ( uint32_t i = 0; i < surfaceCount; ++i )
	{
		model->surfs[ i ].vb		= rm.GetVB();
		model->surfs[ i ].ib		= rm.GetIB();
		model->surfs[ i ].vbOffset	= inSurfaces[ i ].vbOffset	+ vbOffset;
		model->surfs[ i ].vbEnd		= inSurfaces[ i ].vbEnd		+ vbOffset;
		model->surfs[ i ].ibOffset	= inSurfaces[ i ].ibOffset	+ ibOffset;
		model->surfs[ i ].ibEnd		= inSurfaces[ i ].ibEnd		+ ibOffset;
		model->surfs[ i ].materialId = inSurfaces[ i ].materialId;
	}

	// Materials
	const uint32_t baseMaterialId = rm.GetMaterialCount();
	for ( uint32_t i = 0; i < inHeader.numMaterials; ++i )
	{
		material_t material = inMaterials[ i ];

		// Relative-to-Absolute IDs
		if( material.colorMapId != InvalidHdl )
		{
			material.colorMapId += baseImageId;
		}

		if ( material.normalMapId != InvalidHdl )
		{
			material.normalMapId += baseImageId;
		}

		rm.StoreMaterialCopy( material );
	}

	for ( uint32_t i = 0; i < surfaceCount; ++i )
	{
		// Relative-to-Absolute IDs
		if( model->surfs[ i ].materialId != InvalidHdl )
		{
			model->surfs[ i ].materialId += baseMaterialId;
		}
		assert( model->surfs[ i ].materialId <= rm.GetMaterialCount() );
	}

	model->name = path;

	return modelIx;
}


void StoreModelBin( const std::string& path, ResourceManager& rm, const uint32_t modelIx )
{
	ModelSource* model = rm.GetModel( modelIx );

	surface_t& surf = model->surfs[ 0 ];

	rm.PushVB( surf.vb );
	rm.PushIB( surf.ib );

	const uint32_t vertexCount = rm.GetVbOffset();
	const uint32_t indexCount = rm.GetIbOffset();
	const uint32_t imageCount = rm.GetImageCount();
	const uint32_t surfCount = static_cast<uint32_t>( model->surfs.size() );
	const uint32_t materialCount = rm.GetMaterialCount();
	const uint32_t vertexBytes = sizeof( vertex_t ) * vertexCount;
	const uint32_t indexBytes = sizeof( uint32_t ) * indexCount;
	const uint32_t surfBytes = sizeof( surface_t ) * surfCount;
	const uint32_t materialBytes = sizeof( material_t ) * materialCount;
	uint32_t imageBytes = 0;

	for( uint32_t i = 0; i < imageCount; ++i )
	{
		const Image<Color>* imageRef = rm.GetImageRef( i );
		const uint32_t size = imageRef->GetWidth() * imageRef->GetHeight();
		
		imageBytes += 2 * sizeof( uint32_t ); // Header: width, height
		imageBytes += sizeof( uint32_t ) * size;		
	}

	mdlHeader_t header;
	header.info				= 0; // version, etc
	header.vertexOffset		= sizeof( mdlHeader_t );
	header.indexOffset		= vertexBytes + header.vertexOffset;
	header.surfOffset		= indexBytes + header.indexOffset;
	header.materialOffset	= surfBytes + header.surfOffset;
	header.imageOffset		= materialBytes + header.materialOffset;
	header.numVertices		= vertexCount;
	header.numIndices		= indexCount;
	header.numSurfaces		= surfCount;
	header.numMaterials		= materialCount;
	header.numImages		= imageCount;
	
	uint32_t fileSize = sizeof( mdlHeader_t ) + header.imageOffset + imageBytes;

	auto binBucket = std::unique_ptr<uint8_t[]>( new uint8_t[ fileSize ] );

	memcpy( binBucket.get(), &header, sizeof( mdlHeader_t ) );

	// Vertices
	{
		vertex_t* vertAry = rm.GetVertex( 0 );
		memcpy( binBucket.get() + header.vertexOffset, vertAry, vertexBytes );
	}

	// Indices
	{
		for ( uint32_t i = 0; i < indexCount; ++i )
		{
			// TODO: can replace with ptr version now
			uint32_t index = rm.GetIndex( i );
			memcpy( binBucket.get() + header.indexOffset + sizeof( uint32_t ) * i, &index, sizeof( uint32_t ) );
		}
	}

	// Surfaces
	{
		memcpy( binBucket.get() + header.surfOffset, &model->surfs[ 0 ], surfBytes );
	}

	// Materials
	{
		memcpy( binBucket.get() + header.materialOffset, rm.GetMaterialRef( 0 ), materialBytes );
	}

	// Images
	{
		uint8_t* filePtr = binBucket.get() + header.imageOffset;

		for ( uint32_t i = 0; i < imageCount; ++i )
		{
			const Image<Color>* imageRef = rm.GetImageRef( i );
			const uint32_t size = imageRef->GetWidth() * imageRef->GetHeight();

			const uint32_t width = imageRef->GetWidth();
			const uint32_t height = imageRef->GetHeight();

			memcpy( filePtr,						&width, sizeof( uint32_t ) );
			memcpy( filePtr + sizeof( uint32_t ),	&height, sizeof( uint32_t ) );

			filePtr += 2 * sizeof( uint32_t );

			for ( uint32_t y = 0; y < height; ++y )
			{
				for ( uint32_t x = 0; x < width; ++x )
				{
					Color color = imageRef->GetPixel( static_cast<int32_t>( x ), static_cast<int32_t>( y ) );
					const uint32_t pixel = color.AsR8G8B8A8();
					
					memcpy( filePtr, &pixel, sizeof( uint32_t ) );
					filePtr += sizeof( uint32_t );
				}
			}
		}
	}

	std::fstream file;
	file.open( path, std::fstream::out | std::ios::binary );
	file.write( (const char*)binBucket.get(), fileSize );
	file.close();

	rm.PopVB();
	rm.PopIB();
}


void CreateRayTraceModel( ResourceManager& rm, const uint32_t modelIx, const mat4x4f& modelMatrix, const bool smoothNormals, const Color& tint, RtModel* outInstance, const matHdl_t materialId )
{
	const ModelSource* model = rm.GetModel( modelIx );

	const uint32_t surfCount = static_cast<uint32_t>( model->surfs.size() );
	const surface_t& baseSurf = model->surfs[ 0 ];

	// These are same for all surfaces
	uint32_t modelVB = baseSurf.vb;
	uint32_t modelIB = baseSurf.ib;

	uint32_t vbOffset	= baseSurf.vbOffset;
	uint32_t vbEnd		= baseSurf.vbEnd;
	uint32_t ibOffset	= baseSurf.ibOffset;
	uint32_t ibEnd		= baseSurf.ibEnd;

	for ( uint32_t surfIx = 1; surfIx < surfCount; ++surfIx )
	{
		const surface_t& surf = model->surfs[ surfIx ];
		ibOffset = std::min( ibOffset, surf.ibOffset );
		ibEnd = std::max( ibEnd, surf.ibEnd );

		assert( surf.vb == baseSurf.vb );
		assert( surf.ib == baseSurf.ib );
	}

	outInstance->transform = modelMatrix;
	const uint32_t vb = rm.AllocVB();
	outInstance->modelIx = modelIx;
	outInstance->triCache.reserve( ( ibEnd - ibOffset ) / 3 );

	rm.PushVB( modelVB );
	rm.PushIB( modelIB );

	using triIndices = std::tuple<uint32_t, uint32_t, uint32_t>;
	std::map< uint32_t, std::deque<triIndices> > vertToPolyMap;

	vec3f centroid = vec3f( 0.0f, 0.0f, 0.0f );
	for ( uint32_t i = vbOffset; i < vbEnd; ++i )
	{
		vertex_t vertex = *rm.GetVertex( i );

		vertex.pos = outInstance->transform * vec4f( Trunc<4,1>( vertex.pos ), 1.0 );
		vertex.color *= tint;

		rm.PushVB( vb );
		rm.AddVertex( vertex );
		rm.PopVB();

		centroid += Trunc<4, 1>( vertex.pos );
	}

	outInstance->centroid = centroid / (float)( vbEnd - vbOffset );

	for ( uint32_t i = ibOffset; i < ibEnd; i += 3 )
	{
		uint32_t indices[ 3 ];
		for ( uint32_t t = 0; t < 3; ++t )
		{
			indices[ t ] = rm.GetIndex( i + t ) - vbOffset;
		}

		triIndices tIndices = std::make_tuple( indices[ 0 ], indices[ 1 ], indices[ 2 ] );

		vertToPolyMap[ indices[ 0 ] ].push_back( tIndices );
		vertToPolyMap[ indices[ 1 ] ].push_back( tIndices );
		vertToPolyMap[ indices[ 2 ] ].push_back( tIndices );
	}

	rm.PopVB();
	rm.PopIB();

	rm.PushVB( vb );
	rm.PushIB( modelIB );

	if ( smoothNormals )
	{
		using vertMapIter = std::map< uint32_t, std::deque<triIndices> >::iterator;

		for ( vertMapIter iter = vertToPolyMap.begin(); iter != vertToPolyMap.end(); ++iter )
		{
			vec3f interpretedNormal = vec3f( 0.0f, 0.0f, 0.0f );
			vec3f interpretedTangent = vec3f( 0.0f, 0.0f, 0.0f );
			vec3f interpretedBitangent = vec3f( 0.0f, 0.0f, 0.0f );

			vertex_t* vertex = rm.GetVertex( iter->first );

			for ( std::deque<triIndices>::iterator polyListIter = iter->second.begin(); polyListIter != iter->second.end(); ++polyListIter )
			{
				const uint32_t i0 = std::get<0>( *polyListIter );
				const uint32_t i1 = std::get<1>( *polyListIter );
				const uint32_t i2 = std::get<2>( *polyListIter );

				// These are transformed positions; this is critical for proper normals
				const vec3f pt0 = Trunc<4, 1>( rm.GetVertex( i0 )->pos );
				const vec3f pt1 = Trunc<4, 1>( rm.GetVertex( i1 )->pos );
				const vec3f pt2 = Trunc<4, 1>( rm.GetVertex( i2 )->pos );

				const vec3f tangent = ( pt1 - pt0 ).Normalize();
				const vec3f bitangent = ( pt2 - pt0 ).Normalize();
				const vec3f normal = Cross( tangent, bitangent ).Normalize();

				interpretedNormal += normal;
				interpretedTangent += tangent;
				interpretedBitangent += bitangent;
			}

			interpretedNormal.FlushDenorms();
			interpretedTangent.FlushDenorms();
			interpretedBitangent.FlushDenorms();

			vertex->normal = interpretedNormal.Normalize();
			vertex->tangent = interpretedTangent.Normalize();
			vertex->bitangent = interpretedBitangent.Normalize();
		}
	}
	else
	{
		// TODO: fix normals on this path
	}

	for ( uint32_t surfIx = 0; surfIx < surfCount; ++surfIx )
	{
		const surface_t& surf = model->surfs[ surfIx ];

		const uint32_t triMaterialId = ( materialId == InvalidHdl ) ? surf.materialId : materialId;

		for ( uint32_t i = surf.ibOffset; i < surf.ibEnd; i += 3 )
		{
			uint32_t indices[ 3 ];
			for ( uint32_t t = 0; t < 3; ++t )
			{
				indices[ t ] = rm.GetIndex( i + t ) - surf.vbOffset;
			}

			vertex_t& v0 = *rm.GetVertex( indices[ 0 ] );
			vertex_t& v1 = *rm.GetVertex( indices[ 1 ] );
			vertex_t& v2 = *rm.GetVertex( indices[ 2 ] );

			outInstance->triCache.push_back( Triangle( v0, v1, v2, triMaterialId ) );
		}
	}

	rm.PopVB();
	rm.PopIB();

	outInstance->BuildAS();
}


uint32_t CreatePlaneModel( ResourceManager& rm, const vec2f& size, const vec2i& cellCnt, const matHdl_t materialId )
{
	uint32_t modelIx = rm.AllocModel();
	ModelSource* model = rm.GetModel( modelIx );
	model->surfs.push_back( surface_t() );

	surface_t& surf = model->surfs.back();

	std::stringstream name;
	name << "_plane" << modelIx;

	model->name = name.str();
	surf.vb = rm.GetVB();
	surf.ib = rm.GetIB();
	surf.vbOffset = rm.GetVbOffset();
	surf.ibOffset = rm.GetIbOffset();

	vec2f gridSize = Divide( size, vec2f( (float)cellCnt[ 0 ], (float)cellCnt[ 1 ] ) );

	const uint32_t verticesPerQuad = 6;

	const uint32_t firstIndex = surf.vbOffset;
	uint32_t indicesCnt = surf.ibOffset;
	uint32_t vbIx = surf.vbOffset;

	for ( int32_t j = 0; j <= cellCnt[ 1 ]; ++j )
	{
		for ( int32_t i = 0; i <= cellCnt[ 0 ]; ++i )
		{
			vertex_t v;

			v.pos = vec4f( i * gridSize[ 0 ] - 0.5f * size[ 0 ], j * gridSize[ 1 ] - 0.5f * size[ 1 ], 0.0f, 1.0f );
			v.color = Color::White;
			v.normal = vec3f( 0.0f, 0.0f, 1.0f );

			rm.AddVertex( v );
		}
	}
	surf.vbEnd = rm.GetVbOffset();

	for ( int32_t j = 0; j < cellCnt[ 1 ]; ++j )
	{
		for ( int32_t i = 0; i < cellCnt[ 0 ]; ++i )
		{
			uint32_t vIx[ 4 ];
			vIx[ 0 ] = static_cast<uint32_t>( firstIndex + ( i + 0 ) + ( j + 0 ) * ( cellCnt[ 1 ] + 1 ) );
			vIx[ 1 ] = static_cast<uint32_t>( firstIndex + ( i + 1 ) + ( j + 0 ) * ( cellCnt[ 1 ] + 1 ) );
			vIx[ 2 ] = static_cast<uint32_t>( firstIndex + ( i + 0 ) + ( j + 1 ) * ( cellCnt[ 1 ] + 1 ) );
			vIx[ 3 ] = static_cast<uint32_t>( firstIndex + ( i + 1 ) + ( j + 1 ) * ( cellCnt[ 1 ] + 1 ) );

			// Clockwise-winding
			rm.AddIndex( vIx[ 0 ] );
			rm.AddIndex( vIx[ 1 ] );
			rm.AddIndex( vIx[ 2 ] );

			rm.AddIndex( vIx[ 2 ] );
			rm.AddIndex( vIx[ 1 ] );
			rm.AddIndex( vIx[ 3 ] );
		}
	}
	surf.ibEnd = rm.GetIbOffset();

	surf.materialId = materialId;

	return modelIx;
}