#include "hitmarker.hpp"
#include "features/visuals.hpp"
#include "options.hpp"
#pragma comment(lib, "winmm.lib")
#include "hooks.hpp"
#include "features/ragebot.h"

void HitMarkerEvent::FireGameEvent(IGameEvent* event)
{
	if (!event)
		return;

	if (g_Options.misc_hitmarker)
	{
		if (g_EngineClient->GetPlayerForUserID(event->GetInt("attacker")) == g_EngineClient->GetLocalPlayer() &&
			g_EngineClient->GetPlayerForUserID(event->GetInt("userid")) != g_EngineClient->GetLocalPlayer())
		{
			hitMarkerInfo.push_back({ g_GlobalVars->curtime + 0.8f, event->GetInt("dmg_health") });
			g_EngineClient->ExecuteClientCmd("play buttons\\arena_switch_press_02.wav"); // No other fitting sound. Probs should use a resource
		}
	}
}

int HitMarkerEvent::GetEventDebugID(void)
{
	return EVENT_DEBUG_ID_INIT;
}

void HitMarkerEvent::RegisterSelf()
{
	g_GameEvents->AddListener(this, "player_hurt", false);
	g_GameEvents->AddListener(this, "item_purchase", false);
	g_GameEvents->AddListener(this, "player_hurt", false);
	g_GameEvents->AddListener(this, "bullet_impact", false);

}

void HitMarkerEvent::UnregisterSelf()
{
	g_GameEvents->RemoveListener(this);
}

enum FontRenderFlag_t
{
	FONT_LEFT = 0,
	FONT_RIGHT = 1,
	FONT_CENTER = 2
};

void HitMarkerEvent::Paint(void)
{
	static int width = 0, height = 0;
	if (width == 0 || height == 0)
		g_EngineClient->GetScreenSize(width, height);


	if (eventInfo.size() > 15)
		eventInfo.erase(eventInfo.begin() + 1);

	float alpha = 0.f;

	if (g_Options.misc_hitmarker)
	{


		for (size_t i = 0; i < hitMarkerInfo.size(); i++)
		{

			float diff = hitMarkerInfo.at(i).m_flExpTime - g_GlobalVars->curtime;

			if (diff < 0.f)
			{
				hitMarkerInfo.erase(hitMarkerInfo.begin() + i);
				continue;
			}

			int dist = 24;

			float ratio = 0.4f - (diff / 0.8f);
			alpha = 0.8f - diff / 0.8f;

			Visuals::Get().DrawString(esp_font, width / 2 + 6 + ratio * dist / 2, height / 2 + 6 + ratio * dist, Color(255, 255, 0, (int)(alpha * 255.f)), FONT_LEFT, std::to_string(hitMarkerInfo.at(i).m_iDmg).c_str());
		}

		if (hitMarkerInfo.size() > 0)
		{
			int screenSizeX, screenCenterX;
			int screenSizeY, screenCenterY;
			g_EngineClient->GetScreenSize(screenSizeX, screenSizeY);

			screenCenterX = screenSizeX / 2;
			screenCenterY = screenSizeY / 2;

			int lineSize = 9;
			g_VGuiSurface->DrawSetColor(Color(200, 200, 190, (int)(alpha * 255.f)));
			g_VGuiSurface->DrawLine(screenCenterX - lineSize, screenCenterY - lineSize, screenCenterX - (lineSize / 4), screenCenterY - (lineSize / 4));
			g_VGuiSurface->DrawLine(screenCenterX - lineSize, screenCenterY + lineSize, screenCenterX - (lineSize / 4), screenCenterY + (lineSize / 4));
			g_VGuiSurface->DrawLine(screenCenterX + lineSize, screenCenterY + lineSize, screenCenterX + (lineSize / 4), screenCenterY + (lineSize / 4));
			g_VGuiSurface->DrawLine(screenCenterX + lineSize, screenCenterY - lineSize, screenCenterX + (lineSize / 4), screenCenterY - (lineSize / 4));

		}
	}
}
void BulletImpactEvent::log_damage(C_BasePlayer* player, int damage, int hitgroup)
{
	if (!g_Options.esp_enabled)
		return;

	std::string hitgroup_str;

	switch (hitgroup)
	{
	case HITBOX_HEAD:
	case HITBOX_NECK:
		hitgroup_str = ("HEAD");
		break;

	case HITBOX_PELVIS:
	case HITBOX_STOMACH:
	case HITBOX_LOWER_CHEST:
	case HITBOX_CHEST:
	case HITBOX_UPPER_CHEST:
		hitgroup_str = ("BODY");
		break;

	case HITBOX_RIGHT_THIGH:
	case HITBOX_LEFT_THIGH:
	case HITBOX_RIGHT_CALF:
	case HITBOX_LEFT_CALF:
	case HITBOX_RIGHT_FOOT:
	case HITBOX_LEFT_FOOT:
		hitgroup_str = ("FOOT");
		break;

	case HITBOX_RIGHT_HAND:
	case HITBOX_LEFT_HAND:
	case HITBOX_RIGHT_UPPER_ARM:
	case HITBOX_RIGHT_FOREARM:
	case HITBOX_LEFT_UPPER_ARM:
	case HITBOX_LEFT_FOREARM:
		hitgroup_str = ("ARM");
		break;
	}

	//debug_console::print(
	//	xor_str("hit player \"") + std::string(player->get_player_info().sz_name) + xor_str("\" for ") + std::to_string(damage) +
	//	xor_str(" damage in the ") + hitgroup_str);
	//logs.push_back({
	//	xor_str("hit player \"") + std::string(player->get_player_info().sz_name) + xor_str("\" for ") + std::to_string(damage) +
	//	xor_str(" damage in the ") + hitgroup_str,
	//	interfaces::global_vars->realtime
	//	});
	//logs.push_back ( { xor_str ( "hit player \"" ) + std::string ( player->get_player_info().sz_name ) + xor_str ( "\" for " ) + std::to_string ( damage ) + xor_str ( " damage in the " ) + hitgroup_str, interfaces::global_vars->realtime } );
}

