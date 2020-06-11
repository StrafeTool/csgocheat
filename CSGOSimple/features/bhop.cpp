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
 // ragestrafer(cmd);


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



void BunnyHop::ragestrafer(CUserCmd* cmd)
{

    //static auto down = [](ButtonCode_t bt) -> bool {
    //    return g_csgo.m_inputsys()->IsButtonDown(bt);
    //};

    if (g_LocalPlayer->m_nMoveType() == MoveType_t::MOVETYPE_NOCLIP || g_LocalPlayer->m_nMoveType() == MoveType_t::MOVETYPE_LADDER)
        return;

    if (!GetAsyncKeyState(VK_SPACE) || g_LocalPlayer->m_vecVelocity().Length2D() < 0.5)
        return;

    if (!(g_LocalPlayer->m_fFlags() & FL_ONGROUND)) {
        static float cl_sidespeed = g_CVar->FindVar("cl_sidespeed")->GetFloat();
        if (fabsf(cmd->mousedx > 2)) {
          cmd->sidemove = (cmd->mousedx < 0.f) ? -cl_sidespeed : cl_sidespeed;
            return;
        }


        if (GetAsyncKeyState('S')) {
            cmd->viewangles.yaw -= 180;
        }
        else if (GetAsyncKeyState('D')) {
            cmd->viewangles.yaw -= 90;
        }
        else if (GetAsyncKeyState('A')) {
            cmd->viewangles.yaw += 90;
        }


        if (!g_LocalPlayer->m_vecVelocity().Length2D() > 0.5 || g_LocalPlayer->m_vecVelocity().Length2D() == NAN || g_LocalPlayer->m_vecVelocity().Length2D() == INFINITE)
        {
            cmd->forwardmove = 400;
            return;
        }


        cmd->forwardmove = std::clamp((5850.f / g_LocalPlayer->m_vecVelocity().Length2D()), -400.f, 400.f);
        if ((cmd->forwardmove < -400 ||cmd->forwardmove > 400))
            cmd->forwardmove = 0;

        const auto vel =g_LocalPlayer->m_vecVelocity();
        const float y_vel = RAD2DEG(atan2(vel.y, vel.x));
        const float diff_ang = (cmd->viewangles.yaw - y_vel); // needs to be normalized

      cmd->sidemove = (diff_ang > 0.0) ? -cl_sidespeed : cl_sidespeed;
      cmd->viewangles.yaw = (cmd->viewangles.yaw - diff_ang); //normalize here too
    }
}