
#include "../gfxcore/math/matrix.h"
#include "../gfxcore/math/matrixSolvers.h"
#include "../gfxcore/core/util.h"

struct testContext_t
{
	uint32_t testNum;
	uint32_t testCount;
	uint32_t testsPassed;
};


class TestKernel
{
public:
	virtual bool Run( testContext_t& context ) = 0;
	virtual const char* Name() = 0;
};


class QRDecompositionTest final : public TestKernel
{
public:
	const char* Name() override
	{
		return "Matrix QR";
	}

	bool Run( testContext_t& context ) override
	{
		float errorTol = 0.0001f;

		mat4x4f A = RandomSolveableMatrix();

		mat4x4f Q, R;
		QR( A, Q, R );

		const float error = fabs( GrandSum( Q * R - A ) / 16.0f );

		if ( ( error <= errorTol ) && IsOrthonormal( Q, errorTol ) ) {
			return true;
		}
		return false;
	}
};


class LUPDecompositionTest final : public TestKernel
{
private:

	bool ScanVectorsLUSolver( const mat4x4f& A, const float tolerance, const float delta )
	{
		float length = 1.0f;

		uint32_t testsPasses = 0;
		uint32_t testCount = 0;

		mat4x4f LU = A;
		LUPSolver<4, float> solver( A, LU, tolerance );

		float v = delta;

		while ( v <= ( 1.0f - delta ) )
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
				}
				++testCount;
				u += delta;
			}
			v += delta;
		}
		return ( testsPasses == testCount );
	}

	bool RandomVectorLUSolver( const mat4x4f& A, const float tolerance, const uint32_t testCount )
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

			if ( error <= tolerance ) {
				++testsPasses;
			}
		}
		return ( testsPasses == testCount );
	}

public:
	const char* Name() override
	{
		return "LUP Solver";
	}

	bool Run( testContext_t& context ) override
	{
		float errorTol = 0.0001f;

		mat4x4f A = RandomSolveableMatrix();

		mat4x4f LU = A;

		LUPSolver<4, float> solver( A, LU, errorTol );

		const mat4x4f& inverseFromLU = solver.Inverse();

		const bool identiyCheck = IsIdentity( A * inverseFromLU, 0.00001f );

		if ( identiyCheck == false || solver.Determinant() == 0.0f )
		{
			std::cout << inverseFromLU << " " << solver.Determinant() << std::endl;
			return false;
		}

		bool ret = true;
		ret = ret && RandomVectorLUSolver( A, errorTol, 1000 );
		ret = ret && ScanVectorsLUSolver( A, errorTol, 0.01f );

		return ret;
	}
};


class MatrixIdentityTest final : public TestKernel
{
private:

	mat4x4f I;
	float errorTol = 0.00001f;

public:

	MatrixIdentityTest()
	{
		I = mat4x4f::Identity();
	}

	const char* Name() override
	{
		return "Matrix Identity";
	}

	bool Run( testContext_t& context ) override
	{
		vec4f x( RandomVector( 1.0f ), 1.0f );

		vec4f b = I * x;

		const float error = ( x - b ).Length();

		if ( error <= errorTol ) {
			return true;
		}
		return false;
	}
};


class MatrixScaleTest final : public TestKernel
{
private:

	mat4x4f I;
	float errorTol = 0.00001f;

public:

	MatrixScaleTest()
	{
		mat4x4f I = mat4x4f::Identity();
	}

	const char* Name() override
	{
		return "Matrix Scale";
	}

	bool Run( testContext_t& context ) override
	{
		const float s = static_cast<float>( context.testNum ) - ( context.testCount / 2.0f );
		const mat4x4f A = s * mat4x4f::Identity();

		const vec4f x( RandomVector( 1.0f ), 1.0f );

		vec4f b = A * x;

		const float error = ( s * x - b ).Length();

		if ( error <= errorTol ) {
			return true;
		}
		return false;
	}
};


class MatrixMultiplyTest final : public TestKernel
{
private:

	mat4x4f I;
	float errorTol = 0.0001f;

public:

	MatrixMultiplyTest()
	{
		mat4x4f I = mat4x4f::Identity();
	}

	const char* Name() override
	{
		return "Matrix Multiply";
	}

	bool Run( testContext_t& context ) override
	{
		const float percentDone = context.testNum / static_cast<float>( context.testCount );
		const float degrees = Degrees( 2.0f * percentDone * PI );
		const mat4x4f A = ComputeRotationX( degrees );

		const vec4f x( 0.0f, 1.0f, 0.0f, 0.0f );

		vec4f b = A * x;

		const float angle = Angle( x, b );
		const float adjustedDegrees = ( degrees > 180.0f ) ? ( 360.0f - degrees ) : degrees;

		const float error = fabs( angle - adjustedDegrees );

		if ( error <= errorTol ) {
			return true;
		}
		return false;
	}
};


void RunHarness( TestKernel* test, const uint32_t testCount )
{
	testContext_t context;

	context.testNum = 0;
	context.testsPassed = 0;
	context.testCount = testCount;

	while ( context.testNum < context.testCount )
	{
		context.testsPassed += test->Run( context ) ? 1 : 0;
		++context.testNum;
	}

	if ( context.testsPassed != context.testCount ) {
		std::cout << test->Name() << ": " << context.testsPassed << " tests passed out of " << context.testCount << std::endl;
	}

	delete test;
}


//template <size_t D, typename T, typename S>
//class Vector
//{
//public:
//
//	union
//	{
//		T elements[ D ];
//	};
//
//	VECTOR_COMMON( D, T, S )
//};

void RunMatrixTests()
{
	mat4x4f A( 1.0f );
	mat4x4f B( 2.0f );
	Flush( A, 0.0001f );

	mat4x4f C = A * 10.0f;
	C = C / 2.0f;
	C = 2.0f * C;

	mat4x4f S0 = CreateMatrix4x4( 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f );

 	MatrixV< 4, 4, 3, 3, float > matrixView( S0, 1, 1 );

	mat3x3f S1 = CreateMatrix3x3( 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f );
	MatrixV< 3, 3, 2, 2, float > cof00( S1, 0, 0 );

	std::cout << Det( cof00 ) << std::endl;

	std::cout << C << std::endl;
	std::cout << S0 << std::endl;
	std::cout << matrixView << std::endl;

	//MatrixView< 4, 4, 3, 3, float> matrixView( A, 0, 0 );

	//matrixView = matrixView * matrixView;

	RunHarness( new MatrixIdentityTest(), 100 );
	RunHarness( new MatrixScaleTest(), 100 );
	RunHarness( new MatrixMultiplyTest(), 100 );
	RunHarness( new QRDecompositionTest(), 100 );
	RunHarness( new LUPDecompositionTest(), 100 );
}


void RunVectorTests()
{
	vec4f v0( 1.0f, 2.0f, 3.0f, 4.0f );
	vec4f v1( 5.0f, 6.0f, 7.0f, 8.0f );

	std::cout << ( v0 + v1 ) << std::endl;
}