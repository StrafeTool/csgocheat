#pragma once
#include "../options.hpp"
#include "../valve_sdk/csgostructs.hpp"
#include "../helpers/math.hpp"
#include "../helpers/utils.hpp"
#include "../singleton.hpp"
#include "../helpers/input.hpp"
#include <deque>

class MovementFix : public Singleton<MovementFix>
{
	friend class Singleton<MovementFix>;
public:
	void Start(CUserCmd* cmd);
	void End(CUserCmd* cmd);
private:
	float m_oldforward, m_oldsidemove;
	QAngle m_oldangle;
};

struct PlayerRecords
{
	matrix3x4_t Matrix[128];
	float Velocity;
	float SimTime;
	bool Shot;
	Vector m_vecAbsOrigin;
};

class RageAimbot :
	public Singleton<RageAimbot>
{
	friend class Singleton<RageAimbot>;
private:
	struct TickInfo
	{
		TickInfo()
		{
			SimulationTime = 0.f;
			Origin = Vector{};
			MatrixBuilt = false;
		}
		TickInfo(C_BasePlayer* Player)
		{
			SimulationTime = Player->m_flSimulationTime();
			Origin = Player->m_vecOrigin();
			MatrixBuilt = false;
			if (Player->SetupBones(BoneMatrix, 128, BONE_USED_BY_HITBOX, g_GlobalVars->curtime))
				MatrixBuilt = true;
		}
		float SimulationTime;
		Vector Origin;
		bool MatrixBuilt;
	
		matrix3x4_t BoneMatrix[128];
	};
	bool Hitscan(C_BasePlayer* Player, Vector& HitboxPos, bool Backtrack, matrix3x4_t* BoneMatrix);
	void StoreRecords(const ClientFrameStage_t stage);
	bool Hitchance(C_BaseCombatWeapon* weapon, QAngle angles, C_BasePlayer* ent, float chance);
	Vector         EnemyEyeAngs[65];
public:
	float bestdamage;
	void StoreRecords();
	void StoreRecords2(C_BasePlayer* ent);
	void ClearRecords(int i);
	//std::vector<TickInfo> BacktrackRecords[65];
	std::deque<PlayerRecords> BacktrackRecords[65] = {  };
	void StartEnginePred(CUserCmd* cmd);
	void EndEnginePred();
	matrix3x4_t Matrix[65][128];
	void Autostop(CUserCmd* cmd);
	void Do(CUserCmd* cmd, C_BaseCombatWeapon* Weapon, bool& bSendPacket);
	void DoAntiaim(CUserCmd* cmd, C_BaseCombatWeapon* Weapon, bool& bSendPacket);
};