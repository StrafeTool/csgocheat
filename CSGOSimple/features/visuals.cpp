#include <algorithm>

#include "visuals.hpp"
#include "../backtrack.hpp"
#include "../options.hpp"
#include "../helpers/math.hpp"
#include "../helpers/utils.hpp"

vgui::HFont esp_font;

void Renders::CreateFonts()
{
	VerdanaBold12 = g_VGuiSurface->CreateFont_();
	icon = g_VGuiSurface->CreateFont_();

	g_VGuiSurface->SetFontGlyphSet(VerdanaBold12, "Tahoma Bold", 14, 400, 0, 0, FONTFLAG_DROPSHADOW);
	g_VGuiSurface->SetFontGlyphSet(icon, "Counter-Strike", 18, 500, 0, 0, FONTFLAG_ANTIALIAS);
}
void Renders::TextSize(int& Width, int& Height, const char* Text, vgui::HFont Font)
{
	std::wstring WText = std::wstring(std::string_view(Text).begin(), std::string_view(Text).end());
	g_VGuiSurface->GetTextSize(Font, WText.c_str(), Width, Height);
}
void Renders::Text(int X, int Y, const char* Text, vgui::HFont Font, Color DrawColor, bool Center/*, bool eatmyasscheeks*/)
{
	std::wstring WText = std::wstring(std::string_view(Text).begin(), std::string_view(Text).end());
	g_VGuiSurface->DrawSetTextFont(Font);
	g_VGuiSurface->DrawSetTextColor(DrawColor);
	if (Center)
	{
		int TextWidth, TextHeight;
		Renders::Get().TextSize(TextWidth, TextHeight, Text, Font);
		g_VGuiSurface->DrawSetTextPos(X - TextWidth / 2, Y);
	}
	else
		g_VGuiSurface->DrawSetTextPos(X, Y);
	g_VGuiSurface->DrawPrintText(WText.c_str(), wcslen(WText.c_str()));
}
void Renders::FilledRectange(int X1, int Y1, int X2, int Y2, Color DrawColor)
{
	g_VGuiSurface->DrawSetColor(DrawColor);
	g_VGuiSurface->DrawFilledRect(X1, Y1, X2, Y2);
}
void Renders::OutlinedRectange(int X1, int Y1, int X2, int Y2, Color DrawColor)
{
	g_VGuiSurface->DrawSetColor(DrawColor);
	g_VGuiSurface->DrawOutlinedRect(X1, Y1, X2, Y2);
}
void Renders::Line(int X1, int Y1, int X2, int Y2, Color DrawColor)
{
	g_VGuiSurface->DrawSetColor(DrawColor);
	g_VGuiSurface->DrawLine(X1, Y1, X2, Y2);
}


RECT GetBBox(C_BaseEntity* ent)
{
	RECT rect{};
	auto collideable = ent->GetCollideable();

	if (!collideable)
		return rect;

	auto min = collideable->OBBMins();
	auto max = collideable->OBBMaxs();

	const matrix3x4_t& trans = ent->m_rgflCoordinateFrame();

	Vector points[] = {
		Vector(min.x, min.y, min.z),
		Vector(min.x, max.y, min.z),
		Vector(max.x, max.y, min.z),
		Vector(max.x, min.y, min.z),
		Vector(max.x, max.y, max.z),
		Vector(min.x, max.y, max.z),
		Vector(min.x, min.y, max.z),
		Vector(max.x, min.y, max.z)
	};

	Vector pointsTransformed[8];
	for (int i = 0; i < 8; i++) {
		Math::VectorTransform(points[i], trans, pointsTransformed[i]);
	}

	Vector screen_points[8] = {};

	for (int i = 0; i < 8; i++) {
		if (!Math::WorldToScreen(pointsTransformed[i], screen_points[i]))
			return rect;
	}

	auto left = screen_points[0].x;
	auto top = screen_points[0].y;
	auto right = screen_points[0].x;
	auto bottom = screen_points[0].y;

	for (int i = 1; i < 8; i++) {
		if (left > screen_points[i].x)
			left = screen_points[i].x;
		if (top < screen_points[i].y)
			top = screen_points[i].y;
		if (right < screen_points[i].x)
			right = screen_points[i].x;
		if (bottom > screen_points[i].y)
			bottom = screen_points[i].y;
	}
	return RECT{ (long)left, (long)top, (long)right, (long)bottom };
}

Visuals::Visuals()
{
	InitializeCriticalSection(&cs);
}

