#include "hitmarker.hpp"
#include "features/visuals.hpp"
#include "options.hpp"
#pragma comment(lib, "winmm.lib")
#include "hooks.hpp"

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
