#include "csgostructs.hpp"
#include "../Helpers/Math.hpp"
#include "../Helpers/Utils.hpp"

bool C_BaseEntity::IsPlayer()
{
	return CallVFunction<bool(__thiscall*)(C_BaseEntity*)>(this, 157)(this);
}

bool C_BaseEntity::IsLoot() {
	return GetClientClass()->m_ClassID == ClassId_CPhysPropAmmoBox ||
		GetClientClass()->m_ClassID == ClassId_CPhysPropLootCrate ||
		GetClientClass()->m_ClassID == ClassId_CPhysPropRadarJammer ||
		GetClientClass()->m_ClassID == ClassId_CPhysPropWeaponUpgrade ||
		GetClientClass()->m_ClassID == ClassId_CDrone ||
		GetClientClass()->m_ClassID == ClassId_CDronegun ||
		GetClientClass()->m_ClassID == ClassId_CItem_Healthshot ||
		GetClientClass()->m_ClassID == ClassId_CItemCash ||
		GetClientClass()->m_ClassID == ClassId_CBumpMine;
}

bool C_BaseEntity::IsWeapon()
{
	return CallVFunction<bool(__thiscall*)(C_BaseEntity*)>(this, 165)(this);
}


bool C_BaseEntity::IsPlantedC4()
{
	return GetClientClass()->m_ClassID == ClassId_CPlantedC4;
}

bool C_BaseEntity::IsDefuseKit()
{
	return GetClientClass()->m_ClassID == ClassId_CBaseAnimating;
}


bool C_BaseCombatWeapon::HasBullets()
{
	return !IsReloading() && m_iClip1() > 0;
}

bool C_BaseCombatWeapon::CanFire()
{
	auto owner = this->m_hOwnerEntity().Get();
	if (!owner)
		return false;

	if (IsReloading() || m_iClip1() <= 0)
		return false;

	auto flServerTime = owner->m_nTickBase() * g_GlobalVars->interval_per_tick;

	if (owner->m_flNextAttack() > flServerTime)
		return false;


	return m_flNextPrimaryAttack() <= flServerTime;
}

bool C_BaseCombatWeapon::IsGrenade()
{
	return GetCSWeaponData()->iWeaponType == WEAPONTYPE_GRENADE;
}

bool C_BaseCombatWeapon::IsZeus()
{
	return this->m_Item().m_iItemDefinitionIndex() == WEAPON_TASER;
}

bool C_BaseCombatWeapon::IsGun()
{
	switch (GetCSWeaponData()->iWeaponType)
	{
	case WEAPONTYPE_C4:
		return false;
	case WEAPONTYPE_GRENADE:
		return false;
	case WEAPONTYPE_KNIFE:
		return false;
	case WEAPONTYPE_UNKNOWN:
		return false;
	default:
		return true;
	}
}

bool C_BaseCombatWeapon::IsKnife()
{
	if (this->m_Item().m_iItemDefinitionIndex() == WEAPON_TASER) return false;
	return GetCSWeaponData()->iWeaponType == WEAPONTYPE_KNIFE;
}

bool C_BaseCombatWeapon::IsAWPScout()
{
	return (this->m_Item().m_iItemDefinitionIndex() == WEAPON_AWP ||
		this->m_Item().m_iItemDefinitionIndex() == WEAPON_SSG08);
}

bool C_BaseCombatWeapon::IsRifle()
{
	switch (GetCSWeaponData()->iWeaponType)
	{
	case WEAPONTYPE_RIFLE:
		return true;
	case WEAPONTYPE_SUBMACHINEGUN:
		return true;
	case WEAPONTYPE_SHOTGUN:
		return true;
	case WEAPONTYPE_MACHINEGUN:
		return true;
	default:
		return false;
	}
}

bool C_BaseCombatWeapon::IsPistol()
{
	switch (GetCSWeaponData()->iWeaponType)
	{
	case WEAPONTYPE_PISTOL:
		return true;
	default:
		return false;
	}
}