Visuals::~Visuals() {
	DeleteCriticalSection(&cs);
}

//--------------------------------------------------------------------------------
void Visuals::Render() {



}
//--------------------------------------------------------------------------------
bool Visuals::Player::Begin(C_BasePlayer* pl)
{
	if (pl->IsDormant() || !pl->IsAlive())
		return false;

	ctx.pl = pl;
	ctx.is_enemy = g_LocalPlayer->m_iTeamNum() != pl->m_iTeamNum();
	ctx.is_visible = g_LocalPlayer->CanSeePlayer(pl, HITBOX_CHEST);

	if (!ctx.is_enemy && g_Options.esp_enemies_only)
		return false;

	ctx.clr = ctx.is_enemy ? (ctx.is_visible ? g_Options.color_esp_enemy_visible : g_Options.color_esp_enemy_occluded) : (ctx.is_visible ? g_Options.color_esp_ally_visible : g_Options.color_esp_ally_occluded);

	auto head = pl->GetHitboxPos(HITBOX_HEAD);
	auto origin = pl->m_vecOrigin();

	head.z += 15;

	if (!Math::WorldToScreen(head, ctx.head_pos) ||
		!Math::WorldToScreen(origin, ctx.feet_pos))
		return false;

	auto h = fabs(ctx.head_pos.y - ctx.feet_pos.y);
	auto w = h / 1.65f;

	ctx.bbox.left = static_cast<long>(ctx.feet_pos.x - w * 0.5f);
	ctx.bbox.right = static_cast<long>(ctx.bbox.left + w);
	ctx.bbox.bottom = static_cast<long>(ctx.feet_pos.y);
	ctx.bbox.top = static_cast<long>(ctx.head_pos.y);

	return true;
}
//--------------------------------------------------------------------------------
bool Visuals::CreateFonts()
{
	esp_font = g_VGuiSurface->CreateFont_();
	g_VGuiSurface->SetFontGlyphSet(esp_font, "Tahoma", 13, 350, 0, 0, FONTFLAG_OUTLINE, FONTFLAG_ANTIALIAS);

	return true;
}
//--------------------------------------------------------------------------------



