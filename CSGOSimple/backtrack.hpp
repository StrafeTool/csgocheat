#pragma once
#include "singleton.hpp"
#include "valve_sdk/csgostructs.hpp"
#include "helpers/math.hpp"
#include "options.hpp"
#include <deque>
#include "valve_sdk/math/Vector.hpp"
#include "valve_sdk/math/VMatrix.hpp"

#define NUM_OF_TICKS 26 

struct StoredData
{
	float simtime;
	Vector hitboxPos;
	matrix3x4_t boneMatrix[MAXSTUDIOBONES];
	float Velocity;
	float SimTime;
	bool Shot;
	Vector m_vecAbsOrigin;


};
class TimeWarp : public Singleton<TimeWarp>
{
	int nLatestTick;
	
public:
	StoredData TimeWarpData[64][NUM_OF_TICKS];
	std::deque<StoredData> PlayerRecord[65] = {  };
	//std::vector<StoredData> BacktrackRecords[65];
	int bestTargetSimTime;
	matrix3x4_t Matrixs[65][128];
	void CreateMove(CUserCmd* cmd);
};

class LegitBacktrack :
	public Singleton<LegitBacktrack>
{
	friend class Singleton<LegitBacktrack>;
private:
	struct TickInfo
	{
		TickInfo()
		{
			HeadPosition = Vector{};
			SimulationTime = -1;
			MatrixBuilt = false;
		}
		TickInfo(C_BasePlayer* Player)
		{
			HeadPosition = Player->GetHitboxPos(HITBOX_HEAD);
			SimulationTime = Player->m_flSimulationTime();
			MatrixBuilt = false;
			if (Player->SetupBones(BoneMatrix, 128, BONE_USED_BY_HITBOX, g_GlobalVars->curtime))
				MatrixBuilt = true;
		}
		Vector HeadPosition;
		float SimulationTime;
		bool MatrixBuilt;
		matrix3x4_t BoneMatrix[128];
	};
public:
	void Do(CUserCmd* cmd);
	std::vector<TickInfo> BacktrackRecords[65];
	int ClosestTick;
};

class LegitAimbot :
	public Singleton<LegitAimbot>
{
	friend class Singleton<LegitAimbot>;
private:
	int GetHitboxFromInt(int Hitbox);
public:
	
	void Do(CUserCmd* cmd, C_BaseCombatWeapon* Weapon);
};