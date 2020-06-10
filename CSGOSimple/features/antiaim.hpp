#pragma once

#include "../options.hpp"
#include "../helpers/math.hpp"
#include "../singleton.hpp"
#include "../valve_sdk/csgostructs.hpp"


class antiaim : public Singleton<antiaim>
{
public:
    void createmove(CUserCmd* cmd, bool& bSendPacket);
};