void Visuals::Player::RenderBox() {


	Renders::Get().OutlinedRectange(ctx.bbox.left - 1, ctx.bbox.top - 1, ctx.bbox.right + 1, ctx.bbox.bottom + 1, Color(0, 0, 0, 150));
	Renders::Get().OutlinedRectange(ctx.bbox.left + 1, ctx.bbox.top + 1, ctx.bbox.right - 1, ctx.bbox.bottom - 1, Color(0, 0, 0, 150));
	Renders::Get().OutlinedRectange(ctx.bbox.left, ctx.bbox.top, ctx.bbox.right, ctx.bbox.bottom, Color(255, 255, 255, 255));






	//Renders::Get().OutlinedRectange(ctx.bbox.left - 1, ctx.bbox.top - 1, ctx.bbox.right + 1, ctx.bbox.bottom + 1, Color(0, 0, 0, 150));
	//Renders::Get().OutlinedRectange(ctx.bbox.left + 1, ctx.bbox.top + 1, ctx.bbox.right - 1, ctx.bbox.bottom - 1, Color(0, 0, 0, 150));
	//Renders::Get().OutlinedRectange(ctx.bbox.left, ctx.bbox.top, ctx.bbox.right, ctx.bbox.bottom, Color(255, 255, 255, 255));

}
//--------------------------------------------------------------------------------
void Visuals::Player::RenderName()
{
	player_info_t PlayerInfo;
	g_EngineClient->GetPlayerInfo(ctx.pl->EntIndex(), &PlayerInfo);

	int TextWidth, TextHeight;
	Renders::Get().TextSize(TextWidth, TextHeight, PlayerInfo.szName, Renders::Get().VerdanaBold12);
	Renders::Get().Text(ctx.bbox.left + (ctx.bbox.right - ctx.bbox.left) / 2, ctx.bbox.top - TextHeight, PlayerInfo.szName, Renders::Get().VerdanaBold12, Color(255, 255, 255, 255), true);

	/*player_info_t info = ctx.pl->GetPlayerInfo();

	auto sz = g_namefont->CalcTextSizeA(14.f, FLT_MAX, 0.0f, info.szName);
	
	Render::Get().RenderText(info.szName, ctx.feet_pos.x - sz.x / 2, ctx.head_pos.y - sz.y, false, ctx.clr);*/
}
//--------------------------------------------------------------------------------
void Visuals::Player::RenderHealth()
{
	int health = ctx.pl->m_iHealth();
	if (health > 100)
		health = 100;

	float box_h = (float)fabs(ctx.bbox.bottom - ctx.bbox.top);
	float off = 6;

	auto height = box_h - (((box_h * health) / 100));

	int green = int(health * 2.55f);
	int red = 255 - green;

	int x = ctx.bbox.left - off;
	int y = ctx.bbox.top;
	int w = 4;
	int h = box_h;
	//25, 25, 25, 100
	g_VGuiSurface->DrawSetColor(Color(25, 25, 25, 100));
	g_VGuiSurface->DrawFilledRect(x, y, x + w, y + h);

	g_VGuiSurface->DrawSetColor(Color(red, green, 0, 100));
	g_VGuiSurface->DrawOutlinedRect(x + 1, y + height + 1, x + w - 1, y + h - 1);



	std::string hp = std::to_string(health);
	auto name = (health < 0.f) ? "%d" : hp;
	auto sz = g_namefont->CalcTextSizeA(14.f, FLT_MAX, 0.0f, name.c_str());
	if (health != 100)
		Render::Get().RenderText(name, ImVec2(x - w, y + height), false, Color(255, 255, 255, 255));
}
//--------------------------------------------------------------------------------
void Visuals::Player::RenderArmour()
{
	int armour = ctx.pl->m_ArmorValue();
	if (armour > 100)
		armour = 100;

	if (armour < 0)
		return;
	float box_h = (float)fabs(ctx.bbox.bottom - ctx.bbox.top);
	float off = 2;

	auto height = box_h - (((box_h * armour) / 100));

	int green = int(armour * 2.55f);
	int red = 255 - green;

	int x = ctx.bbox.right + off;
	int y = ctx.bbox.top;
	int w = 4;
	int h = box_h;

	g_VGuiSurface->DrawSetColor(Color(25, 25, 25, 100));
	g_VGuiSurface->DrawFilledRect(x, y, x + w, y + h);

	g_VGuiSurface->DrawSetColor(Color(0, 172, 235, 100));
	g_VGuiSurface->DrawOutlinedRect(x + 1, y + height + 1, x + w - 1, y + h - 1);



	std::string hp = std::to_string(armour);
	auto name = (armour < 0.f) ? "%d" : hp;
	auto sz = g_namefont->CalcTextSizeA(14.f, FLT_MAX, 0.0f, name.c_str());
	if (armour != 100)
	Render::Get().RenderText(name, ImVec2(x - w, y + height), 14.f, Color(255, 255, 255, 255));



}
void Visuals::Player::renderammo()// ignore this just saved some stuff to it unless i have time to actually make one
{


	//int armour = ctx.pl->max();
	//if (armour > 100)
	//	armour = 100;
	//float box_h = (float)fabs(ctx.bbox.left - ctx.bbox.right);
	//float off = 4;
	////	float box_h2 = (float)fabs(ctx.feet_pos.x - ctx.feet_pos.y);
	//auto height = box_h - (((box_h * armour) / 100));

	//int green = int(armour * 2.55f);
	//int red = 255 - green;

	//int x = ctx.bbox.left - off;
	//int y = ctx.bbox.bottom;
	//int w = 4;
	//int h = box_h;
	//auto height2 = box_h - (((box_h * armour) / 100));
	//g_VGuiSurface->DrawSetColor(0, 0, 0, 100);
	////	g_VGuiSurface->DrawFilledRect(x, y, x + w, y + h);
	//g_VGuiSurface->DrawFilledRect(x, y, x + box_h + 1, y + w);
	////g_VGuiSurface->DrawFilledRect(ctx.feet_pos.x, ctx.feet_pos.y,  ctx.feet_pos.x + w, ctx.feet_pos.y );


	//g_VGuiSurface->DrawSetColor(Color(0, 149, 255, 100));
	////g_VGuiSurface->DrawFilledRect(x, y, x + height2 + 1, y + w);
	//g_VGuiSurface->DrawFilledRect(x + height2, y, x + height2 + 1, y + w);
	////g_VGuiSurface->DrawOutlinedRect(x + height, y + 1, x + w - 1, y + height - 1);




	//std::string hp = std::to_string(armour);
	//auto name = (armour < 0.f) ? "%d" : hp;
	//auto sz = g_namefont->CalcTextSizeA(14.f, FLT_MAX, 0.0f, name.c_str());
	////if (armour != 100)
	//Render::Get().RenderText(name, ImVec2(x - w, y + height), 14.f, Color(255, 255, 255, 255));



}

