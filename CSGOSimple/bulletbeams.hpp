#pragma once
#include "helpers/math.hpp"


struct BulletImpactInfo
{
	float m_flExpTime;
	Vector m_vecHitPos;
};

class BulletBeamsEvent : public IGameEventListener2, public Singleton<BulletBeamsEvent>
{
public:

	void FireGameEvent(IGameEvent* event);
	int  GetEventDebugID(void);

	void RegisterSelf();
	void UnregisterSelf();

	void Paint(void);

private:

	std::vector<BulletImpactInfo> bulletImpactInfo;

};