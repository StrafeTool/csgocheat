#pragma once

#include "../valve_sdk/sdk.hpp"

class CTickbase
{

public:

    bool CanProcessPacket(CUserCmd* m_pCmd);
    bool CanShift(int m_nShiftedTicks = 0);

    void DoubleTap(CUserCmd* m_pCmd, bool* m_pbSendPacket);
    void ShiftTickbase(CUserCmd* m_pCmd, bool m_bSendPacket);
    void ResetData();

    int g_nTickbase = 0;
    int g_nTicks = 0;

    struct
    {
        int m_nCmd = 0;
        int m_nShift = 0;
        int m_nTickbase = 0;
    } m_ShotData;

private:

    int m_iMaxProcessTicks = 0;
    int m_nShift = 0;
    int m_nChokeLimit = 0;

    bool m_bProcessPacket = false;
    bool m_bNextShift = false;
    bool m_bSwap = false;
    bool m_bEnabled = false;

};

extern CTickbase g_ViolanesPaster;