//--------------------------------------------------------------------------------
//void Visuals::Player::RenderWeaponName()
//{
//	auto weapon = ctx.pl->m_hActiveWeapon().Get();
//
//	if (!weapon) return;
//	if (!weapon->GetCSWeaponData()) return;
//
//
//	auto text = weapon->GetCSWeaponData()->szWeaponName + 7;
//	auto sz = g_namefont->CalcTextSizeA(14.f, FLT_MAX, 0.0f, text);
//	Render::Get().RenderText(text, ctx.feet_pos.x, ctx.feet_pos.y, 14.f, ctx.clr, true,
//		g_namefont);
//}

void Visuals::Player::RenderWeaponName()
{
	C_BaseCombatWeapon* Weapon = ctx.pl->m_hActiveWeapon();
	if (!Weapon && g_EngineClient->IsConnected() && g_EngineClient->IsInGame())
		return;
	if (!Weapon->GetCSWeaponData())
		return;

	std::string WeaponName = std::string(Weapon->GetCSWeaponData()->szHudName + std::string("(") + std::to_string(Weapon->m_iClip1()) + std::string("/") + std::to_string(Weapon->m_iPrimaryReserveAmmoCount()) + std::string(")"));
	WeaponName.erase(0, 13);
	int TextWidth, TextHeight;
	Renders::Get().TextSize(TextWidth, TextHeight, WeaponName.c_str(), Renders::Get().VerdanaBold12);
	Renders::Get().Text(ctx.bbox.left + (ctx.bbox.right - ctx.bbox.left) / 2, ctx.bbox.bottom - 1, WeaponName.c_str(), Renders::Get().VerdanaBold12, Color(255, 255, 255, 255), true);


}


//--------------------------------------------------------------------------------
void Visuals::Player::RenderSnapline()
{

	int screen_w, screen_h;
	g_EngineClient->GetScreenSize(screen_w, screen_h);

	Render::Get().RenderLine(screen_w / 2.f, (float)screen_h,
		ctx.feet_pos.x, ctx.feet_pos.y, ctx.clr);
}
void Visuals::CrosshairRecoil()
{
	static auto crosshair_recoil = g_CVar->FindVar("cl_crosshair_recoil");

	if (!g_Options.esp_crosshair)
		crosshair_recoil->SetValue(0);
	else
		crosshair_recoil->SetValue(1);

}//--------------------------------------------------------------------------------



void Visuals::RenderCrosshair()
{
	static auto crosshair = g_CVar->FindVar("weapon_debug_spread_show");
	if (g_LocalPlayer->m_bIsScoped() || !g_Options.esp_crosshair)
		crosshair->SetValue(0);
	else
		crosshair->SetValue(3);

}
//--------------------------------------------------------------------------------
void Visuals::RenderWeapon(C_BaseCombatWeapon* ent)
{
	auto clean_item_name = [](const char* name) -> const char* {
		if (name[0] == 'C')
			name++;

		auto start = strstr(name, "Weapon");
		if (start != nullptr)
			name = start + 6;

		return name;
	};

	// We don't want to Render weapons that are being held
	if (ent->m_hOwnerEntity().IsValid())
		return;

	auto bbox = GetBBox(ent);

	if (bbox.right == 0 || bbox.bottom == 0)
		return;

	Render::Get().RenderBox(bbox, g_Options.color_esp_weapons);


	auto name = clean_item_name(ent->GetClientClass()->m_pNetworkName);

	auto sz = g_namefont->CalcTextSizeA(14.f, FLT_MAX, 0.0f, name);
	int w = bbox.right - bbox.left;


	Render::Get().RenderText(name, ImVec2((bbox.left + w * 0.5f) - sz.x * 0.5f, bbox.bottom + 1), 14.f, g_Options.color_esp_weapons);
}
//--------------------------------------------------------------------------------
//void Visuals::Spectators()
//{
//	if (!g_EngineClient->IsConnected() && !g_EngineClient->IsInGame())
//		return;
//
//	int spectator_index = 0;
//	int width, height;
//	g_EngineClient->GetScreenSize(width, height);
//
//	Render::Get().draw_text(width - 80, height / 2 - 10, 0, "Spectators", true, Color(255, 255, 255));
//	for (int i = 0; i < g_EntityList->GetHighestEntityIndex(); i++) {
//		auto local_player = reinterpret_cast<C_BasePlayer*>(g_EntityList->GetClientEntity(g_EngineClient->GetLocalPlayer()));
//
//		if (!local_player)
//			return;
//
//		auto entity = reinterpret_cast<C_BasePlayer*>(g_EntityList->GetClientEntity(i));
//
//		if (!entity)
//			return;
//
//		player_info_t info;
//
//		if (entity && entity != local_player) {
//			g_EngineClient->GetPlayerInfo(i, &info);
//			if (!entity->IsAlive() && !entity->IsDormant()) {
//				auto target = entity->m_hObserverTarget();
//
//				if (!target)
//					return;
//
//				if (target) {
//					auto spectator_target = g_EntityList->GetClientEntityFromHandle(target);
//					if (spectator_target == local_player) {
//						std::string player_name = info.szName;
//						std::transform(player_name.begin(), player_name.end(), player_name.begin(), ::tolower);
//						player_info_t spectator_info;
//						g_EngineClient->GetPlayerInfo(i, &spectator_info);
//						Render::Get().draw_text(width - 80, height / 2 + (10 * spectator_index), 0, player_name.c_str(), true, Color(255, 255, 255));
//						spectator_index++;
//					}
//				}
//			}
//		}
//	}
//}

