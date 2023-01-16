#include <iostream>
#include <vector>
#include <map>
#include <thread>
#include <chrono>
#include <mutex>
#include "src/core/common.h"
#include "src/core/assetLib.h"

int main()
{
    AssetLib<int> lib;
    //lib.Add( "One", 1 );
    //lib.Add( "Two", 2 );
    std::cout << "Lib count:" << lib.Count() << std::endl;
}