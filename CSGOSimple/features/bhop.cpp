#include "bhop.hpp"
#include "../options.hpp"
#include "../valve_sdk/csgostructs.hpp"

void BunnyHop::OnCreateMove(CUserCmd* cmd)
{
  static bool jumped_last_tick = false;
  static bool should_fake_jump = false;

  if (!g_LocalPlayer)
	  return;

  if (!g_LocalPlayer->IsAlive())
	  return;

  if (g_LocalPlayer->m_nMoveType() == MOVETYPE_LADDER || g_LocalPlayer->m_nMoveType() == MOVETYPE_NOCLIP)
	  return;

  if (g_LocalPlayer->m_fFlags() & FL_INWATER)
	  return;

  if(!jumped_last_tick && should_fake_jump) {
    should_fake_jump = false;
    cmd->buttons |= IN_JUMP;
  } else if(cmd->buttons & IN_JUMP) {
    if(g_LocalPlayer->m_fFlags() & FL_ONGROUND) {
      jumped_last_tick = true;
      should_fake_jump = true;
    } else {
      cmd->buttons &= ~IN_JUMP;
      jumped_last_tick = false;
    }
  } else {
    jumped_last_tick = false;
    should_fake_jump = false;
  }
}


void BunnyHop::AutoStrafe(CUserCmd* cmd)
{

    if (!g_LocalPlayer
        || !g_LocalPlayer->IsAlive()
        || g_LocalPlayer->m_nMoveType() == MOVETYPE_LADDER
        || g_LocalPlayer->m_nMoveType() == MOVETYPE_NOCLIP
        || g_LocalPlayer->m_fFlags() & FL_INWATER
        || g_LocalPlayer->m_fFlags() & FL_ONGROUND)
        return;

    if (g_Options.misc_autostrafe)
    {
        if (cmd->mousedx > 1 || cmd->mousedx < -1)
        {
            if (cmd->buttons & IN_MOVELEFT || cmd->buttons & IN_MOVERIGHT || cmd->buttons & IN_BACK || cmd->buttons & IN_FORWARD)
                return;

            cmd->sidemove = cmd->mousedx < 0.f ? -450.f : 450.f;
        }
    }
   
}