bool C_BaseCombatWeapon::IsSniper()
{
	switch (GetCSWeaponData()->iWeaponType)
	{
	case WEAPONTYPE_SNIPER_RIFLE:
		return true;
	default:
		return false;
	}
}

bool C_BaseCombatWeapon::IsReloading()
{
	static auto inReload = *(uint32_t*)(Utils::PatternScan(GetModuleHandleW(L"client.dll"), "C6 87 ? ? ? ? ? 8B 06 8B CE FF 90") + 2);
	return *(bool*)((uintptr_t)this + inReload);
}

CCSWeaponInfo* C_BaseCombatWeapon::GetCSWeaponData()
{
	return CallVFunction<CCSWeaponInfo* (__thiscall*)(void*)>(this, 460)(this);
}

float C_BaseCombatWeapon::GetInaccuracy()
{
	return CallVFunction<float(__thiscall*)(void*)>(this, 482)(this);
}

float C_BaseCombatWeapon::GetSpread()
{
	return CallVFunction<float(__thiscall*)(void*)>(this, 452)(this);
}

void C_BaseCombatWeapon::UpdateAccuracyPenalty()
{
	CallVFunction<void(__thiscall*)(void*)>(this, 483)(this);
}

CUtlVector<IRefCounted*>& C_BaseCombatWeapon::m_CustomMaterials()
{
	static auto inReload = *(uint32_t*)(Utils::PatternScan(GetModuleHandleW(L"client.dll"), "83 BE ? ? ? ? ? 7F 67") + 2) - 12;
	return *(CUtlVector<IRefCounted*>*)((uintptr_t)this + inReload);
}

bool* C_BaseCombatWeapon::m_bCustomMaterialInitialized()
{
	static auto currentCommand = *(uint32_t*)(Utils::PatternScan(GetModuleHandleW(L"client.dll"), "C6 86 ? ? ? ? ? FF 50 04") + 2);
	return (bool*)((uintptr_t)this + currentCommand);
}

CUserCmd*& C_BasePlayer::m_pCurrentCommand()
{
	static auto currentCommand = *(uint32_t*)(Utils::PatternScan(GetModuleHandleW(L"client.dll"), "89 BE ? ? ? ? E8 ? ? ? ? 85 FF") + 2);
	return *(CUserCmd**)((uintptr_t)this + currentCommand);
}
int& C_BasePlayer::get_take_damage() {
	return *reinterpret_cast<int*>(uintptr_t(this) + 0x280);
}
int C_BasePlayer::GetNumAnimOverlays()
{
	return *(int*)((DWORD)this + 0x298C);
}
float C_BasePlayer::m_flOldSimulationTime()
{
	return *reinterpret_cast<float*>(uintptr_t(this) + NetvarSys::Get().GetOffset("DT_BaseEntity", "m_flSimulationTime") + 0x4);
}
AnimationLayer* C_BasePlayer::GetAnimOverlays()
{
	return *(AnimationLayer**)((DWORD)this + 0x2980);
}

