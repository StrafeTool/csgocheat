#include "ragebot.h"
#include "../hooks.hpp"
#include "autowall.h"
float flOldCurtime;
float flOldFrametime;

void MovementFix::Start(CUserCmd* cmd)
{
	m_oldangle = cmd->viewangles;
	m_oldforward = cmd->forwardmove;
	m_oldsidemove = cmd->sidemove;
}

void MovementFix::End(CUserCmd* cmd)
{
	float yaw_delta = cmd->viewangles.yaw - m_oldangle.yaw;
	float f1;
	float f2;

	if (m_oldangle.yaw < 0.f)
		f1 = 360.0f + m_oldangle.yaw;
	else
		f1 = m_oldangle.yaw;

	if (cmd->viewangles.yaw < 0.0f)
		f2 = 360.0f + cmd->viewangles.yaw;
	else
		f2 = cmd->viewangles.yaw;

	if (f2 < f1)
		yaw_delta = abs(f2 - f1);
	else
		yaw_delta = 360.0f - abs(f1 - f2);
	yaw_delta = 360.0f - yaw_delta;

	cmd->forwardmove = cos(DEG2RAD(yaw_delta)) * m_oldforward + cos(DEG2RAD(yaw_delta + 90.f)) * m_oldsidemove;
	cmd->sidemove = sin(DEG2RAD(yaw_delta)) * m_oldforward + sin(DEG2RAD(yaw_delta + 90.f)) * m_oldsidemove;
}

void RageAimbot::StartEnginePred(CUserCmd* cmd)
{
	static int nTickBase;
	static CUserCmd* pLastCmd;

	// fix tickbase if game didnt render previous tick
	if (pLastCmd)
	{
		if (pLastCmd->hasbeenpredicted)
			nTickBase = g_LocalPlayer->m_nTickBase();
		else
			++nTickBase;
	}

	pLastCmd = cmd;
	flOldCurtime = g_GlobalVars->curtime;
	flOldFrametime = g_GlobalVars->frametime;

	g_GlobalVars->curtime = nTickBase * g_GlobalVars->interval_per_tick;
	g_GlobalVars->frametime = g_GlobalVars->interval_per_tick;

	g_GameMovement->StartTrackPredictionErrors(g_LocalPlayer);

	CMoveData data;
	memset(&data, 0, sizeof(CMoveData));

	g_MoveHelper->SetHost(g_LocalPlayer);
	g_Prediction->SetupMove(g_LocalPlayer, cmd, g_MoveHelper, &data);
	g_GameMovement->ProcessMovement(g_LocalPlayer, &data);
	g_Prediction->FinishMove(g_LocalPlayer, cmd, &data);
}

void RageAimbot::EndEnginePred()
{
	g_GameMovement->FinishTrackPredictionErrors(g_LocalPlayer);
	g_MoveHelper->SetHost(nullptr);

	g_GlobalVars->curtime = flOldCurtime;
	g_GlobalVars->frametime = flOldFrametime;
}





bool RageAimbot::Hitchance(C_BaseCombatWeapon* weapon, QAngle angles, C_BasePlayer* ent, float chance) //pasted
{
	Vector forward, right, up;
	Vector src = g_LocalPlayer->GetEyePos();
	Math::AngleVectors(angles, forward, right, up);

	int cHits = 0;
	int cNeededHits = static_cast<int> (150.f * (chance / 100.f));

	weapon->UpdateAccuracyPenalty();
	float weap_spread = weapon->GetSpread();
	float weap_inaccuracy = weapon->GetInaccuracy();

	for (int i = 0; i < 150; i++)
	{
		float a = Math::RandomFloat(0.f, 1.f);
		float b = Math::RandomFloat(0.f, 2.f * M_PI);
		float c = Math::RandomFloat(0.f, 1.f);
		float d = Math::RandomFloat(0.f, 2.f * M_PI);

		float inaccuracy = a * weap_inaccuracy;
		float spread = c * weap_spread;

		if (weapon->m_Item().m_iItemDefinitionIndex() == 64)
		{
			a = 1.f - a * a;
			a = 1.f - c * c;
		}

		Vector spreadView((cos(b) * inaccuracy) + (cos(d) * spread), (sin(b) * inaccuracy) + (sin(d) * spread), 0), direction;

		direction.x = forward.x + (spreadView.x * right.x) + (spreadView.y * up.x);
		direction.y = forward.y + (spreadView.x * right.y) + (spreadView.y * up.y);
		direction.z = forward.z + (spreadView.x * right.z) + (spreadView.y * up.z);
		direction.Normalized();

		QAngle viewAnglesSpread;
		Math::VectorAngles(direction, up, viewAnglesSpread);
		viewAnglesSpread.Normalize();

		Vector viewForward;
		Math::AngleVectors(viewAnglesSpread, viewForward);
		viewForward.NormalizeInPlace();

		viewForward = src + (viewForward * weapon->GetCSWeaponData()->flRange);

		trace_t tr;
		Ray_t ray;

		ray.Init(src, viewForward);
		g_EngineTrace->ClipRayToEntity(ray, MASK_SHOT | CONTENTS_GRATE, ent, &tr);

		if (tr.hit_entity == ent)
			++cHits;

		if (static_cast<int> ((static_cast<float> (cHits) / 150.f) * 100.f) >= chance)
			return true;

		if ((150 - i + cHits) < cNeededHits)
			return false;
	}

	return false;
}

