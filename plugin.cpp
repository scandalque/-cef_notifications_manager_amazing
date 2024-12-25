#include "plugin.h"
#include "utils.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_win32.h"

#include "samp/samp.h"

using namespace std::placeholders;
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool menu = false;
#ifdef DEBUG_CFG
void attach_console() {
    if (!AllocConsole())
        return;

    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    freopen_s(&f, "CONOUT$", "w", stderr);
    freopen_s(&f, "CONIN$", "r", stdin);

    SetConsoleOutputCP(1251);
}
#endif


HRESULT __stdcall c_plugin::wndproc(const decltype(wndproc_hook)& hook, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_CHAR) {
        wchar_t wch;
        MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, reinterpret_cast<char*>(&wParam), 1, &wch, 1);
        wParam = wch;
    }
    if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
        return true;

    return hook.get_trampoline()(hwnd, uMsg, wParam, lParam);
}

bool c_plugin::receive_rpc(unsigned char& id, RakNet::BitStream* bs) {
    if (id == 252) {
        bs->IgnoreBits(40);
        std::string cef_packet_name = rakhook::detail::read_with_size<uint16_t>(bs);
        if (cef_packet_name == "cef_notifications_show") {
            std::string packet_data = utils::read_all_bitstream_as_text(bs);

            std::string message_type = utils::find_text_by_pattern<2>(packet_data, "(messageType.)(.*?)(.text)");
            std::string view_type = utils::find_text_by_pattern<2>(packet_data, "(viewType.)(.*?)$");
#ifdef DEBUG_CFG
            std::cout << view_type << ": " << message_type << std::endl;
#endif
            if (config_p->config.find({ view_type, message_type }) == config_p->config.end()) {
                c_chat::get()->ref()->add_message(0xFFDEADFF, std::string("[cef notifications] {ffffff}unknown key. view type: " + view_type + ". message type: " + message_type));
            }
            else return !config_p->config[{view_type, message_type}];
        }
    }
    return true;
}

bool push_hint = true;

std::optional<HRESULT> CALLBACK c_plugin::present(const decltype(present_hook)& hook, IDirect3DDevice9* device_ptr, const RECT* source_ptr, const RECT* dest_ptr, HWND dest, const RGNDATA* dr_ptr) {
    static bool ImGui_inited = false;
    static bool inited = false;
    if (!ImGui_inited) {
        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        ImGui_ImplWin32_Init(get_game_window_handle());
        ImGui_ImplDX9_Init(device_ptr);
        ImGui::GetIO().IniFilename = nullptr;
        ImFontConfig font_config;
        ImGui::GetIO().Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\trebucbd.ttf", 16.0f, &font_config, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());

        auto latest_wndproc_ptr = GetWindowLongPtrA(get_game_window_handle(), GWLP_WNDPROC);
        wndproc_hook.set_dest(latest_wndproc_ptr);
        wndproc_hook.set_cb(std::bind(&c_plugin::wndproc, this, _1, _2, _3, _4, _5));
        wndproc_hook.install();

        ImGui_inited = true;

        ImGui::StyleColorsLight();
    }

    if (!inited && rakhook::initialize() && c_input::get()->ref() != nullptr) {
        inited = true;
        StringCompressor::AddReference();

        rakhook::on_receive_rpc += std::bind(&c_plugin::receive_rpc, this, _1, _2);

        c_input::get()->ref()->add_command("notif", [](const char* p) {
            menu ^= true;
            utils::show_cursor(menu);
            });
    }

    if (ImGui::GetCurrentContext()) {
        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();

        ImGui::NewFrame();
        if (menu) {
            utils::show_cursor(true);

            ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x / 2, ImGui::GetIO().DisplaySize.y / 2), ImGuiCond_FirstUseEver, ImVec2(0.5, 0.5));
            ImGui::SetNextWindowSize(ImVec2(335, 400));
            ImGui::Begin("cef notifications manager for amazing rp", &menu, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

            std::string current_first_key = "";

            for (const auto& _config : config_p->config) {
                if (_config.first.first != current_first_key) {
                    current_first_key = _config.first.first;
                    ImGui::Text(current_first_key.c_str());
                }
                std::string name = _config.first.second + "##" + _config.first.first + "_" + _config.first.second;
                if (ImGui::Checkbox((name.c_str()), &config_p->config[{_config.first.first, _config.first.second}])) {
                    config_p->save_settings_to_ini();
                }
            }
            ImGui::SetCursorPos(ImVec2(295, 375));

            ImGui::TextDisabled("info");
            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::PushTextWrapPos(400);
                ImGui::TextUnformatted("Author: scandalque");
                ImGui::PopTextWrapPos();
                ImGui::EndTooltip();
            }

            ImGui::End();
        }
        else if (!menu && utils::cursor) {
            utils::show_cursor(false);
        }
        ImGui::EndFrame();

        ImGui::Render();
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
    }
    return std::nullopt;
}

std::optional<HRESULT> CALLBACK c_plugin::lost(const decltype(reset_hook)& hook, IDirect3DDevice9*, D3DPRESENT_PARAMETERS*) {
    ImGui_ImplDX9_InvalidateDeviceObjects();
    return std::nullopt;
}

c_plugin::c_plugin(HMODULE module) : module(module) {
#ifdef DEBUG_CFG
    attach_console();
#endif
    char mod_path[MAX_PATH] = { 0 };
    GetModuleFileNameA(module, mod_path, MAX_PATH);

    config_p = std::make_unique<c_config>(std::filesystem::path(mod_path).replace_extension("ini").filename().string());

    config_p->load_settings();

    present_hook.before += std::bind(&c_plugin::present, this, _1, _2, _3, _4, _5, _6);
    reset_hook.before += std::bind(&c_plugin::lost, this, _1, _2, _3);
}

c_plugin::~c_plugin()
{
    config_p->save_settings_to_ini();
    rakhook::destroy();
}