//--------------------------------------------------------------------------------
void Visuals::ScopeLine()
{
	if (g_LocalPlayer->m_bIsScoped())
	{
		int w, h;
		g_EngineClient->GetScreenSize(w, h);
		Render::Get().RenderLine(w / 2, 0, w / 2, h, Color(0, 0, 0, 255), 1.f);
		Render::Get().RenderLine(0, h / 2, w, h / 2, Color(0, 0, 0, 255), 1.f);
	}
}
//--------------------------------------------------------------------------------
void Visuals::NightMode()
{
	static bool done = false;
	static float r;
	static float g;
	static float b;

	if (!g_EngineClient->IsInGame() || !g_EngineClient->IsConnected())
		done = false;

	if (g_Options.misc_nightmode)
	{
		if (!done)
		{
			static auto ragdoll = g_CVar->FindVar("phys_pushscale");
			static auto postprocess = g_CVar->FindVar("mat_postprocess_enable");
			static auto sv_sky3d = g_CVar->FindVar("r_3dsky");
			static auto sv_skyname = g_CVar->FindVar("sv_skyname");
			static auto r_DrawSpecificStaticProp = g_CVar->FindVar("r_DrawSpecificStaticProp");

			ragdoll->SetValue(1000);
			postprocess->SetValue(0);
			sv_sky3d->SetValue(0);
			r_DrawSpecificStaticProp->SetValue(1);
			sv_skyname->SetValue("sky_csgo_night02");


			for (MaterialHandle_t i = g_MatSystem->FirstMaterial(); i != g_MatSystem->InvalidMaterial(); i = g_MatSystem->NextMaterial(i))
			{
				IMaterial* pMaterial = g_MatSystem->GetMaterial(i);

				if (!pMaterial)
					continue;

				const char* group = pMaterial->GetTextureGroupName();
				const char* name = pMaterial->GetName();

				if (strstr(pMaterial->GetTextureGroupName(), TEXTURE_GROUP_SKYBOX)) {
					pMaterial->ColorModulate(1.f, 0.f, 0.f);
				}				
				if (strstr(group, TEXTURE_GROUP_WORLD))
				{
					pMaterial->GetColorModulation(&r, &g, &b);
					pMaterial->ColorModulate(0.2f, 0.2f, 0.2f);
				}
				if (strstr(group, "StaticProp"))
				{
					pMaterial->GetColorModulation(&r, &g, &b);
					pMaterial->ColorModulate(0.6f, 0.6f, 0.6f);
				}
				if (strstr(name, "models/props/de_dust/palace_bigdome"))
				{
					pMaterial->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);
				}
				if (strstr(name, "models/props/de_dust/palace_pillars"))
				{
					pMaterial->GetColorModulation(&r, &g, &b);
					pMaterial->ColorModulate(0.2f, 0.2f, 0.2f);
				}

				if (strstr(group, TEXTURE_GROUP_PARTICLE))
				{
					pMaterial->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);
				}
				done = true;
			}

		}
	}
	else
	{
		if (done)
		{
			for (MaterialHandle_t i = g_MatSystem->FirstMaterial(); i != g_MatSystem->InvalidMaterial(); i = g_MatSystem->NextMaterial(i))
			{
				IMaterial* pMaterial = g_MatSystem->GetMaterial(i);

				if (!pMaterial)
					continue;

				const char* group = pMaterial->GetTextureGroupName();
				const char* name = pMaterial->GetName();

				if (strstr(pMaterial->GetTextureGroupName(), TEXTURE_GROUP_SKYBOX)) {
					pMaterial->ColorModulate(1, 1, 1);
				}
				if (strstr(group, TEXTURE_GROUP_WORLD))
				{
					pMaterial->ColorModulate(1, 1, 1);
				}
				if (strstr(group, "StaticProp"))
				{
					pMaterial->ColorModulate(1, 1, 1);
				}
				if (strstr(name, "models/props/de_dust/palace_bigdome"))
				{
					pMaterial->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, false);
				}
				if (strstr(name, "models/props/de_dust/palace_pillars"))
				{
					pMaterial->ColorModulate(1, 1, 1);
				}
				if (strstr(group, TEXTURE_GROUP_PARTICLE))
				{
					pMaterial->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, false);
				}
			}
			done = false;
		}

	}
}
//--------------------------------------------------------------------------------
void Visuals::RenderDefuseKit(C_BaseEntity* ent)
{
	if (ent->m_hOwnerEntity().IsValid())
		return;

	auto bbox = GetBBox(ent);

	if (bbox.right == 0 || bbox.bottom == 0)
		return;

	Render::Get().RenderBox(bbox, g_Options.color_esp_defuse);

	auto name = "Defuse Kit";
	auto sz = g_namefont->CalcTextSizeA(14.f, FLT_MAX, 0.0f, name);
	int w = bbox.right - bbox.left;
	Render::Get().RenderText(name, ImVec2((bbox.left + w * 0.5f) - sz.x * 0.5f, bbox.bottom + 1), 14.f, g_Options.color_esp_defuse);
}
//--------------------------------------------------------------------------------
void Visuals::RenderPlantedC4(C_BaseEntity* ent)
{
	auto bbox = GetBBox(ent);

	if (bbox.right == 0 || bbox.bottom == 0)
		return;


	Render::Get().RenderBox(bbox, g_Options.color_esp_c4);


	int bombTimer = std::ceil(ent->m_flC4Blow() - g_GlobalVars->curtime);
	std::string timer = std::to_string(bombTimer);

	auto name = (bombTimer < 0.f) ? "Bomb" : timer;
	auto sz = g_namefont->CalcTextSizeA(14.f, FLT_MAX, 0.0f, name.c_str());
	int w = bbox.right - bbox.left;

	Render::Get().RenderText(name, ImVec2((bbox.left + w * 0.5f) - sz.x * 0.5f, bbox.bottom + 1), 14.f, g_Options.color_esp_c4);
}
//--------------------------------------------------------------------------------
void Visuals::RenderItemEsp(C_BaseEntity* ent)
{
	std::string itemstr = "Undefined";
	const model_t * itemModel = ent->GetModel();
	if (!itemModel)
		return;
	studiohdr_t * hdr = g_MdlInfo->GetStudiomodel(itemModel);
	if (!hdr)
		return;
	itemstr = hdr->szName;
	if (ent->GetClientClass()->m_ClassID == ClassId_CBumpMine)
		itemstr = "";
	else if (itemstr.find("case_pistol") != std::string::npos)
		itemstr = "Pistol Case";
	else if (itemstr.find("case_light_weapon") != std::string::npos)
		itemstr = "Light Case";
	else if (itemstr.find("case_heavy_weapon") != std::string::npos)
		itemstr = "Heavy Case";
	else if (itemstr.find("case_explosive") != std::string::npos)
		itemstr = "Explosive Case";
	else if (itemstr.find("case_tools") != std::string::npos)
		itemstr = "Tools Case";
	else if (itemstr.find("random") != std::string::npos)
		itemstr = "Airdrop";
	else if (itemstr.find("dz_armor_helmet") != std::string::npos)
		itemstr = "Full Armor";
	else if (itemstr.find("dz_helmet") != std::string::npos)
		itemstr = "Helmet";
	else if (itemstr.find("dz_armor") != std::string::npos)
		itemstr = "Armor";
	else if (itemstr.find("upgrade_tablet") != std::string::npos)
		itemstr = "Tablet Upgrade";
	else if (itemstr.find("briefcase") != std::string::npos)
		itemstr = "Briefcase";
	else if (itemstr.find("parachutepack") != std::string::npos)
		itemstr = "Parachute";
	else if (itemstr.find("dufflebag") != std::string::npos)
		itemstr = "Cash Dufflebag";
	else if (itemstr.find("ammobox") != std::string::npos)
		itemstr = "Ammobox";
	else if (itemstr.find("dronegun") != std::string::npos)
		itemstr = "Turrel";
	else if (itemstr.find("exojump") != std::string::npos)
		itemstr = "Exojump";
	else if (itemstr.find("healthshot") != std::string::npos)
		itemstr = "Healthshot";
	else {
		/*May be you will search some missing items..*/
		/*static std::vector<std::string> unk_loot;
		if (std::find(unk_loot.begin(), unk_loot.end(), itemstr) == unk_loot.end()) {
			Utils::ConsolePrint(itemstr.c_str());
			unk_loot.push_back(itemstr);
		}*/
		return;
	}
	
	auto bbox = GetBBox(ent);
	if (bbox.right == 0 || bbox.bottom == 0)
		return;
	auto sz = g_namefont->CalcTextSizeA(14.f, FLT_MAX, 0.0f, itemstr.c_str());
	int w = bbox.right - bbox.left;


	//Render::Get().RenderBox(bbox, g_Options.color_esp_item);
	Render::Get().RenderText(itemstr, ImVec2((bbox.left + w * 0.5f) - sz.x * 0.5f, bbox.bottom + 1), 14.f, g_Options.color_esp_item);
}
//--------------------------------------------------------------------------------
void Visuals::ThirdPerson() {
	if (!g_LocalPlayer)
		return;

	if (g_Options.misc_thirdperson && g_LocalPlayer->IsAlive())
	{
		if (!g_Input->m_fCameraInThirdPerson)
		{
			g_Input->m_fCameraInThirdPerson = true;
		}

		float dist = g_Options.misc_thirdperson_dist;

		QAngle *view = g_LocalPlayer->GetVAngles();
		trace_t tr;
		Ray_t ray;

		Vector desiredCamOffset = Vector(cos(DEG2RAD(view->yaw)) * dist,
			sin(DEG2RAD(view->yaw)) * dist,
			sin(DEG2RAD(-view->pitch)) * dist
		);

		//cast a ray from the Current camera Origin to the Desired 3rd person Camera origin
		ray.Init(g_LocalPlayer->GetEyePos(), (g_LocalPlayer->GetEyePos() - desiredCamOffset));
		CTraceFilter traceFilter;
		traceFilter.pSkip = g_LocalPlayer;
		g_EngineTrace->TraceRay(ray, MASK_SHOT, &traceFilter, &tr);

		Vector diff = g_LocalPlayer->GetEyePos() - tr.endpos;

		float distance2D = sqrt(abs(diff.x * diff.x) + abs(diff.y * diff.y));// Pythagorean

		bool horOK = distance2D > (dist - 2.0f);
		bool vertOK = (abs(diff.z) - abs(desiredCamOffset.z) < 3.0f);

		float cameraDistance;

		if (horOK && vertOK)  // If we are clear of obstacles
		{
			cameraDistance = dist; // go ahead and set the distance to the setting
		}
		else
		{
			if (vertOK) // if the Vertical Axis is OK
			{
				cameraDistance = distance2D * 0.95f;
			}
			else// otherwise we need to move closer to not go into the floor/ceiling
			{
				cameraDistance = abs(diff.z) * 0.95f;
			}
		}
		g_Input->m_fCameraInThirdPerson = true;

		g_Input->m_vecCameraOffset.z = cameraDistance;
	}
	else
	{
		g_Input->m_fCameraInThirdPerson = false;
	}
}

