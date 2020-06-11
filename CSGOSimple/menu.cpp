#include "Menu.hpp"
#define NOMINMAX
#include <Windows.h>
#include <chrono>

#include "valve_sdk/csgostructs.hpp"
#include "helpers/input.hpp"
#include "options.hpp"
#include "ui.hpp"
#include "config.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui_internal.h"
#include "imgui/impl/imgui_impl_dx9.h"
#include "imgui/impl/imgui_impl_win32.h"


// =========================================================
// 
// These are the tabs on the sidebar
// 
// =========================================================
static char* sidebar_tabs[] = {
    "ESP",
    "AIM",
    "MISC",
    "CONFIG"
};

constexpr static float get_sidebar_item_width() { return 150.0f; }
constexpr static float get_sidebar_item_height() { return  50.0f; }

enum {
	TAB_ESP,
	TAB_AIMBOT,
	TAB_MISC,
	TAB_CONFIG
};

namespace ImGuiEx
{
    inline bool ColorEdit4(const char* label, Color* v, ImGuiColorEditFlags flags)
    {
        auto clr = ImVec4{
            v->r() / 255.0f,
            v->g() / 255.0f,
            v->b() / 255.0f,
            v->a() / 255.0f
        };
        if (ImGui::ColorEdit4(label, &clr.x, ImGuiColorEditFlags_NoInputs)) {
            v->SetColor(clr.x, clr.y, clr.z, clr.w);
            return true;
        }
        return false;
    }
    inline bool ColorEdit3(const char* label, Color* v)
    {
        return ColorEdit4(label, v, ImGuiColorEditFlags_NoInputs);
    }
}


template<size_t N>
void render_tabs(char* (&names)[N], int& activetab, float w, float h, bool sameline)
{
    bool values[N] = { false };

    values[activetab] = true;

    for(auto i = 0; i < N; ++i) {
        if(ImGui::ToggleButton(names[i], &values[i], ImVec2{ w, h })) {
            activetab = i;
        }
        if(sameline && i < N - 1)
            ImGui::SameLine();
    }
}

ImVec2 get_sidebar_size()
{
    constexpr float padding = 10.0f;
    constexpr auto size_w = padding * 2.0f + get_sidebar_item_width();
    constexpr auto size_h = padding * 2.0f + (sizeof(sidebar_tabs) / sizeof(char*)) * get_sidebar_item_height();

    return ImVec2{ size_w, ImMax(325.0f, size_h) };
}

int get_fps()
{
    using namespace std::chrono;
    static int count = 0;
    static auto last = high_resolution_clock::now();
    auto now = high_resolution_clock::now();
    static int fps = 0;

    count++;

    if(duration_cast<milliseconds>(now - last).count() > 1000) {
        fps = count;
        count = 0;
        last = now;
    }

    return fps;
}