void BulletImpactEvent::FireGameEvent(IGameEvent* event)
{
	if (!event)
		return;

	int iUser = g_EngineClient->GetPlayerForUserID(event->GetInt("userid"));

	auto pPlayer = reinterpret_cast<C_BasePlayer*>(g_EntityList->GetClientEntity(iUser));

	if (!pPlayer)
		return;

	if (iUser != g_EngineClient->GetLocalPlayer())
		return;

	if (pPlayer->IsDormant())
		return;

	float x, y, z;
	x = event->GetFloat("x");
	y = event->GetFloat("y");
	z = event->GetFloat("z");


	static auto last_hurt_curtime = 0.f;
	static auto last_hurt_attacker = -1;
	static auto last_hurt_userid = -1;
	static auto last_hurt_damage = -1;
	static auto last_hurt_health = -1;
	static auto last_hurt_hitgroup = 0;
	static int OldShotsFired[65];
	if (strstr(event->GetName(), "player_hurt"))
	{
		C_BasePlayer* Attacker =
			C_BasePlayer::GetPlayerByIndex(g_EngineClient->GetPlayerForUserID(event->GetInt("attacker")));

		C_BasePlayer* Victim = C_BasePlayer::GetPlayerByIndex(g_EngineClient->GetPlayerForUserID(event->GetInt("userid")));

		if (Attacker == g_LocalPlayer && Victim != g_LocalPlayer)
		{
			if (RageAimbot::Get().ShotsFired[Victim->EntIndex()] != OldShotsFired[Victim->EntIndex()])
			{
				RageAimbot::Get().ShotsHit[Victim->EntIndex()] += 1;

				OldShotsFired[Victim->EntIndex()] = RageAimbot::Get().ShotsFired[Victim->EntIndex()];
			}
		}
	}


	if (!strcmp(event->GetName(),("player_hurt")))
	{
		const auto attacker = g_EngineClient->GetPlayerForUserID(event->GetInt(("attacker")));
		const auto hurt = g_EngineClient->GetPlayerForUserID(event->GetInt(("userid")));
		const auto health = event->GetInt(("health"));
		const auto dmg_health = event->GetInt(("dmg_health"));
		const auto hitgroup = event->GetInt(("hitgroup"));
		if (last_hurt_curtime == g_GlobalVars->curtime && last_hurt_attacker == attacker
			&& last_hurt_userid == hurt && last_hurt_damage == dmg_health
			&& last_hurt_health == health && last_hurt_hitgroup == hitgroup)
			return;


		last_hurt_curtime = g_GlobalVars->curtime;
		last_hurt_attacker = attacker;
		last_hurt_userid = hurt;
		last_hurt_damage = dmg_health;
		last_hurt_health = health;
		last_hurt_hitgroup = hitgroup;

	}



}


int BulletImpactEvent::GetEventDebugID(void)
{
	return EVENT_DEBUG_ID_INIT;
}

void BulletImpactEvent::RegisterSelf()
{
	g_GameEvents->AddListener(this, "bullet_impact", false);
}

void BulletImpactEvent::UnregisterSelf()
{
	g_GameEvents->RemoveListener(this);
}
