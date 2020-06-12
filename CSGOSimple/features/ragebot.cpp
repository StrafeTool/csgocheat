#include "ragebot.h"
#include "autowall.h"

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




bool RageAimbot::Hitchance(C_BasePlayer* Player, C_BaseCombatWeapon* pWeapon, QAngle Angle, Vector Point, int Chance)
{
	static float Seeds = 256.f;

	Vector forward, right, up;

	Math::AngleVectors(Angle, forward, right, up);

	int Hits = 0, neededHits = (Seeds * (Chance / 100.f));

	float weapSpread = pWeapon->GetSpread(), weapInaccuracy = pWeapon->GetInaccuracy();

	bool Return = false;

	for (int i = 0; i < Seeds; i++)
	{
		float Inaccuracy = Math::RandomFloat(0.f, 1.f) * weapInaccuracy;
		float Spread = Math::RandomFloat(0.f, 1.f) * weapSpread;

		Vector spreadView((cos(Math::RandomFloat(0.f, 2.f * M_PI)) * Inaccuracy) + (cos(Math::RandomFloat(0.f, 2.f * M_PI)) * Spread), (sin(Math::RandomFloat(0.f, 2.f * M_PI)) * Inaccuracy) + (sin(Math::RandomFloat(0.f, 2.f * M_PI)) * Spread), 0), direction;
		direction = Vector(forward.x + (spreadView.x * right.x) + (spreadView.y * up.x), forward.y + (spreadView.x * right.y) + (spreadView.y * up.y), forward.z + (spreadView.x * right.z) + (spreadView.y * up.z)).Normalize(); // guess i cant put vector in a cast *nvm im retarded

		QAngle viewanglesSpread;
		Vector viewForward;

		Math::VectorAngles(direction, up, viewanglesSpread);
		Math::Normalize3(viewanglesSpread);

		Math::AngleVectors(viewanglesSpread, viewForward);
		viewForward.NormalizeInPlace();

		viewForward = g_LocalPlayer->GetEyePos() + (viewForward * pWeapon->GetCSWeaponData()->flRange);

		trace_t Trace;
		Ray_t ray;

		ray.Init(g_LocalPlayer->GetEyePos(), viewForward);
		g_EngineTrace->ClipRayToEntity(ray, MASK_SHOT | CONTENTS_GRATE, Player, &Trace);
		//trace_t Trace;
		//g_EngineTrace->ClipRayToEntity(ray_t(g_LocalPlayer->GetEyePos(), viewForward), MASK_SHOT | CONTENTS_GRATE, Player, &Trace);

		if (Trace.hit_entity == Player)
			Hits++;

		if (((Hits / Seeds) * 100.f) >= Chance)
		{
			Return = true;
			break;
		}

		if ((Seeds - i + Hits) < neededHits)
			break;
	}

	return Return;
}
void UpdateAnimationState(CBasePlayerAnimState* state, QAngle angle)
{
	if (!state)
		return;

	static auto UpdateAnimState = Utils::PatternScan(GetModuleHandleW(L"client.dll"), "55 8B EC 83 E4 F8 83 EC 18 56 57 8B F9 F3 0F 11 54 24");//sigchange
	if (!UpdateAnimState)
		return;

	__asm
	{
		mov ecx, state

		movss xmm1, dword ptr[angle + 4]
		movss xmm2, dword ptr[angle]

		call UpdateAnimState
	}
}
void CreateAnimationState(CBasePlayerAnimState* state)
{
	using CreateAnimState_t = void(__thiscall*)(CBasePlayerAnimState*, C_BaseEntity*);
	static auto CreateAnimState = (CreateAnimState_t)Utils::PatternScan(GetModuleHandleW(L"client.dll"), "55 8B EC 56 8B F1 B9 ? ? ? ? C7 46");
	if (!CreateAnimState)
		return;

	CreateAnimState(state, g_LocalPlayer);
}
void ResetAnimationState(CBasePlayerAnimState* state)
{
	if (!state)
		return;

	using ResetAnimState_t = void(__thiscall*)(CBasePlayerAnimState*);
	static auto ResetAnimState = (ResetAnimState_t)Utils::PatternScan(GetModuleHandleW(L"client.dll"), "56 6A 01 68 ? ? ? ? 8B F1");
	if (!ResetAnimState)
		return;

	ResetAnimState(state);
}
void ResetAnimationStatereal(CBasePlayerAnimState* state)
{
	if (!state)
		return;

	using ResetAnimState_t = void(__thiscall*)(CBasePlayerAnimState*);
	static auto ResetAnimState = (ResetAnimState_t)Utils::PatternScan(GetModuleHandleW(L"client.dll"), "56 6A 01 68 ? ? ? ? 8B F1");
	if (!ResetAnimState)
		return;

	ResetAnimState(state);
}
void update_Fake_state(CBasePlayerAnimState* state, QAngle ang) {
	using fn = void(__vectorcall*)(void*, void*, float, float, float, void*);
	static auto ret = reinterpret_cast<fn>(Utils::PatternScan(GetModuleHandleW(L"client.dll"), "55 8B EC 83 E4 F8 83 EC 18 56 57 8B F9 F3 0F 11 54 24"));

	if (!ret)
		return;

	ret(state, NULL, NULL, ang.yaw, ang.pitch, NULL);
}

