#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include "valve_sdk/Misc/Color.hpp"

#define A( s ) #s
#define OPTION(type, var, val) Var<type> var = {A(var), val}

template <typename T = bool>
class Var {
public:
	std::string name;
	std::shared_ptr<T> value;
	int32_t size;
	Var(std::string name, T v) : name(name) {
		value = std::make_shared<T>(v);
		size = sizeof(T);
	}
	operator T() { return *value; }
	operator T*() { return &*value; }
	operator T() const { return *value; }
	//operator T*() const { return value; }
};

class Options
{
public:
		//rage
		OPTION(float, RageAimbotMinDmg, 30);
		OPTION(float, RageAimbotHitchance, 60);
		OPTION(bool, rage_sloswalk, false);
		OPTION(bool, rage_enable, false);
		OPTION(bool, rage_silent, false);
		OPTION(bool, hideshots, false);
		OPTION(bool, doubletap, false);

		OPTION(bool, RageAimbotHead, false);
		OPTION(bool, RageAimbotBody, false);
		OPTION(bool, RageAimbotLegs, false);
		OPTION(bool, RageAimbotToes, false);
		OPTION(bool, RageAimbotSafePoint, false);
		OPTION(bool, RageAimbotDelayShot,  false);
		OPTION(bool, RageAntiaimEnabled, false);
		OPTION(bool, RageAimbotResolver, false);

		OPTION(int, RageAimbotBaimAfter, 2);

		OPTION(int, RageAimbotHeadScale, 40);
		OPTION(int, RageAimbotBodyScale, 60);

	//	RageAimbotBaimAfter


		//legit
		OPTION(bool, legit_enable, false);
		OPTION(float, legit_fov, 5.f);
		OPTION(int, legit_hitbox, 0);
		OPTION(int, LegitAimbotRcs, 100);//LegitAimbotSmooth
		OPTION(int, LegitAimbotSmooth, 10);
		OPTION(bool, legit_autofire, 0);
		// 
		// ESP
		// 
		OPTION(bool, esp_enabled, true);
		OPTION(bool, esp_enemies_only, true);
		OPTION(bool, esp_player_boxes, false);
		OPTION(bool, misc_espdeath, false);
		OPTION(bool, esp_player_names, false);
		OPTION(bool, esp_player_health, false);
		OPTION(bool, esp_player_armour, false);
		OPTION(bool, esp_player_weapons, false);
		OPTION(bool, esp_player_snaplines, false);
		OPTION(bool, esp_crosshair, false);
		OPTION(bool, esp_dropped_weapons, false);
		OPTION(bool, esp_defuse_kit, false);
		OPTION(bool, esp_planted_c4, true);
		OPTION(bool, esp_items, false);
		OPTION(bool, misc_nosmoke, false);

		// 
		// GLOW
		// 
		OPTION(bool, glow_enabled, true);
		OPTION(bool, glow_enemies_only, true);
		OPTION(bool, glow_players, false);
		OPTION(bool, glow_chickens, false);
		OPTION(bool, glow_c4_carrier, false);
		OPTION(bool, glow_planted_c4, true);
		OPTION(bool, glow_defuse_kits, false);
		OPTION(bool, glow_weapons, false);

		//
		// CHAMS
		//
		OPTION(bool, chams_player_enabled, false);
		OPTION(bool, chams_player_enemies_only, true);
		OPTION(bool, chams_player_wireframe, false);
		OPTION(bool, chams_player_flat, false);
		OPTION(bool, chams_player_ignorez, false);
		OPTION(bool, chams_player_backtrack, false);
		OPTION(bool, chams_player_glass, false);
		OPTION(bool, chams_arms_enabled, false);
		OPTION(bool, chams_arms_wireframe, false);
		OPTION(bool, chams_arms_flat, false);
		OPTION(bool, chams_arms_ignorez, false);
		OPTION(bool, chams_arms_glass, false);

		//
		// MISC
		//
		OPTION(bool, misc_triggerbot, false);
		OPTION(bool, misc_fakelag, false);
		OPTION(int, misc_fakelagammount, 0);
		OPTION(bool, misc_spectator, false);
		OPTION(bool, misc_hitmarker, false);
		OPTION(bool, misc_lefthand_knife, false);
		OPTION(bool, misc_bulletbeams, false);
		OPTION(bool, misc_grenadepreview, false);
		OPTION(bool, misc_nightmode, true);
		OPTION(float, misc_nightmode_slider, 1.f);	
		OPTION(float, misc_nightmode_prop_r, 1.f);
		OPTION(float, misc_nightmode_prop_g, 1.f);
		OPTION(float, misc_nightmode_prop_b, 1.f);
		OPTION(float, misc_nightmode_prop_alpha, 1.f);
		OPTION(bool, misc_backtrack, false);
		OPTION(int, misc_backtrack_slider, 12);
		OPTION(bool, misc_removezoom, false);
		OPTION(bool, misc_bhop, false);
		OPTION(int, misc_bhop_hitchance, 100);
		OPTION(int, misc_bhop_maxfailed, 10);
		OPTION(int, misc_bhop_max, 10);
		OPTION(bool, misc_autoaccept, false);
		OPTION(bool, misc_autostrafe, false);
		OPTION(bool, misc_no_hands, false);
		OPTION(bool, misc_thirdperson, false);
		OPTION(bool, misc_showranks, true);
		OPTION(bool, misc_watermark, true);
		OPTION(float, misc_thirdperson_dist, 150.f);
		OPTION(bool, viewmodel_fov, false);		

		// 
		// COLORS
		// 
		OPTION(Color, color_esp_ally_visible, Color(0, 128, 255));
		OPTION(Color, color_esp_enemy_visible, Color(255, 255, 255));
		OPTION(Color, color_esp_ally_occluded, Color(0, 128, 255));
		OPTION(Color, color_esp_enemy_occluded, Color(255, 255, 255));
		OPTION(Color, color_esp_crosshair, Color(255, 255, 255));
		OPTION(Color, color_esp_weapons, Color(128, 0, 128));
		OPTION(Color, color_esp_defuse, Color(0, 128, 255));
		OPTION(Color, color_esp_c4, Color(255, 255, 0));
		OPTION(Color, color_esp_item, Color(255, 255, 255));

		OPTION(Color, color_glow_ally, Color(0, 128, 255));
		OPTION(Color, color_glow_enemy, Color(176, 37, 204, 190));
		OPTION(Color, color_glow_chickens, Color(0, 128, 0));
		OPTION(Color, color_glow_c4_carrier, Color(255, 255, 0));
		OPTION(Color, color_glow_planted_c4, Color(128, 0, 128));
		OPTION(Color, color_glow_defuse, Color(255, 255, 255));
		OPTION(Color, color_glow_weapons, Color(255, 128, 0));

		OPTION(Color, color_chams_player_ally_visible, Color(0, 128, 255));
		OPTION(Color, color_chams_player_ally_occluded, Color(0, 255, 128));
		OPTION(Color, color_chams_player_enemy_visible, Color(96, 196, 20));
		OPTION(Color, color_chams_player_enemy_occluded, Color(27, 161, 209));
		OPTION(Color, color_chams_arms_visible, Color(0, 128, 255));
		OPTION(Color, color_chams_arms_occluded, Color(0, 128, 255));
		OPTION(Color, color_watermark, Color(255, 255, 255)); // no menu config cuz its useless
};

inline Options g_Options;
inline bool   g_Unload;
