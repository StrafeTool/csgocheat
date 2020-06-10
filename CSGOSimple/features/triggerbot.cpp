#include "Triggerbot.h"

namespace triggerbot {

	void Triggerbot(CUserCmd* pCmd) {

		if (g_LocalPlayer->IsAlive() && g_LocalPlayer->IsAlive() && !(g_LocalPlayer->m_lifeState() & LIFE_DYING))
		{
			auto pWeapon = g_LocalPlayer->m_hActiveWeapon();

			if (pWeapon)
			{
				static bool enable = false;
				static bool head = false;
				static bool arms = false;
				static bool chest = false;
				static bool stomach = false;
				static bool legs = false;

				if (pWeapon->IsPistol()) enable = true;
				else if (pWeapon->IsRifle()) enable = true;
				else if (pWeapon->IsSniper()) enable = true;

				if (pWeapon->IsPistol()) head = true;
				else if (pWeapon->IsRifle()) head = true;
				else if (pWeapon->IsSniper()) head = true;

				if (pWeapon->IsPistol()) arms = true;
				else if (pWeapon->IsRifle()) arms = true;
				else if (pWeapon->IsSniper()) arms = true;

				if (pWeapon->IsPistol()) chest = true;
				else if (pWeapon->IsRifle()) chest = true;
				else if (pWeapon->IsSniper()) chest = true;

				if (pWeapon->IsPistol()) stomach = true;
				else if (pWeapon->IsRifle()) stomach = true;
				else if (pWeapon->IsSniper()) stomach = true;

				if (pWeapon->IsPistol()) legs = true;
				else if (pWeapon->IsRifle()) legs = true;
				else if (pWeapon->IsSniper()) legs = true;

				Vector src, dst, forward;
				trace_t tr;
				Ray_t ray;
				CTraceFilter filter;

				QAngle viewangle = pCmd->viewangles;

				viewangle += g_LocalPlayer->m_aimPunchAngle() * 2.f;

				Math::AngleVectors(viewangle, forward);

				forward *= g_LocalPlayer->m_hActiveWeapon()->GetCSWeaponData()->flRange;
				filter.pSkip = g_LocalPlayer;
				src = g_LocalPlayer->GetEyePos();
				dst = src + forward;
				ray.Init(src, dst);

				g_EngineTrace->TraceRay(ray, MASK_NPCWORLDSTATIC | CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_MONSTER | CONTENTS_WINDOW | CONTENTS_DEBRIS | CONTENTS_HITBOX, &filter, &tr);

				if (!tr.hit_entity)
					return;

				int hitgroup = tr.hitgroup;
				bool didHit = false;
				if ((head && tr.hitgroup == 1)
					|| (chest && tr.hitgroup == 2)
					|| (stomach && tr.hitgroup == 3)
					|| (arms && (tr.hitgroup == 4 || tr.hitgroup == 5))
					|| (legs && (tr.hitgroup == 6 || tr.hitgroup == 7)))
				{
					didHit = true;
				}


				if (didHit && (tr.hit_entity->GetBaseEntity()->m_iTeamNum() != g_LocalPlayer->m_iTeamNum()))
				{
					if (GetAsyncKeyState(VK_XBUTTON1) && enable)
					{
						pCmd->buttons |= IN_ATTACK;
					}
				}

			}
		}
	}
}