void RageAimbot::UpdateFakeAnimations()
{
	if (!g_LocalPlayer || !g_LocalPlayer->IsAlive())
		return;

	if (!g_Options.chams_player_backtrack)
		return;

	if (m_fake_spawntime != g_LocalPlayer->m_flSpawnTime() || m_should_update_fake)
	{
		init_fake_anim = false;
		m_fake_spawntime = g_LocalPlayer->m_flSpawnTime();
		m_should_update_fake = false;
	}

	if (!init_fake_anim)
	{
		//m_fake_state = static_cast<CCSGOPlayerAnimState*> (g_pMemAlloc->Alloc(sizeof(CCSGOPlayerAnimState)));

		if (m_fake_state != nullptr)
			CreateAnimationState(m_fake_state);

		init_fake_anim = true;
	}

	if (SendPacket)
	{
		std::array<AnimationLayer, 13> networked_layers;
		std::memcpy(&networked_layers, g_LocalPlayer->GetAnimOverlays(), sizeof(AnimationLayer) * 13);

		auto backup_abs_angles = g_LocalPlayer->GetAbsAngles();
		auto backup_poses = g_LocalPlayer->m_flPoseParameter();
		if (g_LocalPlayer->m_fFlags() & FL_ONGROUND)
			g_LocalPlayer->m_fFlags() |= FL_ONGROUND;
		else
		{
			if (g_LocalPlayer->GetAnimOverlays()[4].m_flWeight != 1.f && g_LocalPlayer->GetAnimOverlays()[5].m_flWeight != 0.f)
				g_LocalPlayer->m_fFlags() |= FL_ONGROUND;

			if (g_LocalPlayer->m_fFlags() & FL_ONGROUND)
				g_LocalPlayer->m_fFlags() &= ~FL_ONGROUND;
		}

		update_Fake_state(m_fake_state, cmd->viewangles);
		m_got_fake_matrix = g_LocalPlayer->SetupBones(FakeAngleMatrix, 128, 524032 - 66666/*g_Menu.Config.nightmodeval*/, false);
		const auto org_tmp = g_LocalPlayer->GetRenderOrigin();
		if (m_got_fake_matrix)
		{
			for (auto& i : FakeAngleMatrix)
			{
				i[0][3] -= org_tmp.x;
				i[1][3] -= org_tmp.y;
				i[2][3] -= org_tmp.z;
			}
		}
		std::memcpy(g_LocalPlayer->GetAnimOverlays(), &networked_layers, sizeof(AnimationLayer) * 13);

		g_LocalPlayer->m_flPoseParameter() = backup_poses;
		g_LocalPlayer->GetAbsAngles() = backup_abs_angles;
	}
}
bool fresh_tick()
{
	static int old_tick_count;

	if (old_tick_count != g_GlobalVars->tickcount)
	{
		old_tick_count = g_GlobalVars->tickcount;
		return true;
	}

	return false;
}
void update_state(CBasePlayerAnimState* state, QAngle ang) {
	using fn = void(__vectorcall*)(void*, void*, float, float, float, void*);
	static auto ret = reinterpret_cast<fn>(Utils::PatternScan(GetModuleHandleW(L"client.dll"), "55 8B EC 83 E4 F8 83 EC 18 56 57 8B F9 F3 0F 11 54 24"));

	if (!ret)
		return;

	ret(state, NULL, NULL, ang.yaw, ang.pitch, NULL);
}
void RageAimbot::LocalAnimationFix(C_BasePlayer* entity)
{

	if (!entity || !entity->IsAlive() || !cmd)
		return;

	static float proper_abs = entity->GetPlayerAnimState()->m_flGoalFeetYaw;
	static std::array<float, 24> sent_pose_params = entity->m_flPoseParameter();
	static AnimationLayer backup_layers[15];

	if (fresh_tick())
	{
		std::memcpy(backup_layers, entity->GetAnimOverlays(), (sizeof(AnimationLayer) * entity->GetNumAnimOverlays()));
		entity->client_animation() = true;
		entity->UpdateAnimationState(entity->GetPlayerAnimState(),cmd->viewangles);

		if (entity->GetPlayerAnimState())
			entity->GetPlayerAnimState()->m_iLastClientSideAnimationUpdateFramecount = g_GlobalVars->framecount - 1;

		entity->UpdateClientSideAnimation();
		if (SendPacket)
		{
			proper_abs = entity->GetPlayerAnimState()->m_flGoalFeetYaw;
			sent_pose_params = entity->m_flPoseParameter();
		}
	}
	entity->client_animation() = false;
	g_LocalPlayer->SetAbsAngles(QAngle(0, proper_abs, 0));
	entity->GetPlayerAnimState()->m_flUnknownFraction = 0.f; // Lol.
	std::memcpy(entity->GetAnimOverlays(), backup_layers, (sizeof(AnimationLayer) * entity->GetNumAnimOverlays()));
	entity->m_flPoseParameter() = sent_pose_params;
}

