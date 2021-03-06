#include "chams.hpp"
#include <fstream>

#include "../valve_sdk/csgostructs.hpp"
#include "../options.hpp"
#include "../hooks.hpp"
#include "../helpers/input.hpp"
#include "../backtrack.hpp"
#include "ragebot.h"

Chams::Chams() {
	std::ofstream("csgo\\materials\\material_textured.vmt") << R"#("VertexLitGeneric"
{
  "$basetexture" "vgui/white_additive"
  "$ignorez"      "0"
  "$envmap"       ""
  "$nofog"        "1"
  "$model"        "1"
  "$nocull"       "0"
  "$selfillum"    "1"
  "$halflambert"  "1"
  "$znearer"      "0"
  "$flat"         "1"
}
)#";
	std::ofstream("csgo\\materials\\material_textured_ignorez.vmt") << R"#("VertexLitGeneric"
{
  "$basetexture" "vgui/white_additive"
  "$ignorez"      "1"
  "$envmap"       ""
  "$nofog"        "1"
  "$model"        "1"
  "$nocull"       "0"
  "$selfillum"    "1"
  "$halflambert"  "1"
  "$znearer"      "0"
  "$flat"         "1"
}
)#";
	std::ofstream("csgo\\materials\\material_flat.vmt") << R"#("UnlitGeneric"
{
  "$ignorez"      "0"
  "$envmap"       ""
  "$nofog"        "1"
  "$model"        "1"
  "$nocull"       "0"
  "$selfillum"    "1"
  "$halflambert"  "1"
  "$znearer"      "0"
  "$flat"         "1"
}
)#";
	std::ofstream("csgo\\materials\\material_flat_ignorez.vmt") << R"#("UnlitGeneric"
{
  "$ignorez"      "1"
  "$envmap"       ""
  "$nofog"        "1"
  "$model"        "1"
  "$nocull"       "0"
  "$selfillum"    "1"
  "$halflambert"  "1"
  "$znearer"      "0"
  "$flat"         "1"
}
)#";

	materialRegular = g_MatSystem->FindMaterial("material_textured", TEXTURE_GROUP_MODEL);
	materialRegularIgnoreZ = g_MatSystem->FindMaterial("material_textured_ignorez", TEXTURE_GROUP_MODEL);
	materialFlat = g_MatSystem->FindMaterial("material_flat", TEXTURE_GROUP_MODEL);
	materialFlatIgnoreZ = g_MatSystem->FindMaterial("material_flat_ignorez", TEXTURE_GROUP_MODEL);
	materialDogtag = g_MatSystem->FindMaterial("models\\inventory_items\\dogtags\\dogtags_outline", TEXTURE_GROUP_OTHER);
}

Chams::~Chams() {
}


void Chams::OverrideMaterial(bool ignoreZ, bool flat, bool wireframe, bool glass, const Color& rgba) {
	IMaterial* material = nullptr;

	if (flat) {
		material = materialFlat;
	}
	else {
		material = materialRegular;
	}

	material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, ignoreZ);


	if (glass) {
		material = materialFlat;
		material->AlphaModulate(0.45f);
	}
	else {
		material->AlphaModulate(
			rgba.a() / 255.0f);
	}

	material->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, wireframe);
	material->ColorModulate(
		rgba.r() / 255.0f,
		rgba.g() / 255.0f,
		rgba.b() / 255.0f);

	g_MdlRender->ForcedMaterialOverride(material);
}