//void RenderEspTab()
//{
//    static char* esp_tab_names[] = { "ESP", "GLOW", "CHAMS" };
//    static int   active_esp_tab = 0;
//
//    bool placeholder_true = true;
//
//    auto& style = ImGui::GetStyle();
//    float group_w = ImGui::GetCurrentWindow()->Size.x - style.WindowPadding.x * 2;
//    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
//    render_tabs(esp_tab_names, active_esp_tab, group_w / _countof(esp_tab_names), 25.0f, true);
//    ImGui::PopStyleVar();
//
//    ImGui::BeginGroupBox("##body_content");
//    {
//        if(active_esp_tab == 0) {
//            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ style.WindowPadding.x, style.ItemSpacing.y });
//            ImGui::Columns(3, nullptr, false);
//            ImGui::SetColumnOffset(1, group_w / 3.0f);
//            ImGui::SetColumnOffset(2, 2 * group_w / 3.0f);
//            ImGui::SetColumnOffset(3, group_w);
//
//            ImGui::Checkbox("Enabled", g_Options.esp_enabled);
//            ImGui::Checkbox("Team check", g_Options.esp_enemies_only);
//            ImGui::Checkbox("Boxes", g_Options.esp_player_boxes);
//            ImGui::Checkbox("Names", g_Options.esp_player_names);
//            ImGui::Checkbox("Health", g_Options.esp_player_health);
//            ImGui::Checkbox("Armour", g_Options.esp_player_armour);
//            ImGui::Checkbox("Weapon", g_Options.esp_player_weapons);
//            ImGui::Checkbox("Snaplines", g_Options.esp_player_snaplines);
//
//            ImGui::NextColumn();
//
//            ImGui::Checkbox("Dropped Weapons", g_Options.esp_dropped_weapons);
//            ImGui::Checkbox("Defuse Kit", g_Options.esp_defuse_kit);
//            ImGui::Checkbox("Planted C4", g_Options.esp_planted_c4);
//			ImGui::Checkbox("Item Esp", g_Options.esp_items);
//
//            ImGui::NextColumn();
//
//            ImGui::PushItemWidth(100);
//            ImGuiEx::ColorEdit3("Allies Visible", g_Options.color_esp_ally_visible);
//            ImGuiEx::ColorEdit3("Enemies Visible", g_Options.color_esp_enemy_visible);
//            ImGuiEx::ColorEdit3("Allies Occluded", g_Options.color_esp_ally_occluded);
//            ImGuiEx::ColorEdit3("Enemies Occluded", g_Options.color_esp_enemy_occluded);
//            //ImGuiEx::ColorEdit3("Crosshair", g_Options.color_esp_crosshair);
//            ImGuiEx::ColorEdit3("Dropped Weapons", g_Options.color_esp_weapons);
//            ImGuiEx::ColorEdit3("Defuse Kit", g_Options.color_esp_defuse);
//            ImGuiEx::ColorEdit3("Planted C4", g_Options.color_esp_c4);
//			ImGuiEx::ColorEdit3("Item Esp", g_Options.color_esp_item);
//            ImGui::PopItemWidth();
//
//            ImGui::Columns(1, nullptr, false);
//            ImGui::PopStyleVar();
//        } else if(active_esp_tab == 1) {
//            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ style.WindowPadding.x, style.ItemSpacing.y });
//            ImGui::Columns(3, nullptr, false);
//            ImGui::SetColumnOffset(1, group_w / 3.0f);
//            ImGui::SetColumnOffset(2, 2 * group_w / 3.0f);
//            ImGui::SetColumnOffset(3, group_w);
//
//            ImGui::Checkbox("Enabled", g_Options.glow_enabled);
//            ImGui::Checkbox("Team check", g_Options.glow_enemies_only);
//            ImGui::Checkbox("Players", g_Options.glow_players);
//            ImGui::Checkbox("Chickens", g_Options.glow_chickens);
//            ImGui::Checkbox("C4 Carrier", g_Options.glow_c4_carrier);
//            ImGui::Checkbox("Planted C4", g_Options.glow_planted_c4);
//            ImGui::Checkbox("Defuse Kits", g_Options.glow_defuse_kits);
//            ImGui::Checkbox("Weapons", g_Options.glow_weapons);
//
//            ImGui::NextColumn();
//
//            ImGui::PushItemWidth(100);
//            ImGuiEx::ColorEdit3("Ally", g_Options.color_glow_ally);
//            ImGuiEx::ColorEdit3("Enemy", g_Options.color_glow_enemy);
//            ImGuiEx::ColorEdit3("Chickens", g_Options.color_glow_chickens);
//            ImGuiEx::ColorEdit3("C4 Carrier", g_Options.color_glow_c4_carrier);
//            ImGuiEx::ColorEdit3("Planted C4", g_Options.color_glow_planted_c4);
//            ImGuiEx::ColorEdit3("Defuse Kits", g_Options.color_glow_defuse);
//            ImGuiEx::ColorEdit3("Weapons", g_Options.color_glow_weapons);
//            ImGui::PopItemWidth();
//
//            ImGui::NextColumn();
//
//            ImGui::Columns(1, nullptr, false);
//            ImGui::PopStyleVar();
//        } else if(active_esp_tab == 2) {
//            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ style.WindowPadding.x, style.ItemSpacing.y });
//            ImGui::Columns(3, nullptr, false);
//            ImGui::SetColumnOffset(1, group_w / 3.0f);
//            ImGui::SetColumnOffset(2, 2 * group_w / 2.9f);
//            ImGui::SetColumnOffset(3, group_w);
//
//            ImGui::BeginGroupBox("Players");
//            {
//                ImGui::Checkbox("Enabled", g_Options.chams_player_enabled); ImGui::SameLine();
//                ImGui::Checkbox("Team Check", g_Options.chams_player_enemies_only);
//                ImGui::Checkbox("Wireframe", g_Options.chams_player_wireframe);
//                ImGui::Checkbox("Flat", g_Options.chams_player_flat);
//                ImGui::Checkbox("Ignore-Z", g_Options.chams_player_ignorez); ImGui::SameLine();
//                ImGui::Checkbox("Glass", g_Options.chams_player_glass);
//                ImGui::PushItemWidth(110);
//                ImGuiEx::ColorEdit4("Ally (Visible)", g_Options.color_chams_player_ally_visible);
//                ImGuiEx::ColorEdit4("Ally (Occluded)", g_Options.color_chams_player_ally_occluded);
//                ImGuiEx::ColorEdit4("Enemy (Visible)", g_Options.color_chams_player_enemy_visible);
//                ImGuiEx::ColorEdit4("Enemy (Occluded)", g_Options.color_chams_player_enemy_occluded);
//                ImGui::PopItemWidth();
//            }
//            ImGui::EndGroupBox();
//
//            ImGui::NextColumn();
//
//            ImGui::BeginGroupBox("Arms");
//            {
//                ImGui::Checkbox("Enabled", g_Options.chams_arms_enabled);
//                ImGui::Checkbox("Wireframe", g_Options.chams_arms_wireframe);
//                ImGui::Checkbox("Flat", g_Options.chams_arms_flat);
//                ImGui::Checkbox("Ignore-Z", g_Options.chams_arms_ignorez);
//                ImGui::Checkbox("Glass", g_Options.chams_arms_glass);
//                ImGui::PushItemWidth(110);
//                ImGuiEx::ColorEdit4("Color (Visible)", g_Options.color_chams_arms_visible);
//                ImGuiEx::ColorEdit4("Color (Occluded)", g_Options.color_chams_arms_occluded);
//                ImGui::PopItemWidth();
//            }
//            ImGui::EndGroupBox();
//
//            ImGui::Columns(1, nullptr, false);
//            ImGui::PopStyleVar();
//        }
//    }
//    ImGui::EndGroupBox();
//}
//
//void RenderMiscTab()
//{
//    bool placeholder_true = true;
//
//    auto& style = ImGui::GetStyle();
//    float group_w = ImGui::GetCurrentWindow()->Size.x - style.WindowPadding.x * 2;
//
//    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
//    ImGui::ToggleButton("MISC", &placeholder_true, ImVec2{ group_w, 25.0f });
//    ImGui::PopStyleVar();
//
//    ImGui::BeginGroupBox("##body_content");
//    {
//        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ style.WindowPadding.x, style.ItemSpacing.y });
//        ImGui::Columns(3, nullptr, false);
//        ImGui::SetColumnOffset(1, group_w / 3.0f);
//        ImGui::SetColumnOffset(2, 2 * group_w / 3.0f);
//        ImGui::SetColumnOffset(3, group_w);
//
//        ImGui::Checkbox("NightMode", g_Options.misc_nightmode);
//        //ImGui::Checkbox("Desync", g_Options.misc_desync);
//        ImGui::Checkbox("Bunny hop", g_Options.misc_bhop);
//		ImGui::Checkbox("Third Person", g_Options.misc_thirdperson);
//		//if(g_Options.misc_thirdperson)
//			//ImGui::SliderFloat("Distance", g_Options.misc_thirdperson_dist, 0.f, 150.f);
//        //ImGui::Checkbox("No hands", g_Options.misc_no_hands);
//		//ImGui::Checkbox("Rank reveal", g_Options.misc_showranks);
//		//ImGui::Checkbox("Watermark##hc", g_Options.misc_watermark);
//        //ImGui::PushItemWidth(-1.0f);
//		ImGui::NextColumn();
//        //ImGui::SliderInt("viewmodel_fov:", g_Options.viewmodel_fov, 68, 120);
//        //ImGui::PopItemWidth();
//
//        ImGui::Columns(1, nullptr, false);
//        ImGui::PopStyleVar();
//    }
//    ImGui::EndGroupBox();
//}
//
//void RenderEmptyTab()
//{
//	auto& style = ImGui::GetStyle();
//	float group_w = ImGui::GetCurrentWindow()->Size.x - style.WindowPadding.x * 2;
//
//	bool placeholder_true = true;
//
//	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
//	ImGui::PopStyleVar();
//
//
//;
//    ImGui::Checkbox("autoshoot", g_Options.legit_autofire);
//  /*  if (g_Options.misc_backtrack)
//        ImGui::SliderInt("", g_Options.misc_backtrack_slider, 0, 12);*/
//    ImGui::Checkbox("Fakelag", g_Options.misc_fakelag);
//    if (g_Options.misc_fakelag)
//        ImGui::SliderInt(" ", g_Options.misc_fakelagammount, 0, 14);
//    ImGui::Checkbox("Grenade Preview", g_Options.misc_grenadepreview);
//    ImGui::Checkbox("Bunny Hop", g_Options.misc_bhop);
//    ImGui::Checkbox("Night Mode", g_Options.misc_nightmode);   
//    ImGui::Checkbox("Force Crosshair", g_Options.esp_crosshair);
//    ImGui::Checkbox("Boxes", g_Options.esp_player_boxes);
//    ImGui::Checkbox("Names", g_Options.esp_player_names);//glow_enemies_only
//    ImGui::Checkbox("Glow", g_Options.glow_players);
//    ImGui::Checkbox("Chams", g_Options.chams_player_enabled);
//    ImGui::Checkbox("Ignore-Z", g_Options.chams_player_ignorez); 
//    ImGui::Checkbox("Viewmodel", g_Options.viewmodel_fov);
//    ImGui::Checkbox("Third Person", g_Options.misc_thirdperson);
//    ImGui::Checkbox("Remove Scope", g_Options.misc_removezoom);
//    ImGui::Checkbox("Bullet Beams", g_Options.misc_bulletbeams); 
//    ImGui::Checkbox("Hit Marker", g_Options.misc_hitmarker);
//    //ImGui::Checkbox("Spectator List", g_Options.misc_spectator);
//
//
//
//	auto pos = ImGui::GetCurrentWindow()->Pos;
//	auto wsize = ImGui::GetCurrentWindow()->Size;
//
//	pos = pos + wsize / 2.0f;
//}