void Visuals::AddToDrawList() {
	for (auto i = 1; i <= g_EntityList->GetHighestEntityIndex(); ++i) {
		auto entity = C_BaseEntity::GetEntityByIndex(i);

		if (!entity)
			continue;
		
		if (entity == g_LocalPlayer && !g_Input->m_fCameraInThirdPerson)
			continue;

		if (i <= g_GlobalVars->maxClients) {
			auto player = Player();
			if (player.Begin((C_BasePlayer*)entity)) {

				if (!g_Options.misc_espdeath)
				{				
						if (g_Options.esp_player_snaplines) player.RenderSnapline();
						if (g_Options.esp_player_boxes)     player.RenderBox();
						if (g_Options.esp_player_boxes)   player.RenderWeaponName();
						if (g_Options.esp_player_boxes)     player.RenderName();
						if (g_Options.esp_player_boxes)    player.RenderHealth();
						if (g_Options.esp_player_boxes)    player.RenderArmour();					

				}
				else if (g_Options.misc_espdeath)
				{
					if (!g_LocalPlayer->IsAlive())
					{
						if (g_Options.esp_player_snaplines) player.RenderSnapline();
						if (g_Options.esp_player_boxes)     player.RenderBox();
						if (g_Options.esp_player_boxes)   player.RenderWeaponName();
						if (g_Options.esp_player_boxes)     player.RenderName();
						if (g_Options.esp_player_boxes)    player.RenderHealth();
						if (g_Options.esp_player_boxes)    player.RenderArmour();
					}
				}

			}
		}
		else if (g_Options.esp_dropped_weapons && entity->IsWeapon())
			RenderWeapon(static_cast<C_BaseCombatWeapon*>(entity));
		else if (g_Options.esp_dropped_weapons && entity->IsDefuseKit())
			RenderDefuseKit(entity);
		else if (entity->IsPlantedC4() && g_Options.esp_planted_c4)
			RenderPlantedC4(entity);
		else if (entity->IsLoot() && g_Options.esp_items)
			RenderItemEsp(entity);
	}

	if (g_Options.esp_crosshair)
		RenderCrosshair();



	
}

