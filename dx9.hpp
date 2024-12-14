#pragma once

#include <iostream>
#include <windows.h>

namespace dx9 {
	std::uintptr_t find_device(std::uint32_t Len);
	void* get_function_address(int VTableIndex);
};