void RenderConfigTab()
{
    auto& style = ImGui::GetStyle();
    float group_w = ImGui::GetCurrentWindow()->Size.x - style.WindowPadding.x * 2;

    bool placeholder_true = true;

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0,0));
    ImGui::ToggleButton("CONFIG", &placeholder_true, ImVec2{ group_w, 25.0f });
    ImGui::PopStyleVar();

    ImGui::BeginGroupBox("##body_content");
    {
		if (ImGui::Button("Save cfg")) {
			Config::Get().Save();
		}
		if (ImGui::Button("Load cfg")) {
			Config::Get().Load();
		}
    }
    ImGui::EndGroupBox();
}

void Menu::Initialize()
{
	CreateStyle();

    _visible = true;
}

void Menu::Shutdown()
{
    ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void Menu::OnDeviceLost()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
}

void Menu::OnDeviceReset()
{
    ImGui_ImplDX9_CreateDeviceObjects();
}

void Menu::Render()
{
    ImGui::GetIO().MouseDrawCursor = _visible;

    if (!_visible)
        return;

    const auto sidebar_size = get_sidebar_size();
    static int active_sidebar_tab = 0;

    ImGui::SetNextWindowPos(ImVec2{ 0, 0 }, ImGuiSetCond_Once);
    ImGui::SetNextWindowSize(ImVec2{ 400, 280 }, ImGuiSetCond_Once);
    if (ImGui::Begin(" ",
        &_visible,
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoTitleBar)) {
        auto size = ImVec2{ 0.0f, sidebar_size.y };

        static char* tab_names[] = {"Rage", "Legit", "Visual", "Misc", "Skinchanger" };

        static char* tabweapon_names[] = { "Sniper", "Rifle", "Smg", "Pistol", "Other" };
        static int   active_tab = 0;

        bool placeholder_true = true;

        auto& style = ImGui::GetStyle();
        float group_w = ImGui::GetCurrentWindow()->Size.x - style.WindowPadding.x * 2;
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        render_tabs(tab_names, active_tab, group_w / _countof(tab_names), 25.0f, true);
        ImGui::PopStyleVar();
        if (active_tab == 0)
        {
            ImGui::BeginGroupBox("Coming soon");
            {
                
            }
            ImGui::EndGroupBox();
        }

        else if (active_tab == 1) // Legit
        {
            ImGui::BeginGroupBox("main");
            {
                //ImGui::Text("Legit");

                ImGui::BeginChild("Aim", ImVec2(115, 210), true);
                {

                    ImGui::Checkbox("enable", g_Options.legit_enable);
                    ImGui::Checkbox("Lagcomp", g_Options.misc_backtrack);
                    ImGui::Text("fov");
                    ImGui::SliderFloat("    ", g_Options.legit_fov, 0, 180, "%.1f");
                    ImGui::Text("rcs");
                    ImGui::SliderInt("      ", g_Options.LegitAimbotRcs, 0, 100, "%d");
                    ImGui::Text("smooth");
                    ImGui::SliderInt("        ", g_Options.LegitAimbotSmooth, 0, 50, "%d");


                }
                ImGui::EndChild();
                ImGui::SameLine();
                ImGui::BeginChild("misc", ImVec2(115, 210), true);
                {

                    ImGui::Text("test");

                }
                ImGui::EndChild();
                ImGui::SameLine();
                ImGui::BeginChild("lmao", ImVec2(115, 210), true);
                {
                    ImGui::Text("test");
             
                }
                ImGui::EndChild();

            }
            ImGui::EndGroupBox();
        }
        else if (active_tab == 2) // Visual
        {
            ImGui::BeginGroupBox("main");
            {
                ImGui::BeginChild("main", ImVec2(115, 210), true);
                {


                    ImGui::Checkbox("Chams", g_Options.chams_player_enabled);
                    ImGui::SameLine();
                    ImGuiEx::ColorEdit4(" ", g_Options.color_chams_player_enemy_visible, ImGuiColorEditFlags_NoInputs);
                    if (g_Options.chams_player_enabled)
                    {
                        ImGui::Checkbox("IgnoreZ", g_Options.chams_player_ignorez);
                        ImGui::SameLine();
                        ImGuiEx::ColorEdit4("##xqzcolor", g_Options.color_chams_player_enemy_occluded, ImGuiColorEditFlags_NoInputs);
                        //   ImGui::Checkbox("btchams", g_Options.chams_player_backtrack);

                    }
                    ImGui::Checkbox("Glow", g_Options.glow_players);
                    ImGui::SameLine();
                    ImGuiEx::ColorEdit4("##glowcolor", g_Options.color_glow_enemy, ImGuiColorEditFlags_NoInputs);
                    ImGui::Checkbox("ESP", g_Options.esp_player_boxes);

                }ImGui::EndChild();
                ImGui::SameLine();
                ImGui::BeginChild("misc", ImVec2(115, 210), true);
                {
                    ImGui::Checkbox("EspOnDeath", g_Options.misc_espdeath);

                }ImGui::EndChild();
                ImGui::SameLine();
                ImGui::BeginChild("other", ImVec2(115, 210), true);
                {
                 
                    ImGui::Checkbox("NadePredict", g_Options.misc_grenadepreview);
                    ImGui::Checkbox("NightMode", g_Options.misc_nightmode);
                    ImGui::Checkbox("crosshair", g_Options.esp_crosshair);
                    ImGui::Checkbox("BulletBeams", g_Options.misc_bulletbeams);
                }ImGui::EndChild();
            }
            ImGui::EndGroupBox();
        }
        else if (active_tab == 3) // Misc
        {
            ImGui::BeginGroupBox("main");
            {
   
                //   ImGui::Checkbox("AutoAccept", g_Options.misc_autoaccept);
                ImGui::Checkbox("BunnyHop", g_Options.misc_bhop);
                ImGui::Checkbox("FakeLag", g_Options.misc_fakelag);
                ImGui::Checkbox("ViewModel", g_Options.viewmodel_fov);
                ImGui::Checkbox("ThirdPerson", g_Options.misc_thirdperson);
                ImGui::Checkbox("RemoveScope", g_Options.misc_removezoom);
                ImGui::Checkbox("HitMarker", g_Options.misc_hitmarker);



                if (ImGui::Button("Save cfg")) {
                    Config::Get().Save();
                }
                if (ImGui::Button("Load cfg")) {
                    Config::Get().Load();
                }
            }
            ImGui::EndGroupBox();
        }
        else if (active_tab == 4) // Misc
        {
            ImGui::BeginGroupBox("main");
            {
                ImGui::Text("lol u thought");
               
            }
            ImGui::EndGroupBox();
        }
        ImGui::End();
    }  
}

void Menu::Toggle()
{
    _visible = !_visible;
}

void Menu::CreateStyle()
{
    ImGui::StyleColorsDark();
    ImGui::SetColorEditOptions(ImGuiColorEditFlags_HEX);
    _style.FrameRounding = 4.f;
    _style.WindowRounding = 4.f;
    _style.ChildRounding = 4.f;
    ImGui::GetStyle() = _style;
}