#include "backtrack.hpp"



#define TICK_INTERVAL			(g_GlobalVars->interval_per_tick)
#define TIME_TO_TICKS( dt )		( (int)( 0.5f + (float)(dt) / TICK_INTERVAL ) )


void TimeWarp::CreateMove(CUserCmd* cmd)
{
	/*if (!g_Options.misc_backtrack) return;*/

	int bestTargetIndex = -1;
	float bestFov = FLT_MAX;
	StoredData setupdatas;
	if (!g_LocalPlayer->IsAlive())
		return;

	for (int i = 1; i < g_EngineClient->GetMaxClients(); i++)
	{


		auto pEntity = static_cast<C_BasePlayer*>(g_EntityList->GetClientEntity(i));
		if (!pEntity || !g_LocalPlayer) continue;
		if (!pEntity->IsPlayer()) continue;
		if (pEntity == g_LocalPlayer) continue;
		if (pEntity->IsDormant()) continue;
		if (!pEntity->IsAlive()) continue;
		if (pEntity->m_iTeamNum() == g_LocalPlayer->m_iTeamNum()) continue;
		if (pEntity != g_LocalPlayer)
			pEntity->FixSetupBones(Matrixs[pEntity->EntIndex()]);


		float simtime = pEntity->m_flSimulationTime();
		//float curtime = g_GlobalVars->curtime;
		Vector hitboxPos = pEntity->GetHitboxPos(0);
	
		
		TimeWarpData[i][cmd->command_number % (static_cast<int>(NUM_OF_TICKS) + 1)] = StoredData{ simtime, hitboxPos };
		pEntity->SetupBones(TimeWarpData[i][cmd->command_number % (NUM_OF_TICKS + 1)].boneMatrix, MAXSTUDIOBONES, BONE_USED_BY_ANYTHING, simtime);
		Vector ViewDir;
		Math::AngleVectors(cmd->viewangles + (g_LocalPlayer->m_aimPunchAngle() * 2.f), ViewDir);
		float FOVDistance = Math::DistancePointToLine(hitboxPos, g_LocalPlayer->GetEyePos(), ViewDir);

		if (bestFov > FOVDistance)
		{
			bestFov = FOVDistance;
			bestTargetIndex = i;
		}


		float bestTargetSimTime = -1;
		if (bestTargetIndex != -1)
		{
			float tempFloat = FLT_MAX;
			Vector ViewDir;
			Math::AngleVectors(cmd->viewangles + (g_LocalPlayer->m_aimPunchAngle() * 2.f), ViewDir);
			for (int t = 0; t < static_cast<int>(NUM_OF_TICKS); ++t)
			{
				float tempFOVDistance = Math::DistancePointToLine(TimeWarpData[bestTargetIndex][t].hitboxPos, g_LocalPlayer->GetEyePos(), ViewDir);
				if (tempFloat > tempFOVDistance && TimeWarpData[bestTargetIndex][t].simtime > g_LocalPlayer->m_flSimulationTime() - 1)
				{
					if (g_LocalPlayer->CanSeePlayer(static_cast<C_BasePlayer*>(g_EntityList->GetClientEntity(bestTargetIndex)), TimeWarpData[bestTargetIndex][t].hitboxPos))
					{
						//auto pEntity = static_cast<C_BasePlayer*>(g_EntityList->GetClientEntity(i));
						tempFloat = tempFOVDistance;
						bestTargetSimTime = TimeWarpData[bestTargetIndex][t].simtime;
						//memcpy(setupdatas.Matrix, Matrixs[pEntity->EntIndex()], (sizeof(matrix3x4_t) * 128));
					}
				}
			}

			if (bestTargetSimTime >= 0 && cmd->buttons & IN_ATTACK)
				cmd->tick_count = TIME_TO_TICKS(bestTargetSimTime);
		}
		if (bestTargetIndex < 0)
		{
			setupdatas.Velocity = abs(pEntity->m_vecVelocity().Length2D());
			setupdatas.SimTime = pEntity->m_flSimulationTime();
			//memcpy(setupdatas.Matrix, Matrixs[pEntity->EntIndex()], (sizeof(matrix3x4_t) * 128));
		}
	}
}



float GetLerpTimes()
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
bool IsTickValids(float SimulationTime, float MaxTime)
{
	INetChannelInfo* NetChannelInfo = g_EngineClient->GetNetChannelInfo();
	if (!NetChannelInfo) return true;
	float Correct = 0;
	Correct += NetChannelInfo->GetLatency(FLOW_OUTGOING);
	Correct += NetChannelInfo->GetLatency(FLOW_INCOMING);
	Correct += GetLerpTimes();

	std::clamp(Correct, 0.f, g_CVar->FindVar("sv_maxunlag")->GetFloat());

	float DeltaTime = Correct - (g_GlobalVars->curtime - SimulationTime);

	float TimeLimit = MaxTime;
	std::clamp(TimeLimit, 0.001f, 0.2f);

	if (fabsf(DeltaTime) > TimeLimit/*float(Variables.LegitBacktrackDuration) / 1000.f  0.2f*/)
		return false;

	return true;
}