AnimationLayer* C_BasePlayer::GetAnimOverlay(int i)
{
	if (i < 15)
		return &GetAnimOverlays()[i];
	return nullptr;
}
void C_BasePlayer::SetAbsAngles(QAngle angle)
{
	using SetAbsAnglesFn = void(__thiscall*)(void*, const QAngle& Angles);
	static SetAbsAnglesFn SetAbsAngles = (SetAbsAnglesFn)Utils::PatternScan(GetModuleHandleA("client.dll"), "55 8B EC 83 E4 F8 83 EC 64 53 56 57 8B F1 E8");
	SetAbsAngles(this, angle);
}
int C_BasePlayer::GetSequenceActivity(int sequence)
{
	auto hdr = g_MdlInfo->GetStudiomodel(this->GetModel());

	if (!hdr)
		return -1;

	// sig for stuidohdr_t version: 53 56 8B F1 8B DA 85 F6 74 55
	// sig for C_BaseAnimating version: 55 8B EC 83 7D 08 FF 56 8B F1 74 3D
	// c_csplayer vfunc 242, follow calls to find the function.

	static auto get_sequence_activity = reinterpret_cast<int(__fastcall*)(void*, studiohdr_t*, int)>(Utils::PatternScan(GetModuleHandle(L"client.dll"), "55 8B EC 53 8B 5D 08 56 8B F1 83"));

	return get_sequence_activity(this, hdr, sequence);
}
float C_BasePlayer::MaxDesyncDelta() {

	auto animstate = uintptr_t(this->GetPlayerAnimState());

	float duckammount = *(float*)(animstate + 0xA4);
	float speedfraction = std::fmax(0, std::fmin(*reinterpret_cast<float*>(animstate + 0xF8), 1));

	float speedfactor = std::fmax(0, std::fmin(1, *reinterpret_cast<float*> (animstate + 0xFC)));

	float unk1 = ((*reinterpret_cast<float*> (animstate + 0x11C) * -0.30000001) - 0.19999999) * speedfraction;
	float unk2 = unk1 + 1.f;
	float unk3;

	if (duckammount > 0) {

		unk2 += ((duckammount * speedfactor) * (0.5f - unk2));

	}

	unk3 = *(float*)(animstate + 0x334) * unk2;

	return unk3;
}
CBasePlayerAnimState* C_BasePlayer::GetPlayerAnimState()
{
	static auto m_bIsScoped = NetvarSys::Get().GetOffset("DT_CSPlayer", "m_bIsScoped");
	return *(CBasePlayerAnimState**)((uintptr_t)this + m_bIsScoped - 0x14);//"DT_CSPlayer", "m_bIsScoped" - 0x14
}

void C_BasePlayer::UpdateAnimationState(CBasePlayerAnimState* state, QAngle angle)
{
	if (!state)
		return;

	static auto UpdateAnimState = Utils::PatternScan(
		GetModuleHandleW(L"client.dll"), "55 8B EC 83 E4 F8 83 EC 18 56 57 8B F9 F3 0F 11 54 24");

	if (!UpdateAnimState)
		return;

	__asm {
		push 0
		mov ecx, state
		movss xmm1, dword ptr[angle + 4]
		movss xmm2, dword ptr[angle]

		call UpdateAnimState
	}
}

void C_BasePlayer::ResetAnimationState(CBasePlayerAnimState* state)
{
	if (!state)
		return;

	using ResetAnimState_t = void(__thiscall*)(CBasePlayerAnimState*);
	static auto ResetAnimState = (ResetAnimState_t)Utils::PatternScan(
		GetModuleHandleW(L"client.dll"), "56 6A 01 68 ? ? ? ? 8B F1");
	if (!ResetAnimState)
		return;

	ResetAnimState(state);
}

void C_BasePlayer::CreateAnimationState(CBasePlayerAnimState* state)
{
	using CreateAnimState_t = void(__thiscall*)(CBasePlayerAnimState*, C_BasePlayer*);
	static auto CreateAnimState = (CreateAnimState_t)Utils::PatternScan(GetModuleHandleA("client.dll"), "55 8B EC 56 8B F1 B9 ? ? ? ? C7 46");
	if (!CreateAnimState)
		return;

	CreateAnimState(state, this);
}

Vector C_BasePlayer::GetEyePos()
{
	return m_vecOrigin() + m_vecViewOffset();
}

player_info_t C_BasePlayer::GetPlayerInfo()
{
	player_info_t info;
	g_EngineClient->GetPlayerInfo(EntIndex(), &info);
	return info;
}

bool C_BasePlayer::IsAlive()
{
	return this->m_iHealth() > 0;
}

bool C_BasePlayer::IsFlashed()
{
	static auto m_flFlashMaxAlpha = NetvarSys::Get().GetOffset("DT_CSPlayer", "m_flFlashMaxAlpha");
	return *(float*)((uintptr_t)this + m_flFlashMaxAlpha - 0x8) > 200.0;
}