enum FontRenderFlag_t
{
	FONT_LEFT = 0,
	FONT_RIGHT = 1,
	FONT_CENTER = 2
};

void Visuals::DrawString(unsigned long font, int x, int y, Color color, unsigned long alignment, const char* msg, ...)
{

	va_list va_alist;
	char buf[1024];
	va_start(va_alist, msg);
	_vsnprintf(buf, sizeof(buf), msg, va_alist);
	va_end(va_alist);
	wchar_t wbuf[1024];
	MultiByteToWideChar(CP_UTF8, 0, buf, 256, wbuf, 256);

	int r = 255, g = 255, b = 255, a = 255;
	color.GetColor(r, g, b, a);

	int width, height;
	g_VGuiSurface->GetTextSize(font, wbuf, width, height);

	if (alignment & FONT_RIGHT)
		x -= width;
	if (alignment & FONT_CENTER)
		x -= width / 2;

	g_VGuiSurface->DrawSetTextFont(font);
	g_VGuiSurface->DrawSetTextColor(r, g, b, a);
	g_VGuiSurface->DrawSetTextPos(x, y - height / 2);
	g_VGuiSurface->DrawPrintText(wbuf, wcslen(wbuf));
}

void SpecListStyle()
{
	ImVec4* colorss = ImGui::GetStyle().Colors;
	colorss[ImGuiCol_WindowBg].w = 1;
	colorss[ImGuiCol_TitleBg] = ImColor(21, 21, 21, 255);
	colorss[ImGuiCol_TitleBgCollapsed] = ImColor(21, 21, 21, 255);
	colorss[ImGuiCol_TitleBgActive] = ImColor(21, 21, 21, 255);

}