bool RageAimbot::Hitscan(C_BasePlayer* Player, Vector& HitboxPos, bool Backtrack, matrix3x4_t* BoneMatrix)
{
	std::vector<int> HitBoxesToScan{ 0,1,2,3,4,5,6, HITBOX_LEFT_FOOT, HITBOX_RIGHT_FOOT };

	int bestHitbox = -1;
	if (!Backtrack)
	{
		float highestDamage;

		highestDamage = g_Options.RageAimbotMinDmg;

		for (auto HitBoxID : HitBoxesToScan)
		{
			Player->SetAbsOrigin(Player->m_vecOrigin());
			Vector Point = Player->GetHitboxPos2(HitBoxID, BoneMatrix);
			float damage = Autowall::Get().CanHit(Point);
			if (damage >= highestDamage || damage >= Player->m_iHealth())
			{
				bestHitbox = HitBoxID;
				highestDamage = damage;
				HitboxPos = Point;
				return true;
			}
		}
	}
	else //didnt autowall the backtrackz coz my pc is slow af and it goes skra
	{
		for (auto HitBoxID : HitBoxesToScan)
		{
			//Player->SetAbsOrigin(BacktrackRecords[Player->EntIndex()].back().Origin);
			Vector Point = Player->GetHitboxPos2(HitBoxID, BoneMatrix);
			if (g_LocalPlayer->CanSeePlayer(Player, Point))
			{
				bestHitbox = HitBoxID;
				HitboxPos = Point;
				return true;
			}
		}
	}
	return false;
}


float GetLerpTime()
{
	int ud_rate = g_CVar->FindVar("cl_updaterate")->GetInt();
	ConVar* min_ud_rate = g_CVar->FindVar("sv_minupdaterate");
	ConVar* max_ud_rate = g_CVar->FindVar("sv_maxupdaterate");

	if (min_ud_rate && max_ud_rate)
		ud_rate = max_ud_rate->GetInt();

	float ratio = g_CVar->FindVar("cl_interp_ratio")->GetFloat();

	if (ratio == 0)
		ratio = 1.0f;

	float lerp = g_CVar->FindVar("cl_interp")->GetFloat();
	ConVar* c_min_ratio = g_CVar->FindVar("sv_client_min_interp_ratio");
	ConVar* c_max_ratio = g_CVar->FindVar("sv_client_max_interp_ratio");

	if (c_min_ratio && c_max_ratio && c_min_ratio->GetFloat() != 1)
		ratio = std::clamp(ratio, c_min_ratio->GetFloat(), c_max_ratio->GetFloat());

	return std::max(lerp, (ratio / ud_rate));
}

bool IsTickValid(int tick, CUserCmd* cmd) // gucci i think cant remember
{
	auto nci = g_EngineClient->GetNetChannelInfo();

	if (!nci)
		return false;

	auto PredictedCmdArrivalTick = cmd->tick_count + 1 + TIME_TO_TICKS(nci->GetAvgLatency(FLOW_INCOMING) + nci->GetAvgLatency(FLOW_OUTGOING));
	auto Correct = std::clamp(GetLerpTime() + nci->GetLatency(FLOW_OUTGOING), 0.f, 1.f) - TICKS_TO_TIME(PredictedCmdArrivalTick + TIME_TO_TICKS(GetLerpTime()) - (tick + TIME_TO_TICKS(GetLerpTime())));

	return (abs(Correct) <= 0.2f);
}