bool C_BasePlayer::HasC4()
{
	static auto fnHasC4
		= reinterpret_cast<bool(__thiscall*)(void*)>(
			Utils::PatternScan(GetModuleHandleW(L"client.dll"), "56 8B F1 85 F6 74 31")
			);

	return fnHasC4(this);
}

Vector C_BasePlayer::GetHitboxPos(int hitbox_id)
{
	matrix3x4_t boneMatrix[MAXSTUDIOBONES];

	if (SetupBones(boneMatrix, MAXSTUDIOBONES, BONE_USED_BY_HITBOX, 0.0f)) {
		auto studio_model = g_MdlInfo->GetStudiomodel(GetModel());
		if (studio_model) {
			auto hitbox = studio_model->GetHitboxSet(0)->GetHitbox(hitbox_id);
			if (hitbox) {
				auto
					min = Vector{},
					max = Vector{};

				Math::VectorTransform(hitbox->bbmin, boneMatrix[hitbox->bone], min);
				Math::VectorTransform(hitbox->bbmax, boneMatrix[hitbox->bone], max);

				return (min + max) / 2.0f;
			}
		}
	}
	return Vector{};
}
Vector C_BasePlayer::GetHitboxPos(int hitbox_id, matrix3x4_t* boneMatrix)
{
	auto studio_model = g_MdlInfo->GetStudiomodel(GetModel());
	if (studio_model) {
		auto hitbox = studio_model->GetHitboxSet(0)->GetHitbox(hitbox_id);
		if (hitbox) {
			auto
				min = Vector{},
				max = Vector{};

			Math::VectorTransform(hitbox->bbmin, boneMatrix[hitbox->bone], min);
			Math::VectorTransform(hitbox->bbmax, boneMatrix[hitbox->bone], max);

			return (min + max) / 2.0f;
		}
	}
	return Vector{};
}
Vector C_BasePlayer::GetHitboxPos(int hitbox_id, matrix3x4_t* boneMatrix, float& Radius, Vector& Min, Vector& Max, int& Bone)
{
	auto studio_model = g_MdlInfo->GetStudiomodel(GetModel());
	if (studio_model) {
		auto hitbox = studio_model->GetHitboxSet(0)->GetHitbox(hitbox_id);
		if (hitbox) {
			auto
				min = Vector{},
				max = Vector{};

			Math::VectorTransform(hitbox->bbmin, boneMatrix[hitbox->bone], min);
			Math::VectorTransform(hitbox->bbmax, boneMatrix[hitbox->bone], max);

			Radius = hitbox->m_flRadius;
			Min = hitbox->bbmin;
			Max = hitbox->bbmax;
			Bone = hitbox->bone;

			return (min + max) / 2.0f;
		}
	}
	return Vector{};
}
mstudiobbox_t* C_BasePlayer::GetHitbox(int hitbox_id)
{
	matrix3x4_t boneMatrix[MAXSTUDIOBONES];

	if (SetupBones(boneMatrix, MAXSTUDIOBONES, BONE_USED_BY_HITBOX, 0.0f)) {
		auto studio_model = g_MdlInfo->GetStudiomodel(GetModel());
		if (studio_model) {
			auto hitbox = studio_model->GetHitboxSet(0)->GetHitbox(hitbox_id);
			if (hitbox) {
				return hitbox;
			}
		}
	}
	return nullptr;
}

bool C_BasePlayer::GetHitboxPos(int hitbox, Vector& output)
{
	if (hitbox >= HITBOX_MAX)
		return false;

	const model_t* model = this->GetModel();
	if (!model)
		return false;

	studiohdr_t* studioHdr = g_MdlInfo->GetStudiomodel(model);
	if (!studioHdr)
		return false;

	matrix3x4_t matrix[MAXSTUDIOBONES];
	if (!this->SetupBones(matrix, MAXSTUDIOBONES, 0x100, 0))
		return false;

	mstudiobbox_t* studioBox = studioHdr->GetHitboxSet(0)->GetHitbox(hitbox);
	if (!studioBox)
		return false;

	Vector min, max;

	Math::VectorTransform(studioBox->bbmin, matrix[studioBox->bone], min);
	Math::VectorTransform(studioBox->bbmax, matrix[studioBox->bone], max);

	output = (min + max) * 0.5f;

	return true;
}

