#ifndef plugin_utils
#define plugin_utils

#include <iostream>
#include <windows.h>
#include "d3d9.h"

#include <RakNet/BitStream.h>

#include <regex>

namespace utils {

    template <int number> inline std::string find_text_by_pattern(std::string text, std::string pattern) {
        std::regex _pattern(pattern);
        std::smatch _match;
        std::regex_search(text, _match, _pattern);
        std::string _return = _match.str(number);

        return _return;
    }

    inline std::string read_all_bitstream_as_text(RakNet::BitStream* bs) {
        int offset = bs->GetReadOffset();

        bs->ResetReadPointer();

        std::vector<uint8_t> bytes;
        for (int i = 0; i < bs->GetNumberOfBytesUsed(); i++) {
            uint8_t b;
            bs->Read(b);
            bytes.push_back(b);
        }

        std::string _return(bytes.begin(), bytes.end());

        bs->SetReadOffset(offset);
        return _return;
    }

    inline bool cursor = false;
    inline void show_cursor(bool state)
    {
        using RwD3D9GetCurrentD3DDevice_t = LPDIRECT3DDEVICE9(__cdecl*)();

        auto rwCurrentD3dDevice{ reinterpret_cast<
            RwD3D9GetCurrentD3DDevice_t>(0x7F9D50U)() };

        if (nullptr == rwCurrentD3dDevice) {
            return;
        }

        static DWORD
            updateMouseProtection,
            rsMouseSetPosProtFirst,
            rsMouseSetPosProtSecond;

        if (state)
        {
            ::VirtualProtect(reinterpret_cast<void*>(0x53F3C6U), 5U,
                PAGE_EXECUTE_READWRITE, &updateMouseProtection);

            ::VirtualProtect(reinterpret_cast<void*>(0x53E9F1U), 5U,
                PAGE_EXECUTE_READWRITE, &rsMouseSetPosProtFirst);

            ::VirtualProtect(reinterpret_cast<void*>(0x748A1BU), 5U,
                PAGE_EXECUTE_READWRITE, &rsMouseSetPosProtSecond);

            // NOP: CPad::UpdateMouse
            *reinterpret_cast<uint8_t*>(0x53F3C6U) = 0xE9U;
            *reinterpret_cast<uint32_t*>(0x53F3C6U + 1U) = 0x15BU;

            // NOP: RsMouseSetPos
            memset(reinterpret_cast<void*>(0x53E9F1U), 0x90, 5U);
            memset(reinterpret_cast<void*>(0x748A1BU), 0x90, 5U);

            rwCurrentD3dDevice->ShowCursor(TRUE);
        }
        else
        {
            // Original: CPad::UpdateMouse
            memcpy(reinterpret_cast<void*>(0x53F3C6U), "\xE8\x95\x6C\x20\x00", 5U);

            // Original: RsMouseSetPos
            memcpy(reinterpret_cast<void*>(0x53E9F1U), "\xE8\xAA\xAA\x0D\x00", 5U);
            memcpy(reinterpret_cast<void*>(0x748A1BU), "\xE8\x80\x0A\xED\xFF", 5U);

            using CPad_ClearMouseHistory_t = void(__cdecl*)();
            using CPad_UpdatePads_t = void(__cdecl*)();

            reinterpret_cast<CPad_ClearMouseHistory_t>(0x541BD0U)();
            reinterpret_cast<CPad_UpdatePads_t>(0x541DD0U)();

            ::VirtualProtect(reinterpret_cast<void*>(0x53F3C6U), 5U,
                updateMouseProtection, &updateMouseProtection);

            ::VirtualProtect(reinterpret_cast<void*>(0x53E9F1U), 5U,
                rsMouseSetPosProtFirst, &rsMouseSetPosProtFirst);

            ::VirtualProtect(reinterpret_cast<void*>(0x748A1BU), 5U,
                rsMouseSetPosProtSecond, &rsMouseSetPosProtSecond);

            rwCurrentD3dDevice->ShowCursor(FALSE);
        }
        cursor = state;
    }

    inline bool string_to_bool(std::string str) {
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
        std::istringstream is(str);
        bool b;
        is >> std::boolalpha >> b;
        return b;
    }

    inline std::string const bool_to_string(bool b)
    {
        return b ? "true" : "false";
    }

}
#endif