void RageAimbot::StoreRecords2(C_BasePlayer* ent)
{
	PlayerRecords Setup;
	static float ShotTime[65];
	static float OldSimtime[65];
	
	for (int i = 1; i <= 64; i++)
	{
		C_BasePlayer* Player = C_BasePlayer::GetPlayerByIndex(i);
		if (ent != g_LocalPlayer)
			ent->FixSetupBones(Matrix[ent->EntIndex()]);

		if (BacktrackRecords[i].size() > 0)
		{
			Setup.Velocity = abs(ent->m_vecVelocity().Length2D());
			Setup.SimTime = ent->m_flSimulationTime();
			memcpy(Setup.Matrix, Matrix[ent->EntIndex()], (sizeof(matrix3x4_t) * 128));
			Setup.Shot = false;
			BacktrackRecords[ent->EntIndex()].push_back(Setup);
		}
		if (OldSimtime[ent->EntIndex()] != ent->m_flSimulationTime())
		{
			Setup.Velocity = abs(ent->m_vecVelocity().Length2D());

			Setup.SimTime = ent->m_flSimulationTime();

			Setup.m_vecAbsOrigin = ent->GetAbsOrigin();

			if (ent == g_LocalPlayer)
				ent->FixSetupBones(Matrix[ent->EntIndex()]);

			memcpy(Setup.Matrix, Matrix[ent->EntIndex()], (sizeof(matrix3x4_t) * 128));

			

			BacktrackRecords[ent->EntIndex()].push_back(Setup);

			OldSimtime[ent->EntIndex()] = ent->m_flSimulationTime();
		}

	}

}
void RageAimbot::ClearRecords(int i)
{
	if (BacktrackRecords[i].size() > 0)
	{
		for (int tick = 0; tick < BacktrackRecords[i].size(); tick++)
		{
			BacktrackRecords[i].erase(BacktrackRecords[i].begin() + tick);
		}
	}
}



//void RageAimbot::StoreRecords()
//{
//	for (int i = 1; i <= 64; i++)
//	{
//		C_BasePlayer* Player = C_BasePlayer::GetPlayerByIndex(i);
//		if (!Player ||
//			Player->IsDormant() ||
//			!Player->IsPlayer() ||
//			!Player->IsAlive() ||
//			!Player->IsEnemy())
//		{
//			if (BacktrackRecords[i].size() > 0)
//				for (int Tick = 0; Tick < BacktrackRecords[i].size(); Tick++)
//					BacktrackRecords[i].erase(BacktrackRecords[i].begin() + Tick);
//			continue;
//		}
//
//		BacktrackRecords[i].insert(BacktrackRecords[i].begin(), TickInfo(Player));
//		for (auto Tick : BacktrackRecords[i])
//			if (!IsTickValid(Tick.SimulationTime, 0.2f))
//				BacktrackRecords[i].pop_back();
//	}
//}
float Hitchance2(C_BaseCombatWeapon* Weapon)
// coz i need to restore shit for the proper hitchance to work with backtrack
{
	float Hitchance = 101;
	if (!Weapon) return 0;
	if (g_Options.RageAimbotHitchance > 1)
	{
		float Inaccuracy = Weapon->GetInaccuracy();
		if (Inaccuracy == 0) Inaccuracy = 0.0000001;
		Inaccuracy = 1 / Inaccuracy;
		Hitchance = Inaccuracy;

	}
	return Hitchance;
}



void RageAimbot::Autostop(CUserCmd* cmd)
{
	

	//cmd->viewangles.clamp();

	auto vel2 = g_LocalPlayer->m_vecVelocity();
	const auto speed = vel2.Length();
	if (speed > 15.f)
	{
		Vector dir;
		//Math::VectorAngles(vel2, &dir);
		dir.y = cmd->viewangles.yaw - dir.y;

		Vector new_move;
		//Math::AngleVectors(dir, &new_move);
		const auto max = std::max(std::fabs(cmd->forwardmove), std::fabs(cmd->sidemove));
		const auto mult = 450.f / max;
		new_move *= -mult;

		cmd->forwardmove = new_move.x;
		cmd->sidemove = new_move.y;
	}
	else
	{
		cmd->forwardmove = 0.f;
		cmd->sidemove = 0.f;
	}
}

void RageAimbot::Do(CUserCmd* cmd, C_BaseCombatWeapon* Weapon, bool& bSendPacket)
{
	for (int i = 1; i <= g_EngineClient->GetMaxClients(); ++i)
	{
		C_BasePlayer* Players = C_BasePlayer::GetPlayerByIndex(i);
		if (!g_EngineClient->IsConnected() && g_EngineClient->IsInGame())
			return;
		if (!g_LocalPlayer ||
			!g_LocalPlayer->IsAlive() ||
			!Weapon ||
			Weapon->IsKnife() ||
			Weapon->IsGrenade() ||
			!g_Options.rage_enable)
		{
			ClearRecords(i);
		}
		return;
		StoreRecords2(Players);
		EnemyEyeAngs[i] = Players->eyeangles();

		if (BacktrackRecords[i].size() == 0 || !g_LocalPlayer->IsAlive())
			continue;


		if (!g_LocalPlayer->m_hActiveWeapon() || Weapon->IsGrenade())
			continue;

		bestdamage = 0;

	//	Vector Hitbox = Hitscan(Players);

	}
}