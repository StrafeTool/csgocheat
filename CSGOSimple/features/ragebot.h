#pragma once
#include "../options.hpp"
#include "../valve_sdk/csgostructs.hpp"
#include "../helpers/math.hpp"
#include "../helpers/utils.hpp"
#include "../singleton.hpp"
#include "../helpers/input.hpp"
#include "../hooks.hpp"
#include <deque>

struct LayerRecord
{
	LayerRecord()
	{
		m_nOrder = 0;
		m_nSequence = 0;
		m_flWeight = 0.f;
		m_flCycle = 0.f;
	}

	LayerRecord(const LayerRecord& src)
	{
		m_nOrder = src.m_nOrder;
		m_nSequence = src.m_nSequence;
		m_flWeight = src.m_flWeight;
		m_flCycle = src.m_flCycle;
	}

	uint32_t m_nOrder;
	uint32_t m_nSequence;
	float_t m_flWeight;
	float_t m_flCycle;
};
struct TickInfo
{
	TickInfo()
	{
		HeadPosition.Init();
		SimulationTime = -1.f;
		Origin.Init();
		Angles.Init();
		Mins.Init();
		Maxs.Init();
		Velocity.Init();
		MatrixBuilt = false;
	}
	TickInfo(C_BasePlayer* Player)
	{
		HeadPosition = Player->GetHitboxPos(HITBOX_HEAD);
		SimulationTime = Player->m_flSimulationTime();

		Origin = Player->m_vecOrigin();
		Angles = Player->m_angEyeAngles();
		Mins = Player->GetCollideable()->OBBMins();
		Maxs = Player->GetCollideable()->OBBMaxs();
		Flags = Player->m_fFlags();
		Velocity = Player->m_vecVelocity();

		int layerCount = Player->GetNumAnimOverlays();
		for (int i = 0; i < layerCount; i++)
		{
			AnimationLayer* currentLayer = Player->GetAnimOverlay(i);
			LayerRecords[i].m_nOrder = currentLayer->m_nOrder;
			LayerRecords[i].m_nSequence = currentLayer->m_nSequence;
			LayerRecords[i].m_flWeight = currentLayer->m_flWeight;
			LayerRecords[i].m_flCycle = currentLayer->m_flCycle;
		}
		PoseParams = Player->m_flPoseParameter();


		MatrixBuilt = false;
		if (Player->SetupBones(BoneMatrix, 128, BONE_USED_BY_ANYTHING, g_GlobalVars->curtime))
			MatrixBuilt = true;
	}
	Vector HeadPosition;
	float SimulationTime;
	int32_t Flags;
	Vector Origin;
	QAngle Angles;
	Vector Mins;
	Vector Maxs;
	Vector Velocity;

	std::array<float_t, 24> PoseParams;
	std::array<LayerRecord, 15> LayerRecords;

	bool MatrixBuilt;
	matrix3x4_t BoneMatrix[128];
};


class MovementFix : public Singleton<MovementFix>
{
	friend class Singleton<MovementFix>;
public:
	void Start(CUserCmd* cmd);
	void End(CUserCmd* cmd);
	float m_oldforward, m_oldsidemove;
	QAngle m_oldangle;
};
class RageAimbot :
	public Singleton<RageAimbot>
{
	friend class Singleton<RageAimbot>;
private:
	bool ShouldBaim(C_BasePlayer* Player);
	void GetMultipointPositions(C_BasePlayer* Player, std::vector<Vector>& Positions, int HitboxIndex, float Scale, matrix3x4_t* BoneMatrix);
	bool Hitscan(C_BasePlayer* Player, Vector& HitboxPos, matrix3x4_t* BoneMatrix, bool Backtrack, TickInfo Record);
	bool Hitchance(C_BasePlayer* Player, C_BaseCombatWeapon* pWeapon, QAngle Angle, Vector Point, int Chance);
//	void WeaponSettings(C_BaseCombatWeapon* Weapon);
	float HitchanceValue;
	float MinDmgValue;
	bool UseFreestand[65];
	float FreestandAngle[65];
	float LastFreestandAngle[65];
	bool ForceSafePoint[65];
public:

	QAngle RealAngle;
	QAngle FakeAngle;
	bool SendPacket;
	int TickCount;
	matrix3x4_t FakeLagMatrix[128];
	matrix3x4_t FakeAngleMatrix[128];
	Vector FakeOrigin;
	int ShotsFired[65];
	int ShotsHit[65];
	CUserCmd* cmd;
	int TickBaseShift;
	bool bInSendMove = false, bFirstSendMovePack = false;

	bool m_should_update_fake = false;
	std::array< AnimationLayer, 13 > m_fake_layers;
	std::array< float, 20 > m_fake_poses;
	CBasePlayerAnimState* m_fake_states = nullptr;
	CBasePlayerAnimState* m_fake_state = nullptr;
	float m_fake_rotation = 0.f;
	bool init_fake_anim = false;
	float m_fake_spawntime = 0.f;
	float m_fake_delta = 0.f;
	matrix3x4_t m_fake_matrix[128];
	matrix3x4_t m_fake_position_matrix[128];
	//std::array< matrix3x4_t, 128 > m_fake_matrix;
	//std::array< matrix3x4_t, 128 > m_fake_position_matrix;
	bool m_got_fake_matrix = false;
	float m_real_yaw_ang = 0.f;
	bool m_should_update_entity_animstate = true;
	float m_server_abs_rotation = 0.f;
	struct infos
	{
		std::array<float, 24> m_poses;
		AnimationLayer m_overlays;

	};

	std::vector<TickInfo> BacktrackRecords[65];
	void AntiFreestanding();
	void LocalAnimationFix(C_BasePlayer* entity);
	void Resolver(C_BasePlayer* Player, CBasePlayerAnimState* Animstate);
	void AnimationFix();
	//void LocalAnimationFix();
	void UpdateFakeAnimations();
	void StoreRecords();
	void Do(CUserCmd* cmd, C_BaseCombatWeapon* Weapon, bool& bSendPacket);
	void DoFakelag(bool& bSendPacket);
	void DoAntiaim(CUserCmd* cmd, C_BaseCombatWeapon* Weapon, bool& bSendPacket);
};