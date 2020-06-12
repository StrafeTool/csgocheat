#include "tickbase.h"

#include "../valve_sdk/csgostructs.hpp"
#include "../options.hpp"

CTickbase g_ViolanesPaster;

bool CTickbase::CanProcessPacket(CUserCmd* m_pCmd)
{
    m_nChokeLimit = 14;
    m_nShift = 0;
    m_iMaxProcessTicks++;
    m_bEnabled = g_Options.hideshots || g_Options.doubletap;

    C_BaseCombatWeapon* m_pWeapon = g_LocalPlayer->m_hActiveWeapon().Get();
    if (!m_bEnabled || !m_pWeapon || !m_pWeapon->IsGun() || m_pWeapon->m_Item().m_iItemDefinitionIndex() == 64)
    {
        if (!g_Options.doubletap)
            m_bSwap = false;

        return true;
    }

    if (m_bSwap)
    {
        if (m_iMaxProcessTicks >= 12 || g_ClientState->m_nChokedCmds >= 5)
        {
            m_nChokeLimit = 1;
            m_nShift = 9;

            if (g_Options.doubletap)
            {
                if (CanShift(12) || (!CanShift() && m_bNextShift))
                    m_nShift = 12;
                else if (CanShift())
                    m_nShift = 0;
            }
            else
                m_bNextShift = false;

            return true;
        }
        else
        {
            m_nShift = 0;
            m_bEnabled = false;

            return false;
        }
    }
    else
    {
        m_iMaxProcessTicks = 0;
        m_nShift = 0;
        m_bEnabled = false;
        m_pCmd->tick_count = INT_MAX;
        m_bSwap = true;

        return true;
    }
}

bool CTickbase::CanShift(int m_nShiftedTicks)
{
    if (m_bProcessPacket)
        return false;

    C_BaseCombatWeapon* m_pWeapon = g_LocalPlayer->m_hActiveWeapon().Get();
    if (!m_pWeapon)
        return false;

    if (g_LocalPlayer->m_fFlags() & FL_ATCONTROLS)
        return false;

    if (!m_pWeapon->m_iClip1())
        return false;

    if (!m_pWeapon->IsGun() || m_pWeapon->m_Item().m_iItemDefinitionIndex() == 64)
        return false;

    int m_nTickbase = 0;
    if (m_bEnabled && m_nShift > 0)
        m_nTickbase = g_LocalPlayer->m_nTickBase() - m_nShift + 1;

    if (m_nShiftedTicks > 0)
        m_nTickbase = g_LocalPlayer->m_nTickBase() - m_nShiftedTicks;

    float m_flPlayerTime = m_nTickbase * g_GlobalVars->interval_per_tick;
    if (m_flPlayerTime < std::min(g_LocalPlayer->m_flNextAttack(), m_pWeapon->m_flNextPrimaryAttack()))
        return false;

    return true;
}

void CTickbase::DoubleTap(CUserCmd* m_pCmd, bool* m_pbSendPacket)
{
    C_BaseCombatWeapon* m_pWeapon = g_LocalPlayer->m_hActiveWeapon().Get();
    if (!m_pWeapon)
        return;

    if (!(m_pCmd->buttons & IN_ATTACK) || !CanShift() || m_nShift <= 0 || !m_bEnabled)
        return;

    if (g_ClientState->m_nChokedCmds < std::min(m_nChokeLimit, 14))
        *m_pbSendPacket = false;

    m_ShotData.m_nCmd = m_pCmd->command_number;
    m_ShotData.m_nShift = m_nShift;
    m_ShotData.m_nTickbase = g_LocalPlayer->m_nTickBase();
}

void CTickbase::ShiftTickbase(CUserCmd* m_pCmd, bool m_bSendPacket)
{
    if (!m_bSendPacket)
    {
        m_bProcessPacket = (m_pCmd->buttons & IN_ATTACK) && CanShift();
        return;
    }

    g_nTickbase = 0;
    if (!m_bEnabled)
        return;

    if (m_bProcessPacket || ((m_pCmd->buttons & IN_ATTACK) && CanShift()))
    {
        g_nTickbase = -m_nShift;

        if (m_nShift == 12)
            m_bNextShift = false;
        else if (!m_nShift)
            m_bNextShift = true;
    }

    m_bProcessPacket = false;
    m_bSwap = false;
}

void CTickbase::ResetData()
{
    g_nTickbase = 0;
    m_iMaxProcessTicks = 0;
    m_bSwap = false;
}