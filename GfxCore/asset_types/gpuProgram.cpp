/*
* MIT License
*
* Copyright( c ) 2021-2023 Thomas Griebel
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

#include "gpuProgram.h"

std::string GpuProgramLoader::GetBinName( const std::string& fileName, const shaderPermId_t permSet )
{
	std::string name;
	std::string baseName;
	std::string ext;
	std::string ext2;
	SplitFileName( fileName, baseName, ext2 );

	if( ext2 == "hlsl" )
	{
		SplitFileName( baseName, name, ext );
	}
	else
	{
		name = baseName;
		ext = ext2;
	}

	if ( ext == "vert" || ext == "vs" ) {
		name += "VS";
	}
	else if ( ext == "frag" || ext == "ps" ) {
		name += "PS";
	}
	else if ( ext == "comp" || ext == "cs" ) {
		name += "CS";
	}

	for ( uint32_t i = 0; i < shaderPermId_t::COUNT; ++i )
	{
		const uint32_t permBit = static_cast<uint32_t>( permSet ) & ( 1u << i );
		if ( permBit == 0 ) {
			continue;
		}
		const shaderPermId_t permId = static_cast<shaderPermId_t>( permBit );

		const shaderPerm_t* perm = FindPerm( permId );
		if ( perm != nullptr ) {
			name += "_" + perm->tag;
		}
	}

	name += ".spv";

	return name;
}


std::string GpuProgramLoader::GetCompileString( const std::string& srcPath, const std::string& binPath, const std::string& perms )
{
	// Extract just the filename from srcPath (e.g. "resolve.ps.hlsl")
	std::string filename = srcPath;
	const size_t lastSlash = srcPath.find_last_of( "\\/" );
	if ( lastSlash != std::string::npos ) {
		filename = srcPath.substr( lastSlash + 1 );
	}

	std::string cmd = "python " + compilerPath + "shader_compiler.py";

	// Perms are comma-delimited (e.g. -p msaa,skycube)
	if ( !perms.empty() ) {
		cmd += " -p " + perms;
	}

	cmd += " " + filename;

	return cmd;
}


void GpuProgramLoader::CheckCompileShader( const std::string& path, const std::string& binPath, const shaderPermId_t permSet, const bool forceRebuild )
{
	if ( FileExists( binPath ) == false || forceRebuild )
	{
		std::string perms = "";

		for ( uint32_t i = 0; i < shaderPermId_t::COUNT; ++i )
		{
			const uint32_t permBit = static_cast<uint32_t>( permSet ) & ( 1u << i );
			if( permBit == 0 ) {
				continue;
			}
			const shaderPermId_t permId = static_cast<shaderPermId_t>( permBit );

			const shaderPerm_t* shaderPerm = FindPerm( permId );
			if ( shaderPerm != nullptr ) {
				if ( !perms.empty() ) {
					perms += ",";
				}
				perms += shaderPerm->tag;
			}
		}

		std::string compileCommand = GetCompileString( path, binPath, perms );
		system( compileCommand.c_str() );
	}
}


bool GpuProgramLoader::LoadRasterProgram( GpuProgram& program )
{
	assert( permIdCount >= 1 );

	program.type = pipelineType_t::RASTER;
	program.shaderCount = 2;
	program.bindsetCount = 0;
	program.permSet = shaderPermId_t::NONE;

	shaderPermId_t permPowerSet[ GpuProgram::MaxPermutations ]; // Array of all set combinations
	uint32_t permSetCount = 1; // Always 1+ b/c of "NONE"

	for ( uint32_t permIndex = 0; permIndex < GpuProgram::MaxPermutations; ++permIndex )
	{
		permPowerSet[ permIndex ] = shaderPermId_t::NONE;
	}

	for ( uint32_t permIndex = 0; permIndex < permIdCount; ++permIndex )
	{
		shaderPermId_t permId = permList[ permIndex ];
		if ( permId == shaderPermId_t::NONE ) {
			continue;
		}
		program.permSet |= permId;

		permPowerSet[ permSetCount ] = program.permSet;
		++permSetCount;
	}

	program.permCount = permSetCount;

	for ( uint32_t permSetIndex = 0; permSetIndex < permSetCount; ++permSetIndex )
	{
		const std::string vsBinName = GetBinName( vsFileName, permPowerSet[ permSetIndex ] );
		const std::string psBinName = GetBinName( psFileName, permPowerSet[ permSetIndex ] );

		CheckCompileShader( srcPath + vsFileName, binPath + vsBinName, permPowerSet[ permSetIndex ], HasFlags( LOAD_HANDLER_FLAGS_REBAKE ) );
		CheckCompileShader( srcPath + psFileName, binPath + psBinName, permPowerSet[ permSetIndex ], HasFlags( LOAD_HANDLER_FLAGS_REBAKE ) );

		shaderSource_t& vs = program.shaders[ 0 ];
		shaderBin_t& vsBin = program.shaderBins[ permSetIndex ][ 0 ];

		vs.name = vsFileName;
		vs.src = ReadTextFile( srcPath + vsFileName );
		vs.type = shaderType_t::VERTEX;

		vsBin.binName = vsBinName;
		vsBin.blob = ReadBinaryFile( binPath + vsBinName );
		vsBin.type = shaderType_t::VERTEX;

		shaderSource_t& ps = program.shaders[ 1 ];
		shaderBin_t& psBin = program.shaderBins[ permSetIndex ][ 1 ];

		ps.name = psFileName;
		ps.src = ReadTextFile( srcPath + psFileName );
		ps.type = shaderType_t::PIXEL;

		psBin.binName = psBinName;
		psBin.blob = ReadBinaryFile( binPath + psBinName );
		psBin.type = shaderType_t::PIXEL;
	}

	return true;
}


bool GpuProgramLoader::LoadComputeProgram( GpuProgram& program )
{
	program.type = pipelineType_t::COMPUTE;
	program.shaderCount = 1;
	program.bindsetCount = 0;
	program.permCount = permIdCount;
	program.permSet = shaderPermId_t::NONE;

	const std::string csBinName = GetBinName( csFileName, shaderPermId_t::NONE );

	CheckCompileShader( srcPath + csFileName, binPath + csBinName, shaderPermId_t::NONE, HasFlags( LOAD_HANDLER_FLAGS_REBAKE ) );

	shaderSource_t& cs = program.shaders[ 0 ];
	shaderBin_t& csBin = program.shaderBins[ 0 ][ 0 ];

	cs.name = csFileName;
	cs.src = ReadTextFile( srcPath + csFileName );
	cs.type = shaderType_t::COMPUTE;

	csBin.binName = csBinName;
	csBin.blob = ReadBinaryFile( binPath + csBinName );
	csBin.type = shaderType_t::COMPUTE;

	return true;
}


bool GpuProgramLoader::Load( Asset<GpuProgram>& programAsset )
{
	GpuProgram& program = programAsset.Get();

	program.bindHash = bindHash;
	program.flags = flags;

	if ( ( !vsFileName.empty() ) && ( !psFileName.empty() ) )
	{
		return LoadRasterProgram( program );
	}
	else if ( !csFileName.empty() )
	{
		return LoadComputeProgram( program );
	}
	return false;
}


void GpuProgramLoader::SetSourcePath( const std::string& path )
{
	srcPath = path;
}


void GpuProgramLoader::SetBinPath( const std::string& path )
{
	binPath = path;
}


void GpuProgramLoader::SetBindSet( const std::string& setName )
{
	bindHash = Hash( setName );
}


void GpuProgramLoader::AddPerm( const std::string& permName )
{
	if ( permIdCount > static_cast<uint32_t>( shaderPermId_t::COUNT ) ) {
		return;
	}

	shaderPermId_t permId = GetPermId( permName );

	if ( permId == shaderPermId_t::NONE ) {
		return;
	}

	permList[ permIdCount ] = permId;
	++permIdCount;
}


void GpuProgramLoader::SetFlags( const shaderFlags_t shaderFlags )
{
	flags = shaderFlags;
}


void GpuProgramLoader::SetCompilerPath( const std::string& path )
{
	compilerPath = path;
}


void GpuProgramLoader::AddFilePaths( const std::string& vertexFileName, const std::string& pixelFileName, const std::string& computeFileName )
{
	if ( !vertexFileName.empty() ) {
		vsFileName = vertexFileName + ".vs.hlsl";
	}
	if ( !pixelFileName.empty() ) {
		psFileName = pixelFileName + ".ps.hlsl";
	}
	if ( !computeFileName.empty() ) {
		csFileName = computeFileName + ".cs.hlsl";
	}
}