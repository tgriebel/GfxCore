
#include "../gfxcore/math/matrix.h"
#include "../gfxcore/math/matrixSolvers.h"
#include "../gfxcore/core/util.h"

void ScanVectorsLUSolver( const mat4x4f& A, const float tolerance, const float delta )
{
	float length = 1.0f;

	uint32_t testsPasses = 0;
	uint32_t testCount = 0;

	mat4x4f LU = A;
	LUPSolver<4, float> solver( A, LU, tolerance );

	float v = delta;

	while( v <= ( 1.0f - delta ) )
	{
		float u = 0.0f;
		while ( u <= 1.0f )
		{
			vec4f x( RandomVector( length ), 1.0f );

			vec4f b = A * x;

			vec4f x2;
			bool ret = solver.Solve( b, x2 );
			if ( !ret )
			{
				std::cout << "Solve Failed" << std::endl;
				continue;
			}

			const float error = ( ( A * x2 ) - b ).Length();

			if ( error < tolerance ) {
				++testsPasses;
			} else {
				std::cout << "LU Solve with Unit Vectors Failed: " << x << " " << x2 << " (" << error << " )" << std::endl;
			}
			++testCount;
			u += delta;
		}	
		v += delta;
	}
	if ( testsPasses != testCount ) {
		std::cout << "LU Solve with Unit Vectors: " << testsPasses << " tests passed out of " << testCount << std::endl;
	}
}

void RandomVectorLUSolver( const mat4x4f& A, const float tolerance, const uint32_t testCount )
{
	uint32_t testsPasses = 0;

	mat4x4f LU = A;

	LUPSolver<4, float> solver( A, LU, tolerance );

	for ( uint32_t t = 0; t < testCount; ++t )
	{
		vec4f x( RandomVector( 1.0f ), 1.0f );

		vec4f b = A * x;

		vec4f x2;
		solver.Solve( b, x2 );

		const float error = ( ( A * x2 ) - b ).Length();

		if ( error > tolerance ) {
			std::cout << "X: " << x << ", Solved X: " << x2 << std::endl;
		} else {
			++testsPasses;
		}
	}

	if( testsPasses != testCount ) {
		std::cout << "LU Solve with Random Unit Vectors: " << testsPasses << " tests passed out of " << testCount << std::endl;
	}
}


mat4x4f RandomMatrix()
{
	mat4x4f M;

	for ( uint32_t r = 0; r < 4; ++r ) {
		for ( uint32_t c = 0; c < 4; ++c ) {
			M[ r ][ c ] = Random();
		}
	}

	float sum = 0.0f;
	for ( uint32_t c = 0; c < 4; ++c ) {
		sum += fabs( M[ 0 ][ c ] );
	}

	for ( uint32_t c = 0; c < 4; ++c ) {
		M[ c ][ c ] += sum;
	}

	return M;
}


void RunMatrixSolverTests()
{
	float errorTol = 0.00001f;

	uint32_t testsPasses = 0;
	uint32_t testCount = 1000;

	uint32_t testNumber = 0;

	while( testNumber < testCount )
	{
		mat4x4f A = RandomMatrix();

		bool invertible;
		mat4x4f IA = m_LUI( A, invertible );

		mat4x4f I = A * IA;
		Flush( I, errorTol );

		if( !( invertible && IsIdentity( I, errorTol ) ) ) {
			continue;
		}

		mat4x4f LU = A;

		LUPSolver<4, float> solver( A, LU, errorTol );

		const mat4x4f& inverseFromLU = solver.Inverse();

		const bool identiyCheck = IsIdentity( A * inverseFromLU, 0.00001f );
		
		if ( identiyCheck == false || solver.Determinant() == 0.0f ) {
			std::cout << inverseFromLU << " " << solver.Determinant() << std::endl;
		}

		//A = mat4x4f( 3.0f );

		RandomVectorLUSolver( A, errorTol, 1000 );
		ScanVectorsLUSolver( A, errorTol, 0.01f );

		++testNumber;
	}
}