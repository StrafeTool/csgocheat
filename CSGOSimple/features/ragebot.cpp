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
bool IsTickValid(float SimulationTime, float MaxTime)
{
	INetChannelInfo* NetChannelInfo = g_EngineClient->GetNetChannelInfo();
	if (!NetChannelInfo) return true;
	float Correct = 0;
	Correct += NetChannelInfo->GetLatency(FLOW_OUTGOING);
	Correct += NetChannelInfo->GetLatency(FLOW_INCOMING);
	Correct += GetLerpTime();

	std::clamp(Correct, 0.f, g_CVar->FindVar("sv_maxunlag")->GetFloat());

	float DeltaTime = Correct - (g_GlobalVars->curtime - SimulationTime);

	float TimeLimit = MaxTime;
	std::clamp(TimeLimit, 0.001f, 0.2f);

	if (fabsf(DeltaTime) > TimeLimit/*float(Variables.LegitBacktrackDuration) / 1000.f  0.2f*/)
		return false;

	return true;
}






void RageAimbot::StoreRecords()
{
	for (int i = 1; i <= 64; i++)
	{
		C_BasePlayer* Player = C_BasePlayer::GetPlayerByIndex(i);
		if (!Player ||
			Player->IsDormant() ||
			!Player->IsPlayer() ||
			!Player->IsAlive() ||
			!Player->IsEnemy())
		{
			if (BacktrackRecords[i].size() > 0)
				for (int Tick = 0; Tick < BacktrackRecords[i].size(); Tick++)
					BacktrackRecords[i].erase(BacktrackRecords[i].begin() + Tick);
			continue;
		}

		BacktrackRecords[i].insert(BacktrackRecords[i].begin(), TickInfo(Player));
		for (auto Tick : BacktrackRecords[i])
			if (!IsTickValid(Tick.SimulationTime, 0.2f))
				BacktrackRecords[i].pop_back();
	}
}
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
void RageAimbot::Do(CUserCmd* cmd, C_BaseCombatWeapon* Weapon, bool& bSendPacket)
{
	if (!g_EngineClient->IsConnected() && g_EngineClient->IsInGame())
		return;
	if (!g_LocalPlayer ||
		!g_LocalPlayer->IsAlive() ||
		!Weapon ||
		Weapon->IsKnife() ||
		Weapon->IsGrenade() ||
		!g_Options.rage_enable)
		return;

	int BestTargetIndex = -1;
	float BestTargetDistance = FLT_MAX;
	float BestTargetSimtime = 0.f;
	Vector Hitbox = Vector{};

	bool Backtrack = false;

	for (int i = 1; i <= 64; i++)
	{
		C_BasePlayer* Player = C_BasePlayer::GetPlayerByIndex(i);
		if (!Player ||
			!Player->IsPlayer() ||
			Player->IsDormant() ||
			!Player->IsAlive() ||
			!Player->IsEnemy() ||
			BacktrackRecords[i].size() < 1)
			continue;


		float PlayerDistance = Math::VectorDistance(g_LocalPlayer->m_vecOrigin(), Player->m_vecOrigin());
		if (BestTargetDistance > PlayerDistance)
		{
			if (BacktrackRecords[i].front().MatrixBuilt && BacktrackRecords[i].front().BoneMatrix != nullptr &&
				Hitscan(Player, Hitbox, false, BacktrackRecords[i].front().BoneMatrix))
			{
				BestTargetDistance = PlayerDistance;
				BestTargetIndex = i;
				BestTargetSimtime = Player->m_flSimulationTime();
				Backtrack = false;
			}
			else if (BacktrackRecords[i].back().MatrixBuilt && BacktrackRecords[i].back().BoneMatrix != nullptr &&
				Hitscan(Player, Hitbox, true, BacktrackRecords[i].back().BoneMatrix))
			{
				BestTargetDistance = PlayerDistance;
				BestTargetIndex = i;
				BestTargetSimtime = BacktrackRecords[i].back().SimulationTime;
				Backtrack = true;
			}
		}
	}
	if (BestTargetIndex != -1 && Hitbox.IsValid() && BestTargetSimtime)
	{
		C_BasePlayer* Target = C_BasePlayer::GetPlayerByIndex(BestTargetIndex);
		if (!Target) return;
		QAngle AimAngle = Math::CalcAngle(g_LocalPlayer->GetEyePos(), Hitbox);
		AimAngle -= g_LocalPlayer->m_aimPunchAngle() * g_CVar->FindVar("weapon_recoil_scale")->GetFloat();
		Math::Normalize3(AimAngle);
		Math::ClampAngles(AimAngle);

		cmd->viewangles = AimAngle;

		if (Hitchance(Weapon, cmd->viewangles, Target, float(g_Options.RageAimbotHitchance)) ||
			Backtrack && g_Options.RageAimbotHitchance * 1.5 <= Hitchance2(Weapon) ||
			g_Options.RageAimbotHitchance == 0)
		{
			if (!(cmd->buttons & IN_ATTACK) && Weapon->CanFire())
			{
				bSendPacket = true;
				cmd->tick_count = TIME_TO_TICKS(BestTargetSimtime + GetLerpTime());
				cmd->buttons |= IN_ATTACK;
			}
		}
	}
}