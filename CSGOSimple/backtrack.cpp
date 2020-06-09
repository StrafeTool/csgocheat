#include "backtrack.hpp"

c_backtrack backtrack;

std::deque<stored_records> records[65];
convars cvars;

float c_backtrack::get_lerp_time() noexcept {
	auto ratio = std::clamp(cvars.interp_ratio->GetFloat(), cvars.min_interp_ratio->GetFloat(), cvars.max_interp_ratio->GetFloat());
	return std::max(cvars.interp->GetFloat(), (ratio / ((cvars.max_update_rate) ? cvars.max_update_rate->GetFloat() : cvars.update_rate->GetFloat())));
}

int c_backtrack::time_to_ticks(float time) noexcept {
	return static_cast<int>((0.5f + static_cast<float>(time) / g_GlobalVars->interval_per_tick));
}

bool c_backtrack::valid_tick(float simtime) noexcept {
	auto network = g_EngineClient->GetNetChannelInfo();
	if (!network)
		return false;

	auto delta = std::clamp(network->GetLatency(0) + get_lerp_time(), 0.f, cvars.max_unlag->GetFloat()) - (g_GlobalVars->curtime - simtime);
	return std::fabsf(delta) <= 0.2f;
}

void c_backtrack::update() noexcept {
	if (!g_Options.misc_backtrack || !g_LocalPlayer || !g_LocalPlayer->IsAlive()) {
		
		if (!records->empty())
			records->clear();

		return;
	}

	for (int i = 1; i <= g_GlobalVars->maxClients; i++) {
		auto entity = reinterpret_cast<C_BasePlayer*>(g_EntityList->GetClientEntity(i));
		if (!entity || entity == g_LocalPlayer || entity->IsDormant() || !entity->IsAlive()) {
			records[i].clear();
			continue;
		}

		if (records[i].size() && (records[i].front().simulation_time == entity->m_flSimulationTime()))
			continue;

		auto var_map = reinterpret_cast<uintptr_t>(entity) + 0x24;
		auto vars_count = *reinterpret_cast<int*>(static_cast<uintptr_t>(var_map) + 0x14);
		for (int j = 0; j < vars_count; j++)
			*reinterpret_cast<uintptr_t*>(*reinterpret_cast<uintptr_t*>(var_map) + j * 0xC) = 0;

		stored_records record{ };
		record.head = g_LocalPlayer->GetHitboxPos(HITBOX_HEAD);
		record.simulation_time = entity->m_flSimulationTime();

		entity->SetupBones(record.matrix, 128, 0x7FF00, g_GlobalVars->curtime);

		records[i].push_front(record);

		while (records[i].size() > 3 && records[i].size() > static_cast<size_t>(time_to_ticks(static_cast<float>(200.0f) / 1000.f)))
			records[i].pop_back();

		if (auto invalid = std::find_if(std::cbegin(records[i]), std::cend(records[i]), [](const stored_records& rec) { return !backtrack.valid_tick(rec.simulation_time); }); invalid != std::cend(records[i]))
			records[i].erase(invalid, std::cend(records[i]));
	}
}

void c_backtrack::run(CUserCmd* cmd) noexcept {
	if (!g_Options.misc_backtrack)
		return;

	if (!(cmd->buttons & IN_ATTACK))
		return;

	if (!g_LocalPlayer)
		return;

	auto best_fov{ 255.f };
	C_BasePlayer* best_target{ };
	int besst_target_index{ };
	Vector best_target_head_position{ };
	int best_record{ };

	for (int i = 1; i <= g_GlobalVars->maxClients; i++) {
		auto entity = reinterpret_cast<C_BasePlayer*>(g_EntityList->GetClientEntity(i));
		if (!entity || entity == g_LocalPlayer || entity->IsDormant() || !entity->IsAlive())
			continue;

		auto head_position = entity->GetBonePos(8);

		auto angle = Math::CalcAngle(g_LocalPlayer->GetEyePos(), head_position);
		auto fov = std::hypotf(angle.pitch, angle.yaw);
		if (fov < best_fov) {
			best_fov = fov;
			best_target = entity;
			besst_target_index = i;
			best_target_head_position = head_position;
		}
	}

	if (best_target) {
		if (records[besst_target_index].size() <= 3)
			return;

		best_fov = 255.f;

		for (size_t i = 0; i < records[besst_target_index].size(); i++) {
			auto record = &records[besst_target_index][i];
			if (!record || !valid_tick(record->simulation_time))
				continue;

			auto angle = Math::CalcAngle(g_LocalPlayer->GetEyePos(), record->head);
			auto fov = std::hypotf(angle.pitch, angle.yaw);
			if (fov < best_fov) {
				best_fov = fov;
				best_record = i;
			}
		}
	}

	if (best_record) {
		auto record = records[besst_target_index][best_record];
		cmd->tick_count = time_to_ticks(record.simulation_time);
	}
}