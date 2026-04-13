#include <iostream>
#include <vector>
#include <map>
#include <thread>
#include <chrono>
#include <mutex>
#include "gfxcore/core/common.h"

void RunMatrixTests();
void RunVectorTests();

int main()
{
	RunVectorTests();
	RunMatrixTests();
}
