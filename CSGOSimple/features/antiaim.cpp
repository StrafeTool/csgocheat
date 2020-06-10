#include "antiaim.hpp"

void fakelag(CUserCmd* cmd, bool& bSendPacket)
{
    if (g_EngineClient->IsVoiceRecording())
        return;
    if (!g_LocalPlayer)
        return;

    int chockepack = 0;
    auto NetChannel = g_EngineClient->GetNetChannel();
    if (!NetChannel)
        return;
    bSendPacket = true;

    chockepack = g_Options.misc_fakelagammount;
    bSendPacket = (NetChannel->m_nChokedPackets >= chockepack);


}


void antiaim::createmove(CUserCmd* cmd, bool& sendpacket)
{
    if (!g_LocalPlayer && !g_LocalPlayer->IsAlive() && g_EngineClient->IsConnected() && g_EngineClient->IsInGame())
        return;

    //desync(cmd, sendpacket);
    fakelag(cmd, sendpacket);
}