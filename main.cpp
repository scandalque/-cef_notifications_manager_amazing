#include <iostream>
#include <windows.h>
#include "plugin.h"

std::unique_ptr<c_plugin> plug;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH: {
        DisableThreadLibraryCalls(hModule);

        plug = std::make_unique<c_plugin>(hModule);
        break;
    }
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}