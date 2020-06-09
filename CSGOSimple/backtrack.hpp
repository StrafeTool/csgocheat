#include "singleton.hpp"
#include "valve_sdk/csgostructs.hpp"
#include "helpers/math.hpp"
#include "options.hpp"
#include <deque>
#include "valve_sdk/math/Vector.hpp"
#include "valve_sdk/math/VMatrix.hpp"

#define NUM_OF_TICKS 12

struct StoredData
{
	float simtime;
	Vector hitboxPos;
};
class TimeWarp : public Singleton<TimeWarp>
{
	int nLatestTick;
	
public:
	StoredData TimeWarpData[64][NUM_OF_TICKS];
	void CreateMove(CUserCmd* cmd);
};