void RageAimbot::Resolver(C_BasePlayer* Player, CBasePlayerAnimState* Animstate)
{
	if (!Player || !Player->IsAlive() || !Player->IsEnemy() || !Animstate)
		return;

	int MissedShots = ShotsFired[Player->EntIndex()] - ShotsHit[Player->EntIndex()];
	static float OldYaw = Player->m_angEyeAngles().yaw;
	float Back = Math::NormalizeYaw(Math::CalcAngle(g_LocalPlayer->m_vecOrigin(), Player->m_vecOrigin()).yaw + 180.f);
	float EyeDelta = fabs(Math::NormalizeYaw(Player->m_angEyeAngles().yaw - OldYaw));
	float AntiSide = 0.f;
	float Brute = 0.f;

	ForceSafePoint[Player->EntIndex()] = false;

	if (UseFreestand[Player->EntIndex()] && MissedShots <= 1 && EyeDelta < 45.f)
	{
		Brute = Math::NormalizeYaw(Back + LastFreestandAngle[Player->EntIndex()]);
	}
	else if (EyeDelta >= 45.f)
	{
		ForceSafePoint[Player->EntIndex()] = true;
	}
	else
	{
		switch ((MissedShots - 2) % 2)
		{
		case 0:
			if (Math::NormalizeYaw(Player->m_angEyeAngles().yaw - Back) > 0.f)
			{
				AntiSide = 90.f;
			}
			else if (Math::NormalizeYaw(Player->m_angEyeAngles().yaw - Back) < 0.f)
			{
				AntiSide = -90.f;
			}
			break;

		case 1:
			if (Math::NormalizeYaw(Player->m_angEyeAngles().yaw - Back) > 0.f)
			{
				AntiSide = -90.f;
			}
			else if (Math::NormalizeYaw(Player->m_angEyeAngles().yaw - Back) < 0.f)
			{
				AntiSide = 90.f;
			}

			break;
		}
		Brute = Math::NormalizeYaw(Animstate->m_flGoalFeetYaw + AntiSide);
	}
	OldYaw = Player->m_angEyeAngles().yaw;
	Animstate->m_flGoalFeetYaw = Brute;
}

void RageAimbot::AnimationFix()
{
	for (int i = 1; i <= 64; i++)
	{
		C_BasePlayer* Player = C_BasePlayer::GetPlayerByIndex(i);
		if (!Player ||
			Player->IsDormant() ||
			!Player->IsPlayer() ||
			!Player->IsAlive() ||
			Player == g_LocalPlayer)
			continue;
		if (g_Options.rage_enable)
		{
			CBasePlayerAnimState* state = Player->GetPlayerAnimState();
			if (state)
			{
				// backup
				const float curtime = g_GlobalVars->curtime;
				const float frametime = g_GlobalVars->frametime;

				static auto host_timescale = g_CVar->FindVar(("host_timescale"));

				g_GlobalVars->frametime = g_GlobalVars->interval_per_tick * host_timescale->GetFloat();
				g_GlobalVars->curtime = Player->m_flSimulationTime();

				AnimationLayer backup_layers[15];
				std::memcpy(backup_layers, Player->GetAnimOverlays(), (sizeof(AnimationLayer) * Player->GetNumAnimOverlays()));
				static std::array<float, 24> backup_params = g_LocalPlayer->m_flPoseParameter();

				int backup_eflags = Player->m_iEFlags();

				// SetLocalVelocity
				Player->m_iEFlags() &= ~0x1000; // InvalidatePhysicsRecursive(VELOCITY_CHANGED); EFL_DIRTY_ABSVELOCITY = 0x1000
				Player->m_vecAbsVelocity() = Player->m_vecVelocity();

				Player->client_animation() = true;

				player_info_t info;
				g_EngineClient->GetPlayerInfo(Player->EntIndex(), &info);
				bool Legit = (TIME_TO_TICKS(Player->m_flSimulationTime() - Player->m_flOldSimulationTime()) <= 1) || (info.fakeplayer);
				if (g_Options.RageAimbotResolver && !Legit)
					Resolver(Player, state);

				state->m_iLastClientSideAnimationUpdateFramecount = 0;

				Player->UpdateClientSideAnimation();

				// restore
				std::memcpy(Player->GetAnimOverlays(), backup_layers, (sizeof(AnimationLayer) * Player->GetNumAnimOverlays()));
				g_LocalPlayer->m_flPoseParameter() = backup_params;

				g_GlobalVars->curtime = curtime;
				g_GlobalVars->frametime = frametime;

				Player->m_iEFlags() = backup_eflags;

				Player->client_animation() = false;

				Player->SetupBones2(nullptr, -1, 0x7FF00, g_GlobalVars->curtime);
			}
		}
		else
			Player->UpdateClientSideAnimation();
	}
}//