void Visuals::Spectators() {
	if (g_Options.misc_spectator)
	{
		int cnt = 0;
		for (int i = 1; i <= g_EntityList->GetHighestEntityIndex(); i++)
		{

			C_BasePlayer* player = C_BasePlayer::GetPlayerByIndex(i);

			if (!player || player == nullptr)
				continue;

			player_info_t player_info;
			if (player != g_LocalPlayer)
			{
				if (g_EngineClient->GetPlayerInfo(i, &player_info) && !player->IsAlive() && !player->IsDormant())
				{
					auto observer_target = player->m_hObserverTarget();
					if (!observer_target)
						continue;

					auto target = observer_target.Get();
					if (!target)
						continue;

					SpecListStyle();

					player_info_t player_info2;
					if (g_EngineClient->GetPlayerInfo(target->EntIndex(), &player_info2))
					{
						char player_name[255] = { 0 };
						sprintf_s(player_name, "%s -> %s", player_info.szName, player_info2.szName);


						ImGui::SetNextWindowSize(ImVec2(300, 160));
						ImGui::Begin("Spectator List", g_Options.misc_spectator, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
						{
							ImVec2 siz = ImGui::CalcTextSize(player_name);


							if (target->EntIndex() == g_LocalPlayer->EntIndex())
							{
								ImGui::TextColored(ImVec4(1.00f, 0.00f, 0.23f, 1.f), player_name);
							}
							else
							{
								ImGui::Text(player_name);
							}

						}ImGui::End();

					}
					int w, h;
					++cnt;
				}
			}
			else
			{

				ImGui::SetNextWindowSize(ImVec2(300, 160));
				ImGui::Begin("Spectator List", g_Options.misc_spectator, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
				{


				}ImGui::End();

			}

		}
	}
}