void Chams::OnDrawModelExecute(
	IMatRenderContext* ctx,
	const DrawModelState_t& state,
	const ModelRenderInfo_t& info,
	matrix3x4_t* matrix)
{
	static auto fnDME = Hooks::mdlrender_hook.get_original<decltype(&Hooks::hkDrawModelExecute)>(index::DrawModelExecute);

	const auto mdl = info.pModel;

	bool is_arm = strstr(mdl->szName, "arms") != nullptr;
	bool is_player = strstr(mdl->szName, "models/player") != nullptr;
	bool is_sleeve = strstr(mdl->szName, "sleeve") != nullptr;
	static std::vector<IMaterial*> Materials =
	{
		nullptr,
		materialRegular,
		materialFlat,
		materialDogtag,
		nullptr,
		nullptr,
		nullptr,
		nullptr
	};
	static std::vector<IMaterial*> ZMaterials =
	{
		nullptr,
		materialRegularIgnoreZ,
		materialFlatIgnoreZ,
		materialDogtag,
		nullptr,
		nullptr,
		nullptr,
		nullptr
	};

	//bool is_weapon = strstr(mdl->szName, "weapons/v_")  != nullptr;
	if (is_player)
	{

		auto entity = C_BasePlayer::GetPlayerByIndex(info.entity_index);
		const auto clr_front = entity ? g_Options.color_chams_player_enemy_visible : g_Options.color_chams_player_ally_visible;
		const auto clr_back = entity ? g_Options.color_chams_player_enemy_occluded : g_Options.color_chams_player_ally_occluded;
		if (g_LocalPlayer && entity && entity->IsAlive() && !entity->IsDormant())
		{
			if (entity->IsEnemy() && g_Options.chams_player_enabled)
			{
				if (g_Options.rage_enable && RageAimbot::Get().BacktrackRecords[info.entity_index].size() > 0 && g_Options.chams_player_backtrack)
				{
					if (RageAimbot::Get().BacktrackRecords[info.entity_index].back().MatrixBuilt
						&& RageAimbot::Get().BacktrackRecords[info.entity_index].back().BoneMatrix)
					{
						g_RenderView->SetColorModulation(255,255,255);
						g_RenderView->SetBlend(float(90.f) / 255.f);
						g_MdlRender->ForcedMaterialOverride(materialFlatIgnoreZ);
						fnDME(g_MdlRender, 0, ctx, state, info, RageAimbot::Get().BacktrackRecords[info.entity_index].back().BoneMatrix);
					}
				}
				else if (g_Options.misc_backtrack && LegitBacktrack::Get().BacktrackRecords[info.entity_index].size() > 0 && g_Options.chams_player_backtrack)
				{		
						if (LegitBacktrack::Get().BacktrackRecords[info.entity_index].back().MatrixBuilt
							&& LegitBacktrack::Get().BacktrackRecords[info.entity_index].back().BoneMatrix)
						{
							g_RenderView->SetColorModulation(255, 255, 255);
							g_RenderView->SetBlend(float(90) / 255.f);
							g_MdlRender->ForcedMaterialOverride(materialFlatIgnoreZ);
							fnDME(g_MdlRender, 0, ctx, state, info, LegitBacktrack::Get().BacktrackRecords[info.entity_index].back().BoneMatrix);
						}
				
				}
				if (g_Options.chams_player_ignorez)
				{
					g_RenderView->SetColorModulation(27, 161, 209);
					g_RenderView->SetBlend(float(255.f) / 255.f);
					g_MdlRender->ForcedMaterialOverride(materialFlatIgnoreZ);
					fnDME(g_MdlRender, 0, ctx, state, info, matrix);

					/*OverrideMaterial(true, g_Options.chams_player_flat, g_Options.chams_player_wireframe, false, clr_back);
					fnDME(g_MdlRender, 0, ctx, state, info, matrix);
					OverrideMaterial(false, g_Options.chams_player_flat, g_Options.chams_player_wireframe, false, clr_front);*/

				}
				g_RenderView->SetColorModulation(96, 196, 20);
				g_RenderView->SetBlend(float(90.f) / 255.f);
				g_MdlRender->ForcedMaterialOverride(materialFlat);
				fnDME(g_MdlRender, 0, ctx, state, info, matrix);
			}
		}
	}
	//if (is_player && g_Options.chams_player_enabled) {
	//	// 
	//	// Draw player Chams.
	//	// 
	//	auto ent = C_BasePlayer::GetPlayerByIndex(info.entity_index);

	//	if (ent && g_LocalPlayer && ent->IsAlive()) {
	//		const auto enemy = ent->m_iTeamNum() != g_LocalPlayer->m_iTeamNum();
	//		if (!enemy && g_Options.chams_player_enemies_only)
	//			return;

	//		const auto clr_front = enemy ? g_Options.color_chams_player_enemy_visible : g_Options.color_chams_player_ally_visible;
	//		const auto clr_back = enemy ? g_Options.color_chams_player_enemy_occluded : g_Options.color_chams_player_ally_occluded;

	//		if (g_Options.chams_player_backtrack)
	//		{

	//			for (int t = 0; t < LegitBacktrack::Get().BacktrackRecords[info.entity_index].size(); t++)
	//			{
	//				if (!LegitBacktrack::Get().BacktrackRecords[info.entity_index].at(t).MatrixBuilt
	//					|| !LegitBacktrack::Get().BacktrackRecords[info.entity_index].at(t).BoneMatrix)
	//					continue;

	//				g_RenderView->SetColorModulation(255, 255, 255);
	//				g_RenderView->SetBlend(float(90.f));
	//				g_MdlRender->ForcedMaterialOverride(materialFlat);
	//				fnDME(g_MdlRender, 0, ctx, state, info, LegitBacktrack::Get().BacktrackRecords[info.entity_index].back().BoneMatrix);

	//				/*OverrideMaterial(false, g_Options.chams_player_flat, g_Options.chams_player_wireframe, false, clr_back);
	//				fnDME(g_MdlRender, 0, ctx, state, info, LegitBacktrack::Get().BacktrackRecords[info.entity_index].back().BoneMatrix);*/
	//			}
	//		}
	//		if (g_Options.chams_player_ignorez)
	//		{
	//			OverrideMaterial(true, g_Options.chams_player_flat, g_Options.chams_player_wireframe,	false, clr_back);
	//			fnDME(g_MdlRender, 0, ctx, state, info, matrix);
	//			OverrideMaterial(false,g_Options.chams_player_flat, g_Options.chams_player_wireframe, false,clr_front);

	//			
	//

	//		}
	//		else {
	//			OverrideMaterial(false,g_Options.chams_player_flat,g_Options.chams_player_wireframe, g_Options.chams_player_glass, clr_front);
	//		}
	//		


	//	}
	//	
	//}

	else if (is_sleeve && g_Options.chams_arms_enabled) {
		auto material = g_MatSystem->FindMaterial(mdl->szName, TEXTURE_GROUP_MODEL);
		if (!material)
			return;
		// 
		// Remove sleeves when drawing Chams.
		// 
		material->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);
		g_MdlRender->ForcedMaterialOverride(material);
	}
	else if (is_arm) {
		auto material = g_MatSystem->FindMaterial(mdl->szName, TEXTURE_GROUP_MODEL);
		if (!material)
			return;
		if (g_Options.misc_no_hands) {
			// 
			// No hands.
			// 
			material->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);
			g_MdlRender->ForcedMaterialOverride(material);
		}
		else if (g_Options.chams_arms_enabled) {
			if (g_Options.chams_arms_ignorez) {
				OverrideMaterial(
					true,
					g_Options.chams_arms_flat,
					g_Options.chams_arms_wireframe,
					false,
					g_Options.color_chams_arms_occluded);
				fnDME(g_MdlRender, 0, ctx, state, info, matrix);
				OverrideMaterial(
					false,
					g_Options.chams_arms_flat,
					g_Options.chams_arms_wireframe,
					false,
					g_Options.color_chams_arms_visible);
			}
			else {
				OverrideMaterial(
					false,
					g_Options.chams_arms_flat,
					g_Options.chams_arms_wireframe,
					g_Options.chams_arms_glass,
					g_Options.color_chams_arms_visible);
			}
		}
	}
}