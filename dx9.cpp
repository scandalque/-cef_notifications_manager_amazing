#include "dx9.hpp"

std::uintptr_t dx9::find_device(std::uint32_t Len) {
    static std::uintptr_t base = [](std::size_t Len) {
        std::string path_to(MAX_PATH, '\0');
        if (auto size = GetSystemDirectoryA(path_to.data(), MAX_PATH)) {
            path_to.resize(size);
            path_to += "\\d3d9.dll";
            std::uintptr_t dwObjBase = reinterpret_cast<std::uintptr_t>(LoadLibraryA(path_to.c_str()));
            while (dwObjBase++ < dwObjBase + Len) {
                if (*reinterpret_cast<std::uint16_t*>(dwObjBase + 0x00) == 0x06C7 &&
                    *reinterpret_cast<std::uint16_t*>(dwObjBase + 0x06) == 0x8689 &&
                    *reinterpret_cast<std::uint16_t*>(dwObjBase + 0x0C) == 0x8689) {
                    dwObjBase += 2;
                    break;
                }
            }
            return dwObjBase;
        }
        return std::uintptr_t(0);
        }(Len);
    return base;
}

void* dx9::get_function_address(int VTableIndex) {
    return (*reinterpret_cast<void***>(find_device(0x128000)))[VTableIndex];
}