Vector C_BasePlayer::GetBonePos(int bone)
{
	matrix3x4_t boneMatrix[MAXSTUDIOBONES];
	if (SetupBones(boneMatrix, MAXSTUDIOBONES, BONE_USED_BY_ANYTHING, 0.0f)) {
		return boneMatrix[bone].at(3);
	}
	return Vector{};
}

Vector C_BasePlayer::RotatedBonePos(int bone, float rotation)
{
	Vector bone_pos;
	Vector origin;
	Vector bone_offset;

	bone_pos = GetHitboxPos(bone);
	origin = m_vecOrigin();
	bone_offset = bone_pos - origin;

	float radius = bone_offset.Length2D();
	float radian = rotation * 57.295779513082f;

	bone_offset.x = cos(radian) * radius;
	bone_offset.y = sin(radian) * radius;

	bone_offset += origin;

	return bone_offset;
}
bool C_BasePlayer::CanSeePlayer(C_BasePlayer* player, int hitbox)
{
	CGameTrace tr;
	Ray_t ray;
	CTraceFilter filter;
	filter.pSkip = this;

	auto endpos = player->GetHitboxPos(hitbox);

	ray.Init(GetEyePos(), endpos);
	g_EngineTrace->TraceRay(ray, MASK_SHOT | CONTENTS_GRATE, &filter, &tr);

	return tr.hit_entity == player || tr.fraction > 0.97f;
}

bool C_BasePlayer::CanSeePlayer(C_BasePlayer* player, const Vector& pos)
{
	CGameTrace tr;
	Ray_t ray;
	CTraceFilter filter;
	filter.pSkip = this;

	ray.Init(GetEyePos(), pos);
	g_EngineTrace->TraceRay(ray, MASK_SHOT | CONTENTS_GRATE, &filter, &tr);

	return tr.hit_entity == player || tr.fraction > 0.97f;
}

void C_BasePlayer::UpdateClientSideAnimation()
{
	return CallVFunction<void(__thiscall*)(void*)>(this, 223)(this);
}

void C_BasePlayer::InvalidateBoneCache()
{
	static DWORD addr = (DWORD)Utils::PatternScan(GetModuleHandleA("client.dll"), "80 3D ? ? ? ? ? 74 16 A1 ? ? ? ? 48 C7 81");

	unsigned long g_iModelBoneCounter = **(unsigned long**)(addr + 10);
	*(unsigned int*)((DWORD)this + 0x2924) = 0xFF7FFFFF; // m_flLastBoneSetupTime = -FLT_MAX;
	*(unsigned int*)((DWORD)this + 0x2690) = (g_iModelBoneCounter - 1); // m_iMostRecentModelBoneCounter = g_iModelBoneCounter - 1;
}

int C_BasePlayer::m_nMoveType()
{
	return *(int*)((uintptr_t)this + 0x25C);
}

QAngle* C_BasePlayer::GetVAngles()
{
	static auto deadflag = NetvarSys::Get().GetOffset("DT_BasePlayer", "deadflag");
	return (QAngle*)((uintptr_t)this + deadflag + 0x4);
}

void C_BaseAttributableItem::SetGloveModelIndex(int modelIndex)
{
	return CallVFunction<void(__thiscall*)(void*, int)>(this, 75)(this, modelIndex);
}

void C_BaseViewModel::SendViewModelMatchingSequence(int sequence)
{
	return CallVFunction<void(__thiscall*)(void*, int)>(this, 244)(this, sequence);
}

float_t C_BasePlayer::m_flSpawnTime()
{
	return *(float_t*)((uintptr_t)this + 0xA360);
}
