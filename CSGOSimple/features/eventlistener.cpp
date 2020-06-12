//#include "eventlistener.hpp"
//
//#include "ragebot.h"
//
//
//struct shotdata
//{
//    int shots = 0;
//};
//
//void::c_event_logger::on_fire_event(IGameEvent* event)
//{
//    if (!strcmp(event->GetName(),("bullet_impact")))
//    {
//        const auto pos = Vector(event->GetFloat(("x")), event->GetFloat(("y")), event->GetFloat(("z")));
//        const auto userid = g_EngineClient->GetPlayerForUserID(event->GetInt(("userid")));
//        if (userid == g_EngineClient->GetLocalPlayer() && g_LocalPlayer)
//        {
//            static auto last_shot_from = g_LocalPlayer->GetEyePos();
//            static auto last_curtime = g_GlobalVars->curtime;
//        }
//    }
//
//}