void StopMovement(CUserCmd* cmd)
{
	constexpr bool isHexagoneGodlike = true;

	if (g_LocalPlayer->m_nMoveType() != MOVETYPE_WALK)
		return; // Not implemented otherwise :(

	Vector hvel = g_LocalPlayer->m_vecVelocity();
	hvel.z = 0;
	float speed = hvel.Length2D();

	if (speed < 1.f) // Will be clipped to zero anyways
	{
		cmd->forwardmove = 0.f;
		cmd->sidemove = 0.f;
		return;
	}

	// Homework: Get these dynamically
	float accel = 5.5f;
	float maxSpeed = 320.f;
	float playerSurfaceFriction = 1.0f; // I'm a slimy boi
	float max_accelspeed = accel * g_GlobalVars->interval_per_tick * maxSpeed * playerSurfaceFriction;

	float wishspeed{};

	// Only do custom deceleration if it won't end at zero when applying max_accel
	// Gamemovement truncates speed < 1 to 0
	if (speed - max_accelspeed <= -1.f)
	{
		// We try to solve for speed being zero after acceleration:
		// speed - accelspeed = 0
		// speed - accel*frametime*wishspeed = 0
		// accel*frametime*wishspeed = speed
		// wishspeed = speed / (accel*frametime)
		// ^ Theoretically, that's the right equation, but it doesn't work as nice as 
		//   doing the reciprocal of that times max_accelspeed, so I'm doing that :shrug:
		wishspeed = max_accelspeed / (speed / (accel * g_GlobalVars->interval_per_tick));
	}
	else // Full deceleration, since it won't overshoot
	{
		// Or use max_accelspeed, doesn't matter
		wishspeed = max_accelspeed;
	}

	// Calculate the negative movement of our velocity, relative to our viewangles
	Vector vndir = (hvel * -1.f);
	QAngle ndir;
	Math::VectorAngles(vndir, ndir);
	ndir.yaw = cmd->viewangles.yaw - ndir.yaw; // Relative to local view
	Vector vndir2;
	Math::AngleVectors(ndir, vndir2);

	cmd->forwardmove = vndir2.x * wishspeed;
	cmd->sidemove = vndir2.y * wishspeed;
}
void DoSlowWalk(CUserCmd* cmd, C_BaseCombatWeapon* Weapon)
{
	float amount = 0.0034f * g_Options.rage_sloswalk; // options.misc.slow_walk_amount has 100 max value

	Vector velocity = g_LocalPlayer->m_vecVelocity();
	QAngle direction;

	Math::VectorAngles(velocity, direction);

	float speed = velocity.Length2D();

	direction.yaw = cmd->viewangles.yaw - direction.yaw;

	Vector forward;

	Math::AngleVectors(direction, forward);

	Vector source = forward * -speed;

	if (speed >= (*(float*)((uintptr_t)Weapon->GetCSWeaponData() + 0x0130/*maxspeed*/) * amount))
	{
		cmd->forwardmove = source.x;
		cmd->sidemove = source.y;
	}
}

