#include "singleton.hpp"
#include "valve_sdk/csgostructs.hpp"
#include "helpers/math.hpp"
#include "Vector3D.hpp"
#include "options.hpp"
#include <deque>
#include "valve_sdk/math/Vector.hpp"
#include "valve_sdk/math/VMatrix.hpp"

struct stored_records {
	Vector head;
	float simulation_time;
	matrix3x4_t matrix[128];
};

struct convars {	
	ConVar* update_rate;
	ConVar* max_update_rate;
	ConVar* interp;
	ConVar* interp_ratio;
	ConVar* min_interp_ratio;
	ConVar* max_interp_ratio;
	ConVar* max_unlag;
};

extern std::deque<stored_records> records[65];
extern convars cvars;

class c_backtrack {
public:
	void update() noexcept;
	void run(CUserCmd*) noexcept;
	float get_lerp_time() noexcept;
	int time_to_ticks(float time) noexcept;
	bool valid_tick(float simtime) noexcept;
	static void init() {
		records->clear();
		
		cvars.update_rate = g_CVar->FindVar("cl_updaterate");
		cvars.max_update_rate = g_CVar->FindVar("sv_maxupdaterate");
		cvars.interp = g_CVar->FindVar("cl_interp");
		cvars.interp_ratio = g_CVar->FindVar("cl_interp_ratio");
		cvars.min_interp_ratio = g_CVar->FindVar("sv_client_min_interp_ratio");
		cvars.max_interp_ratio = g_CVar->FindVar("sv_client_max_interp_ratio");
		cvars.max_unlag = g_CVar->FindVar("sv_maxunlag");
	}
};

extern c_backtrack backtrack;