void LegitBacktrack::Do(CUserCmd* cmd)
{
	if (!g_LocalPlayer ||
		!g_LocalPlayer->IsAlive())
		return;

	int BacktrackedPlayer = -1;
	float MaxPlayerFov = INT_MAX;

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
			if (!IsTickValids(Tick.SimulationTime, float(200.f) / 1000.f))
				BacktrackRecords[i].pop_back();

		Vector ViewAngles;
		Math::AngleVectors(cmd->viewangles + (g_LocalPlayer->m_aimPunchAngle() * g_CVar->FindVar("weapon_recoil_scale")->GetFloat()), ViewAngles);
		float FovDistance = Math::DistancePointToLine(Player->GetHitboxPos(HITBOX_HEAD), g_LocalPlayer->GetEyePos(), ViewAngles);

		if (MaxPlayerFov > FovDistance)
		{
			MaxPlayerFov = FovDistance;
			BacktrackedPlayer = i;
		}
	}
	ClosestTick = -1;
	float MaxTickFov = INT_MAX;
	if (BacktrackedPlayer != -1)
	{
		for (int t = 0; t < BacktrackRecords[BacktrackedPlayer].size(); t++)
		{
			Vector ViewAngles2;
			Math::AngleVectors(cmd->viewangles + (g_LocalPlayer->m_aimPunchAngle() * g_CVar->FindVar("weapon_recoil_scale")->GetFloat()), ViewAngles2);
			float FovDistance2 = Math::DistancePointToLine(BacktrackRecords[BacktrackedPlayer].at(t).HeadPosition, g_LocalPlayer->GetEyePos(), ViewAngles2);

			if (MaxTickFov > FovDistance2)
			{
				MaxTickFov = FovDistance2;
				ClosestTick = t;
			}
		}
		if (ClosestTick != -1 && BacktrackRecords[BacktrackedPlayer].at(ClosestTick).SimulationTime != -1)
			cmd->tick_count = TIME_TO_TICKS(BacktrackRecords[BacktrackedPlayer].at(ClosestTick).SimulationTime + GetLerpTimes());
	}
}



void LegitAimbot::Do(CUserCmd* cmd, C_BaseCombatWeapon* Weapon)
{
	LegitBacktrack::Get().Do(cmd);
	TimeWarp::Get().CreateMove(cmd);
	if (!g_LocalPlayer ||
		!g_LocalPlayer->IsAlive() ||
		!Weapon ||
		Weapon->IsKnife() ||
		Weapon->IsGrenade() ||
		!g_Options.legit_enable ||
		!g_Options.legit_fov)
		return;

	float MaxPlayerFov = g_Options.legit_fov;
	int ClosestPlayerIndex = -1;
	QAngle AimAngle = QAngle{};
	QAngle ViewAngle = QAngle{};
	g_EngineClient->GetViewAngles(&ViewAngle);

	for (int i = 1; i <= 64; i++)
	{
		C_BasePlayer* Player = C_BasePlayer::GetPlayerByIndex(i);
		if (!Player ||
			!Player->IsPlayer() ||
			Player->IsDormant() ||
			!Player->IsAlive() ||
			!Player->IsEnemy())
			continue;

		Vector Hitbox;
		Hitbox = Player->GetHitboxPos(GetHitboxFromInt(g_Options.legit_hitbox));
		float FovDistance = Math::GetFOV(ViewAngle + (g_LocalPlayer->m_aimPunchAngle() * g_CVar->FindVar("weapon_recoil_scale")->GetFloat()), Math::CalcAngle(g_LocalPlayer->GetEyePos(), Hitbox));

		if (MaxPlayerFov > FovDistance)
		{
			if (g_LocalPlayer->CanSeePlayer(Player, Hitbox))
			{
				MaxPlayerFov = FovDistance;
				ClosestPlayerIndex = i;
			}
		}
	}
	if (ClosestPlayerIndex != -1)
	{
		C_BasePlayer* ClosestPlayer = C_BasePlayer::GetPlayerByIndex(ClosestPlayerIndex);
		if (!ClosestPlayer) return;
		Vector Hitbox;
		

		Hitbox = ClosestPlayer->GetHitboxPos(GetHitboxFromInt(g_Options.legit_hitbox));
		AimAngle = Math::CalcAngle(g_LocalPlayer->GetEyePos(), Hitbox);
		AimAngle -= (g_LocalPlayer->m_aimPunchAngle() * g_CVar->FindVar("weapon_recoil_scale")->GetFloat()) * (float(g_Options.LegitAimbotRcs) / 100.f);
		Math::Normalize3(AimAngle);
		Math::ClampAngles(AimAngle);
		QAngle DeltaAngle = ViewAngle - AimAngle;
		Math::Normalize3(DeltaAngle);
		Math::ClampAngles(DeltaAngle);
		float Smoothing = (/*Variables.LegitAimbotType == 1 && */g_Options.LegitAimbotSmooth > 1) ? g_Options.LegitAimbotSmooth : 1;
		QAngle FinalAngle = ViewAngle - DeltaAngle / Smoothing;
		Math::Normalize3(FinalAngle);
		Math::ClampAngles(FinalAngle);


		if (cmd->buttons & IN_ATTACK)
		{
			cmd->viewangles = FinalAngle;
			g_EngineClient->SetViewAngles(&cmd->viewangles);
		}	


	}
}
int LegitAimbot::GetHitboxFromInt(int Hitbox)
{
	if (Hitbox != HITBOX_HEAD && Hitbox != HITBOX_NECK)
		return HITBOX_CHEST;

	return Hitbox;
}