void RestorePlayer(C_BasePlayer* Player, TickInfo Record)
{
	Player->InvalidateBoneCache();

	Player->GetCollideable()->OBBMins() == Record.Mins;
	Player->GetCollideable()->OBBMaxs() == Record.Maxs;

	Player->m_angEyeAngles() = Record.Angles;
	Player->m_vecOrigin() = Record.Origin;
	Player->SetAbsOrigin(Record.Origin);

	Player->m_fFlags() = Record.Flags;

	int layerCount = Player->GetNumAnimOverlays();
	for (int i = 0; i < layerCount; ++i)
	{
		AnimationLayer* currentLayer = Player->GetAnimOverlay(i);
		currentLayer->m_nOrder = Record.LayerRecords[i].m_nOrder;
		currentLayer->m_nSequence = Record.LayerRecords[i].m_nSequence;
		currentLayer->m_flWeight = Record.LayerRecords[i].m_flWeight;
		currentLayer->m_flCycle = Record.LayerRecords[i].m_flCycle;
	}

	Player->m_flPoseParameter() = Record.PoseParams;
}
bool RageAimbot::ShouldBaim(C_BasePlayer* Player)
{
	return false;
}
void RageAimbot::GetMultipointPositions(C_BasePlayer* Player, std::vector<Vector>& Positions, int HitboxIndex, float Scale, matrix3x4_t* BoneMatrix)
{
	auto StudioModel = g_MdlInfo->GetStudiomodel(Player->GetModel());
	if (StudioModel) {
		auto Hitbox = StudioModel->GetHitboxSet(0)->GetHitbox(HitboxIndex);
		if (Hitbox) {

			const float HitboxRadius = Hitbox->m_flRadius * Scale;

			if (Hitbox->m_flRadius == -1.f)
			{
				const auto Center = (Hitbox->bbmin + Hitbox->bbmax) * 0.5f;

				Positions.emplace_back();
			}
			else
			{
				Vector P[12];
				for (int j = 0; j < 6; j++) { P[j] = Hitbox->bbmin; }
				for (int j = 7; j < 12; j++) { P[j] = Hitbox->bbmax; }

				P[1].x += HitboxRadius;
				P[2].x -= HitboxRadius;
				P[3].y += HitboxRadius;
				P[4].y -= HitboxRadius;
				P[5].z += HitboxRadius;

				P[6].x += HitboxRadius;
				P[7].x -= HitboxRadius;
				P[8].y += HitboxRadius;
				P[9].y -= HitboxRadius;
				P[10].z += HitboxRadius;
				P[11].z -= HitboxRadius;

				for (int j = 0; j < 12; j++)
				{
					Math::VectorTransform(P[j], BoneMatrix[Hitbox->bone], P[j]);
					Positions.push_back(P[j]);
				}
			}
		}
	}
}
bool RageAimbot::Hitscan(C_BasePlayer* Player, Vector& HitboxPos, matrix3x4_t* BoneMatrix, bool Backtrack,TickInfo Record)
{
	if (!g_Options.RageAimbotHead && !g_Options.RageAimbotBody && !g_Options.RageAimbotLegs && !g_Options.RageAimbotToes)
		return false;
	int MissedShots = ShotsFired[Player->EntIndex()] -ShotsHit[Player->EntIndex()];
	if (Backtrack)
	{
		std::vector<int> HitboxesToScan;
		if (g_Options.RageAimbotHead)
		{
			HitboxesToScan.push_back(HITBOX_HEAD);
			HitboxesToScan.push_back(HITBOX_NECK);
		}
		if (g_Options.RageAimbotBody)
		{
			for (int i = HITBOX_PELVIS; i <= HITBOX_UPPER_CHEST; i++)
				HitboxesToScan.push_back(i);
		}
		if (g_Options.RageAimbotLegs)
		{
			for (int i = HITBOX_RIGHT_THIGH; i <= HITBOX_LEFT_CALF; i++)
				HitboxesToScan.push_back(i);
		}
		if (g_Options.RageAimbotToes)
		{
			HitboxesToScan.push_back(HITBOX_RIGHT_FOOT);
			HitboxesToScan.push_back(HITBOX_LEFT_FOOT);
		}
		if (g_Options.RageAimbotSafePoint)
		{
			float AngToLocal = Math::CalcAngle(g_LocalPlayer->m_vecOrigin(), Record.Origin).yaw + 180.f;
			bool Backward = ((AngToLocal > 135 || AngToLocal < -135) || (AngToLocal < 45 || AngToLocal > -45));
			bool Freestanding = !Backward;
			player_info_t info;
			g_EngineClient->GetPlayerInfo(Player->EntIndex(), &info);
			bool Legit = (TIME_TO_TICKS(Player->m_flSimulationTime() - Player->m_flOldSimulationTime()) <= 1) || (info.fakeplayer);
			if (!Freestanding && !Legit && Player->MaxDesyncDelta() >= 35.f)
			{
				HitboxesToScan.erase(HitboxesToScan.begin(), HitboxesToScan.begin() + HitboxesToScan.size());

				for (int i = HITBOX_PELVIS; i <= HITBOX_LOWER_CHEST; i++)
				{
					HitboxesToScan.push_back(i);
				}
			}
		}
		if ((g_Options.RageAimbotBaimAfter && MissedShots >= g_Options.RageAimbotBaimAfter) || ForceSafePoint[Player->EntIndex()])
		{
			HitboxesToScan.erase(HitboxesToScan.begin(), HitboxesToScan.begin() + HitboxesToScan.size());
			for (int i = HITBOX_PELVIS; i <= HITBOX_UPPER_CHEST; i++)
			{
				HitboxesToScan.push_back(i);
			}
		}
		if (HitboxesToScan.size())
		{
			for (auto HitBoxID : HitboxesToScan)
			{
				Vector Point = Player->GetHitboxPos(HitBoxID, BoneMatrix);
				if (g_LocalPlayer->CanSeePlayer(Player, Point))
				{
					HitboxPos = Point;
					return true;
				}
			}
		}
	}
	else
	{
		std::vector<Vector> ScanPositions;
		std::vector<Vector> HitboxPositions;
		std::vector<Vector> MultipointPositions;
		int MinimumDamage = std::min<int>(Player->m_iHealth() + 10, g_Options.RageAimbotMinDmg);
		int BestDamage = 0;
		Vector BestPosition = Vector{};

		if (g_Options.RageAimbotHead)
		{
			HitboxPositions.push_back(Player->GetHitboxPos(HITBOX_HEAD, BoneMatrix));
			HitboxPositions.push_back(Player->GetHitboxPos(HITBOX_NECK, BoneMatrix));
			if (g_Options.RageAimbotHeadScale)
				GetMultipointPositions(Player, MultipointPositions, HITBOX_HEAD, g_Options.RageAimbotHeadScale, BoneMatrix);
		}
		float Velocity = abs(Player->m_vecVelocity().Length2D());

		if (!g_Options.RageAimbotDelayShot && Velocity > 0.f || !(Player->m_fFlags() & FL_ONGROUND))
			Velocity = 0.f;

		if (Velocity <= 200.f || ShouldBaim(Player))
		{
			if (g_Options.RageAimbotBody)
			{
				for (int i = HITBOX_PELVIS; i <= HITBOX_UPPER_CHEST; i++)
				{
					HitboxPositions.push_back(Player->GetHitboxPos(i, BoneMatrix));
					if (g_Options.RageAimbotBodyScale)
						GetMultipointPositions(Player, MultipointPositions, i, g_Options.RageAimbotBodyScale, BoneMatrix);
				}
			}
			if (g_Options.RageAimbotLegs)
			{
				for (int i = HITBOX_RIGHT_THIGH; i <= HITBOX_LEFT_CALF; i++)
					HitboxPositions.push_back(Player->GetHitboxPos(i, BoneMatrix));
			}
			if (g_Options.RageAimbotToes)
			{
				HitboxPositions.push_back(Player->GetHitboxPos(HITBOX_RIGHT_FOOT, BoneMatrix));
				HitboxPositions.push_back(Player->GetHitboxPos(HITBOX_LEFT_FOOT, BoneMatrix));
			}
		}
		if (g_Options.RageAimbotSafePoint)
		{
			float AngToLocal = Math::CalcAngle(g_LocalPlayer->m_vecOrigin(), Player->m_vecOrigin()).yaw + 180.f;
			bool Backward = ((AngToLocal > 135 || AngToLocal < -135) || (AngToLocal < 45 || AngToLocal > -45));
			bool Freestanding = !Backward;
			player_info_t info;
			g_EngineClient->GetPlayerInfo(Player->EntIndex(), &info);
			bool Legit = (TIME_TO_TICKS(Player->m_flSimulationTime() - Player->m_flOldSimulationTime()) <= 1) || (info.fakeplayer);
			if (!Freestanding && !Legit && Player->MaxDesyncDelta() >= 35.f)
			{
				HitboxPositions.erase(HitboxPositions.begin(), HitboxPositions.begin() + HitboxPositions.size());
				MultipointPositions.erase(MultipointPositions.begin(), MultipointPositions.begin() + MultipointPositions.size());

				for (int i = HITBOX_PELVIS; i <= HITBOX_LOWER_CHEST; i++)
				{
					HitboxPositions.push_back(Player->GetHitboxPos(i, BoneMatrix));
					if (g_Options.RageAimbotBodyScale)
						GetMultipointPositions(Player, MultipointPositions, i, g_Options.RageAimbotBodyScale, BoneMatrix);
				}
			}
		}
		if ((g_Options.RageAimbotBaimAfter && MissedShots >= g_Options.RageAimbotBaimAfter) || ForceSafePoint[Player->EntIndex()])
		{
			HitboxPositions.erase(HitboxPositions.begin(), HitboxPositions.begin() + HitboxPositions.size());
			MultipointPositions.erase(MultipointPositions.begin(), MultipointPositions.begin() + MultipointPositions.size());
			for (int i = HITBOX_PELVIS; i <= HITBOX_LOWER_CHEST; i++)
			{
				HitboxPositions.push_back(Player->GetHitboxPos(i, BoneMatrix));
				if (g_Options.RageAimbotBodyScale)
					GetMultipointPositions(Player, MultipointPositions, i, g_Options.RageAimbotBodyScale, BoneMatrix);
			}
		}
		for (auto Position : HitboxPositions)
			ScanPositions.push_back(Position);
		for (auto Position : MultipointPositions)
			ScanPositions.push_back(Position);

		for (auto Position : ScanPositions)
		{
			float Damage = Autowall::Get().CanHit(Position);
			if (Damage > BestDamage)
			{
				BestDamage = Damage;
				BestPosition = Position;
			}
		}
		if (BestDamage >= MinimumDamage)
		{
			HitboxPos = BestPosition;
			return true;
		}
	}


	return false;
}
void RageAimbot::StoreRecords()
{
	for (int i = 1; i <= 64; i++)
	{
		C_BasePlayer* Player = C_BasePlayer::GetPlayerByIndex(i);
		if (!Player ||
			Player->IsDormant() ||
			!Player->IsPlayer() ||
			!Player->IsEnemy() ||
			!Player->IsAlive())
		{
			BacktrackRecords[i].erase(BacktrackRecords[i].begin(), BacktrackRecords[i].begin() + BacktrackRecords[i].size());
			continue;
		}

		BacktrackRecords[i].insert(BacktrackRecords[i].begin(), TickInfo(Player));
	}
}
//void RageAimbot::WeaponSettings(C_BaseCombatWeapon* Weapon)
//{
//	/*if (Weapon->m_Item().m_iItemDefinitionIndex() == WEAPON_SCAR20 ||
//		Weapon->m_Item().m_iItemDefinitionIndex() == WEAPON_G3SG1)
//	{
//		HitchanceValue = g_Options.RageAimbotHitchance;
//		MinDmgValue = g_Options.RageAimbotMinDmg;
//	}
//	else if (Weapon->m_Item().m_iItemDefinitionIndex() == WEAPON_SSG08)
//	{
//		HitchanceValue = g_Options.RageAimbotHitchance;
//		MinDmgValue = g_Options.RageAimbotMinDmg;
//	}
//	else if (Weapon->m_Item().m_iItemDefinitionIndex() == WEAPON_AWP)
//	{
//		HitchanceValue = g_Options.RageAimbotHitchance;
//		MinDmgValue = g_Options.RageAimbotMinDmg;
//	}
//	else if (Weapon->IsPistol())
//	{
//		if (Weapon->m_Item().m_iItemDefinitionIndex() == WEAPON_DEAGLE ||
//			Weapon->m_Item().m_iItemDefinitionIndex() == WEAPON_REVOLVER)
//		{
//			HitchanceValue = g_Options.RageAimbotHitchance;
//			MinDmgValue = g_Options.RageAimbotMinDmg;
//		}
//		else
//		{
//			HitchanceValue = g_Options.RageAimbotHitchance;
//			MinDmgValue = g_Options.RageAimbotMinDmg;
//		}
//	}*/
//	else
//	{
//		HitchanceValue = g_Options.RageAimbotMinDmg;
//		MinDmgValue = g_Options.RageAimbotMinDmg;
//	}
//}
void RageAimbot::Do(CUserCmd* cmd, C_BaseCombatWeapon* Weapon, bool& bSendPacket)
{
	if (!g_Options.rage_enable)
		return;
	for (int i = 1; i <= 64; i++)
	{
		C_BasePlayer* Player = C_BasePlayer::GetPlayerByIndex(i);
		if (!Player ||
			!Player->IsPlayer() ||
			Player->IsDormant() ||
			!Player->IsAlive() ||
			!Player->IsEnemy() ||
			Player->m_bGunGameImmunity() ||
			BacktrackRecords[i].size() < 1)
			continue;

		for (auto Tick : BacktrackRecords[i])
			if (!Utils::IsTickValid(Tick.SimulationTime, 0.2f))
				BacktrackRecords[i].pop_back();
	}
	static bool Shot = false;
	if (!g_LocalPlayer ||
		!g_LocalPlayer->IsAlive() ||
		!Weapon || Weapon->IsKnife() ||	Weapon->IsGrenade())
	{
		Shot = false;
		return;
	}

	//WeaponSettings(Weapon);

	//if (InputSys::Get().IsKeyDown(VK_SHIFT))
	//	DoSlowWalk(cmd, Weapon);

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
			Player->m_bGunGameImmunity() ||
			BacktrackRecords[i].size() < 1)
			continue;

		auto CurrentRecord = BacktrackRecords[i].front();
		auto BacktrackRecord = BacktrackRecords[i].back();

		float PlayerDistance = Math::VectorDistance(g_LocalPlayer->m_vecOrigin(), Player->m_vecOrigin());

		if (BestTargetDistance > PlayerDistance)
		{
			if (CurrentRecord.MatrixBuilt && CurrentRecord.BoneMatrix != nullptr &&
				Hitscan(Player, Hitbox, CurrentRecord.BoneMatrix, false, CurrentRecord))
			{
				BestTargetDistance = PlayerDistance;
				BestTargetIndex = i;
				BestTargetSimtime = CurrentRecord.SimulationTime;
				Backtrack = false;
			}
			else if (BacktrackRecord.MatrixBuilt && BacktrackRecord.BoneMatrix != nullptr &&
				Hitscan(Player, Hitbox, BacktrackRecord.BoneMatrix, true, BacktrackRecord))
			{
				BestTargetDistance = PlayerDistance;
				BestTargetIndex = i;
				BestTargetSimtime = BacktrackRecord.SimulationTime;
				Backtrack = true;
			}
		}
	}
	if (Shot)
	{
		bSendPacket = true;
		DoAntiaim(cmd, Weapon, bSendPacket);
		Shot = false;
	}
	if (BestTargetIndex != -1 && Hitbox.IsValid() && BestTargetSimtime)
	{
		C_BasePlayer* Target = C_BasePlayer::GetPlayerByIndex(BestTargetIndex);
		if (!Target) return;

		QAngle AimAngle = Math::CalcAngle(g_LocalPlayer->GetEyePos(), Hitbox);
		AimAngle -= g_LocalPlayer->m_aimPunchAngle() * g_CVar->FindVar("weapon_recoil_scale")->GetFloat();

		//QAngle AimAngle = Math::CalcAngle(g_LocalPlayer->GetEyePos(), Hitbox) - g_LocalPlayer->m_aimPunchAngle() * g_CVar->FindVar("weapon_recoil_scale")->GetFloat();
		Math::Normalize3(AimAngle);
		Math::ClampAngles(AimAngle);
		//AimAngle -= g_LocalPlayer->m_aimPunchAngle() * g_CVar->FindVar("weapon_recoil_scale")->GetFloat();


		if (Weapon->IsSniper() && !g_LocalPlayer->m_bIsScoped())
			cmd->buttons |= IN_ATTACK2;

		if (Backtrack)
			RestorePlayer(Target, BacktrackRecords[BestTargetIndex].back());

		if (!Weapon->IsZeus() && (g_LocalPlayer->m_fFlags() & FL_ONGROUND))
		{
			if (Weapon->CanFire())
			{
				StopMovement(cmd);
			}
		}

		if (!Hitchance(Target, Weapon, AimAngle, Hitbox, g_Options.RageAimbotHitchance) && g_Options.RageAimbotHitchance)
		{


		}
		else
		{
			if (/*!(cmd->buttons & IN_ATTACK) && */Weapon->CanFire())
			{
	
				cmd->viewangles = AimAngle;
				if (!g_Options.rage_silent)
				{
					//cmd->viewangles = AimAngle;
					g_EngineClient->SetViewAngles(&cmd->viewangles);
				}
				cmd->tick_count = TIME_TO_TICKS(BestTargetSimtime + Utils::GetLerpTime());
				cmd->buttons |= IN_ATTACK;
				//ShotsFired[Target->EntIndex()] += 1;
			}
		}
		if (Backtrack)
			RestorePlayer(Target, BacktrackRecords[BestTargetIndex].front());
	}
}
bool LbyUpdate() {

	auto speed = g_LocalPlayer->m_vecVelocity().Length2D();
	static float next_lby = 0.00f;
	float curtime = g_GlobalVars->curtime;

	if (!(g_LocalPlayer->m_fFlags() & FL_ONGROUND))
		return false;

	if (speed > 0.1f)
		next_lby = curtime + 0.22;

	if (next_lby < curtime)
	{
		next_lby = curtime + 1.1;
		return true;
	}
	else
		return false;
}
void RageAimbot::DoFakelag(bool& bSendPacket)
{
	int ChokeLimit = g_Options.misc_fakelag;
	if (ChokeLimit < 1)
		ChokeLimit = 1;

	/*if (Variables.MiscFakeDuckKey && InputSys::Get().IsKeyDown(Variables.MiscFakeDuckKey))
		ChokeLimit = 14;*/

	if (g_EngineClient->IsVoiceRecording() || (g_LocalPlayer->m_vecVelocity().Length2D() <= 2.f /*&& !(Variables.MiscFakeDuckKey && InputSys::Get().IsKeyDown(Variables.MiscFakeDuckKey))*/))
		ChokeLimit = 1;

	bSendPacket = (g_EngineClient->GetNetChannel()->m_nChokedPackets >= ChokeLimit);
}
void RageAimbot::DoAntiaim(CUserCmd* cmd, C_BaseCombatWeapon* Weapon, bool& bSendPacket)
{
	if (!g_LocalPlayer ||
		!g_LocalPlayer->IsAlive() ||
		!Weapon ||
		g_LocalPlayer->m_hActiveWeapon()->IsKnife() && cmd->buttons & IN_ATTACK ||
		!g_LocalPlayer->m_hActiveWeapon()->IsGrenade() && cmd->buttons & IN_ATTACK && Weapon->CanFire() ||
		cmd->buttons & IN_USE ||
		g_LocalPlayer->m_hActiveWeapon()->IsGrenade() && Weapon->m_fThrowTime() > 0.f ||
		g_LocalPlayer->m_nMoveType() == MOVETYPE_LADDER ||
		g_LocalPlayer->m_nMoveType() == MOVETYPE_NOCLIP)
		return;

	cmd->viewangles.pitch = 89.f;
	//cmd->viewangles.yaw += 180.f;
	if (SendPacket)
	{
		cmd->viewangles.yaw += 180.f;
	}
	else
	{
		cmd->viewangles.yaw += 180.f - g_LocalPlayer->MaxDesyncDelta();
	}

	if (g_LocalPlayer->m_fFlags() & FL_ONGROUND && cmd->sidemove < 4 && cmd->sidemove > -4) {
		auto sideAmount = (cmd->buttons & IN_DUCK) ? 3.f : 1.1f;
		static bool switch_ = false;
		if (switch_)
			cmd->sidemove += sideAmount;
		else
			cmd->sidemove += -sideAmount;
		switch_ = !switch_;

		MovementFix::Get().m_oldforward = cmd->forwardmove;
		MovementFix::Get().m_oldsidemove = cmd->sidemove;
	}

	cmd->buttons &= ~(IN_FORWARD | IN_BACK | IN_MOVERIGHT | IN_MOVELEFT);
}