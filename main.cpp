#ifdef _MSC_VER
    #define _CRTDBG_MAP_ALLOC
    #include <stdio.h>
    #include <stdlib.h>
    #include <crtdbg.h>
#endif

#include "Photon/Engine.h"

#ifdef _DEBUG
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#define new DBG_NEW
#endif

int main() {
#ifdef _MSC_VER
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
    Engine engine;
    engine.run();
#ifdef _MSC_VER
    _CrtDumpMemoryLeaks();
#endif
    return 0;
}