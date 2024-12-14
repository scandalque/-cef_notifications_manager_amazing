#ifndef plugin_main
#define plugin_main

#include <iostream>
#include <windows.h>

#include "kthook/kthook.hpp"
#include <RakHook/rakhook.hpp>
#include <RakNet/PacketEnumerations.h>
#include <RakNet/StringCompressor.h>
#include <RakHook/detail.hpp>

#include "d3d9.h"
#include "dx9.hpp"
#include "config.h"
#include <filesystem>

class c_plugin
{
public:
	c_plugin(HMODULE);
	~c_plugin();

	using present_t = HRESULT(__stdcall*)(IDirect3DDevice9*, const RECT*, const RECT*, HWND, const RGNDATA*);
	using reset_t = HRESULT(__stdcall*)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);

	kthook::kthook_simple<WNDPROC> wndproc_hook{};
	kthook::kthook_signal<present_t> present_hook{ dx9::get_function_address(17) };
	kthook::kthook_signal<reset_t> reset_hook{ dx9::get_function_address(16) };
	kthook::kthook_simple<void(__cdecl*)()> update_hook{ reinterpret_cast<void*>(0x561B10) };

	std::optional<HRESULT> CALLBACK present(const decltype(present_hook)& hook, IDirect3DDevice9*, const RECT*, const RECT*, HWND, const RGNDATA*);
	std::optional<HRESULT> CALLBACK lost(const decltype(reset_hook)& hook, IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);
	LRESULT CALLBACK wndproc(const decltype(wndproc_hook)& hook, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	HWND get_game_window_handle() {
		return **reinterpret_cast<HWND**>(0xC17054);
	}

	bool receive_rpc(unsigned char& id, RakNet::BitStream* bs);

	std::unique_ptr<c_config> config_p;

private:
	HMODULE module;

};

#endif