#include "hooks.hpp"
#include <intrin.h>  
#include "features/ragebot.h"
#include "render.hpp"
#include "menu.hpp"
#include "options.hpp"
#include "backtrack.hpp"
#include "helpers/input.hpp"
#include "helpers/utils.hpp"
#include "features/bhop.hpp"
#include "features/chams.hpp"
#include "features/visuals.hpp"
#include "features/glow.hpp"
#include "bulletbeams.hpp"
#include "hitmarker.hpp"
#include "features/antiaim.hpp"
#include "features/triggerbot.h"
#include "features/engineprediction.hpp"
#include "features/tickbase.h"
#pragma intrinsic(_ReturnAddress)  

namespace Hooks {



	void Initialize()
	{
		hlclient_hook.setup(g_CHLClient);
		direct3d_hook.setup(g_D3DDevice9);
		vguipanel_hook.setup(g_VGuiPanel);
		vguisurf_hook.setup(g_VGuiSurface);
		sound_hook.setup(g_EngineSound);
		mdlrender_hook.setup(g_MdlRender);
		clientmode_hook.setup(g_ClientMode);
		ConVar* sv_cheats_con = g_CVar->FindVar("sv_cheats");
		sv_cheats.setup(sv_cheats_con);

		direct3d_hook.hook_index(index::EndScene, hkEndScene);
		direct3d_hook.hook_index(index::Reset, hkReset);
		hlclient_hook.hook_index(index::FrameStageNotify, hkFrameStageNotify);
		hlclient_hook.hook_index(index::CreateMove, hkCreateMove_Proxy);
		vguipanel_hook.hook_index(index::PaintTraverse, hkPaintTraverse);
		sound_hook.hook_index(index::EmitSound1, hkEmitSound1);
		vguisurf_hook.hook_index(index::LockCursor, hkLockCursor);
		mdlrender_hook.hook_index(index::DrawModelExecute, hkDrawModelExecute);
		//hlclient_hook.hook_index(index::hkWriteUsercmdDeltaToBuffer, hkWriteUsercmdDeltaToBuffer);
		clientmode_hook.hook_index(index::DoPostScreenSpaceEffects, hkDoPostScreenEffects);
		clientmode_hook.hook_index(index::OverrideView, hkOverrideView);
		sv_cheats.hook_index(index::SvCheatsGetBool, hkSvCheatsGetBool);

		Renders::Get().CreateFonts();
	}
	//--------------------------------------------------------------------------------
	void Shutdown()
	{
		hlclient_hook.unhook_all();
		direct3d_hook.unhook_all();
		vguipanel_hook.unhook_all();
		vguisurf_hook.unhook_all();
		mdlrender_hook.unhook_all();
		clientmode_hook.unhook_all();
		sound_hook.unhook_all();
		sv_cheats.unhook_all();

		Glow::Get().Shutdown();
	}
	void WriteUsercmd(bf_write* m_pBuffer, CUserCmd* m_pIncoming, CUserCmd* m_pOutgoing)
	{
		static DWORD dwWriteUsercmd = (DWORD)Utils::PatternScan(GetModuleHandleA("client.dll"), "55 8B EC 83 E4 F8 51 53 56 8B D9 8B 0D");
		__asm
		{
			mov     ecx, m_pBuffer
			mov     edx, m_pIncoming
			push    m_pOutgoing
			call    dwWriteUsercmd
			add     esp, 4
		}
	}
	//--------------------------------------------------------------------------------
	long __stdcall hkEndScene(IDirect3DDevice9* pDevice)
	{
		static auto oEndScene = direct3d_hook.get_original<decltype(&hkEndScene)>(index::EndScene);

		static auto viewmodel_fov = g_CVar->FindVar("viewmodel_fov");
		static auto mat_ambient_light_r = g_CVar->FindVar("mat_ambient_light_r");
		static auto mat_ambient_light_g = g_CVar->FindVar("mat_ambient_light_g");
		static auto mat_ambient_light_b = g_CVar->FindVar("mat_ambient_light_b");
		static auto crosshair_cvar = g_CVar->FindVar("crosshair");

		viewmodel_fov->m_fnChangeCallbacks.m_Size = 0;
		if (g_Options.viewmodel_fov)
			viewmodel_fov->SetValue(90);
		else
			viewmodel_fov->SetValue(68);

		
		
		DWORD colorwrite, srgbwrite;
		IDirect3DVertexDeclaration9* vert_dec = nullptr;
		IDirect3DVertexShader9* vert_shader = nullptr;
		DWORD dwOld_D3DRS_COLORWRITEENABLE = NULL;
		pDevice->GetRenderState(D3DRS_COLORWRITEENABLE, &colorwrite);
		pDevice->GetRenderState(D3DRS_SRGBWRITEENABLE, &srgbwrite);

		pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0xffffffff);
		//removes the source engine color correction
		pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, false);

		pDevice->GetRenderState(D3DRS_COLORWRITEENABLE, &dwOld_D3DRS_COLORWRITEENABLE);
		pDevice->GetVertexDeclaration(&vert_dec);
		pDevice->GetVertexShader(&vert_shader);
		pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0xffffffff);
		pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, false);
		pDevice->SetSamplerState(NULL, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
		pDevice->SetSamplerState(NULL, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
		pDevice->SetSamplerState(NULL, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP);
		pDevice->SetSamplerState(NULL, D3DSAMP_SRGBTEXTURE, NULL);

		
		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();


		auto esp_drawlist = Render::Get().RenderScene();

		Menu::Get().Render();
	

		ImGui::Render(esp_drawlist);

		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

		pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, colorwrite);
		pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, srgbwrite);
		pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, dwOld_D3DRS_COLORWRITEENABLE);
		pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, true);
		pDevice->SetVertexDeclaration(vert_dec);
		pDevice->SetVertexShader(vert_shader);

		return oEndScene(pDevice);
	}
	//--------------------------------------------------------------------------------
	long __stdcall hkReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters)
	{
		static auto oReset = direct3d_hook.get_original<decltype(&hkReset)>(index::Reset);

		Menu::Get().OnDeviceLost();

		auto hr = oReset(device, pPresentationParameters);

		if (hr >= 0)
		{
			Menu::Get().OnDeviceReset();
			Renders::Get().CreateFonts();
		}

		return hr;
	}
	//--------------------------------------------------------------------------------
	CMoveData bMoveData[0x200];
	void Prediction(CUserCmd* pCmd, C_BasePlayer* LocalPlayer)
	{
		if (g_MoveHelper && LocalPlayer->IsAlive())
		{
			float curtime = g_GlobalVars->curtime;
			float frametime = g_GlobalVars->frametime;
			int iFlags = LocalPlayer->m_fFlags();

			g_GlobalVars->curtime = (float)LocalPlayer->m_nTickBase() * g_GlobalVars->interval_per_tick;
			g_GlobalVars->frametime = g_GlobalVars->interval_per_tick;

			g_MoveHelper->SetHost(LocalPlayer);

			g_Prediction->SetupMove(LocalPlayer, pCmd, nullptr, bMoveData);
			g_GameMovement->ProcessMovement(LocalPlayer, bMoveData);
			g_Prediction->FinishMove(LocalPlayer, pCmd, bMoveData);

			g_MoveHelper->SetHost(0);

			g_GlobalVars->curtime = curtime;
			g_GlobalVars->frametime = frametime;
			LocalPlayer->m_fFlags() = iFlags;
		}
	}




	bool __fastcall hkWriteUsercmdDeltaToBuffer(IBaseClientDLL* ecx, void*, int m_nSlot, bf_write* m_pBuffer, int m_nFrom, int m_nTo, bool m_bNewCmd)
	{
		static auto oFunc = hlclient_hook.get_original< bool(__thiscall*)(IBaseClientDLL*, int, bf_write*, int, int, bool) >(24);
		int m_nTickbase = g_ViolanesPaster.g_nTickbase;
		if (m_nTickbase >= 0)
			return oFunc(ecx, m_nSlot, m_pBuffer, m_nFrom, m_nTo, m_bNewCmd);

		if (m_nFrom != -1)
			return true;

		g_ViolanesPaster.g_nTickbase = 0;
		m_nFrom = -1;

		*(int*)((uintptr_t)m_pBuffer - 0x30) = 0;
		int* m_pnNewCmds = (int*)((uintptr_t)m_pBuffer - 0x2C);

		int m_nNewCmds = *m_pnNewCmds;
		int m_nNextCmd = g_ClientState->m_nChokedCmds + g_ClientState->m_nLastOutgoingCmd + 1;
		int m_nTotalNewCmds = std::min(m_nNewCmds + abs(m_nTickbase), 62);

		*m_pnNewCmds = m_nTotalNewCmds;

		for (m_nTo = m_nNextCmd - m_nNewCmds + 1; m_nTo <= m_nNextCmd; m_nTo++)
		{
			if (!oFunc(ecx, m_nSlot, m_pBuffer, m_nFrom, m_nTo, true))
				return false;

			m_nFrom = m_nTo;
		}

		CUserCmd* m_pCmd = g_Input->GetUserCmd(m_nSlot, m_nFrom);
		if (!m_pCmd)
			return true;

		CUserCmd m_FromCmd = *m_pCmd, m_ToCmd = *m_pCmd;
		m_ToCmd.tick_count += 200;
		m_ToCmd.command_number++;

		for (int i = m_nNewCmds; i <= m_nTotalNewCmds; i++)
		{
			WriteUsercmd(m_pBuffer, &m_ToCmd, &m_FromCmd);
			m_FromCmd = m_ToCmd;

			m_ToCmd.command_number++;
			m_ToCmd.tick_count++;
		}

		return true;
	}




	C_BasePlayer* pLocal2;
	void __stdcall hkCreateMove(int sequence_number, float input_sample_frametime, bool active, bool& bSendPacket)
	{
		static auto oCreateMove = hlclient_hook.get_original<decltype(&hkCreateMove_Proxy)>(index::CreateMove);

		oCreateMove(g_CHLClient, 0, sequence_number, input_sample_frametime, active);

		auto cmd = g_Input->GetUserCmd(sequence_number);
		auto verified = g_Input->GetVerifiedCmd(sequence_number);

		if (!cmd || !cmd->command_number)
			return;
		if (!g_ViolanesPaster.CanProcessPacket(cmd))
		{
			bSendPacket = false;
			return;
		}

		C_BaseCombatWeapon* Weapon = g_LocalPlayer->m_hActiveWeapon();
		if (Menu::Get().IsVisible())
			cmd->buttons &= ~IN_ATTACK;

		if (g_Options.misc_bhop)
		{
			BunnyHop::OnCreateMove(cmd);
			if(g_Options.legit_enable)
			BunnyHop::AutoStrafe(cmd);
			else if (g_Options.rage_enable)
			BunnyHop::ragestrafer(cmd);
		}
		
		PredictionSystem::Get().Start(cmd, g_LocalPlayer);
		g_ViolanesPaster.g_nTicks = cmd->tick_count;
		if (g_Options.misc_fakelag)
			antiaim::Get().createmove(cmd, bSendPacket);
		//PredictionSystem::Get().Start(cmd, g_LocalPlayer);
		
			if (g_Options.rage_enable)
			{
				if (g_Options.legit_enable)
					g_Options.legit_enable, false;
				if (g_Options.misc_backtrack)
					g_Options.misc_backtrack, false;
				/*	if (Variables.LegitAntiaimEnabled)
						Variables.LegitAntiaimEnabled = false;*/
			}
			if (g_Options.RageAntiaimEnabled)
			{
				g_ViolanesPaster.DoubleTap(cmd, &bSendPacket);
				
				MovementFix::Get().Start(cmd);
				RageAimbot::Get().DoAntiaim(cmd, Weapon, bSendPacket);
				MovementFix::Get().End(cmd);
			}

			if (g_Options.rage_enable)
			{
				/*	if (g_Options.RageAimbotResolver)
					RageAimbot::Get().AntiFreestanding();*/
				RageAimbot::Get().Do(cmd, Weapon, bSendPacket);
			}

			if (g_Options.legit_enable)
				LegitAimbot::Get().Do(cmd, Weapon);


		g_ViolanesPaster.ShiftTickbase(cmd, bSendPacket);
		PredictionSystem::Get().End(g_LocalPlayer);
		// https://github.com/spirthack/CSGOSimple/issues/69
		if (g_Options.misc_showranks && cmd->buttons & IN_SCORE) // rank revealer will work even after unhooking, idk how to "hide" ranks  again
			g_CHLClient->DispatchUserMessage(CS_UM_ServerRankRevealAll, 0, 0, nullptr);

		if (bSendPacket)
		{
			RageAimbot::Get().RealAngle = cmd->viewangles;
		}
		else
		{
			RageAimbot::Get().FakeAngle = cmd->viewangles;
		}


		//clamping movement
		cmd->forwardmove = std::clamp(cmd->forwardmove, -450.0f, 450.0f);
		cmd->sidemove = std::clamp(cmd->sidemove, -450.0f, 450.0f);
		cmd->upmove = std::clamp(cmd->upmove, -450.0f, 450.0f);

		//// clamping angles
		//cmd->viewangles.pitch = std::clamp(cmd->viewangles.pitch, -89.0f, 89.0f);
		//cmd->viewangles.yaw = std::clamp(cmd->viewangles.yaw, -180.0f, 180.0f);
		//cmd->viewangles.roll = 0.0f;



		verified->m_cmd = *cmd;
		verified->m_crc = cmd->GetChecksum();

		//Prediction::StartPrediction(cmd, g_LocalPlayer);
		//{

		//}Prediction::EndPrediction(g_LocalPlayer);
	}
	//--------------------------------------------------------------------------------
	__declspec(naked) void __fastcall hkCreateMove_Proxy(void* _this, int, int sequence_number, float input_sample_frametime, bool active)
	{
		__asm
		{
			push ebp
			mov  ebp, esp
			push ebx; not sure if we need this
			push esp
			push dword ptr[active]
			push dword ptr[input_sample_frametime]
			push dword ptr[sequence_number]
			call Hooks::hkCreateMove
			pop  ebx
			pop  ebp
			retn 0Ch
		}
	}
	//--------------------------------------------------------------------------------
	void __fastcall hkPaintTraverse(void* _this, int edx, vgui::VPANEL panel, bool forceRepaint, bool allowForce)
	{
		static auto panelId = vgui::VPANEL{ 0 };

		if (g_Options.misc_removezoom)
		{
			if (strstr(g_VGuiPanel->GetName(panel), "HudZoom")) {
				if (g_EngineClient->IsConnected() && g_EngineClient->IsInGame())
					return;
			}
		}


		static auto oPaintTraverse = vguipanel_hook.get_original<decltype(&hkPaintTraverse)>(index::PaintTraverse);

		oPaintTraverse(g_VGuiPanel, edx, panel, forceRepaint, allowForce);

		if (!panelId) {
			const auto panelName = g_VGuiPanel->GetName(panel);
			if (!strcmp(panelName, "FocusOverlayPanel")) {
				panelId = panel;
			}
		}
		else if (panelId == panel) 
		{
			//Ignore 50% cuz it called very often
			static bool bSkip = false;
			bSkip = !bSkip;

			if (bSkip)
				return;

			Render::Get().BeginScene();
		}

		if (g_Options.misc_hitmarker)
		{
			HitMarkerEvent::Get().Paint();
		}

	}
	//--------------------------------------------------------------------------------
	void __fastcall hkEmitSound1(void* _this, int edx, IRecipientFilter& filter, int iEntIndex, int iChannel, const char* pSoundEntry, unsigned int nSoundEntryHash, const char* pSample, float flVolume, int nSeed, float flAttenuation, int iFlags, int iPitch, const Vector* pOrigin, const Vector* pDirection, void* pUtlVecOrigins, bool bUpdatePositions, float soundtime, int speakerentity, int unk) {
		static auto ofunc = sound_hook.get_original<decltype(&hkEmitSound1)>(index::EmitSound1);


		if (!strcmp(pSoundEntry, "UIPanorama.popup_accept_match_beep")) {
			static auto fnAccept = reinterpret_cast<bool(__stdcall*)(const char*)>(Utils::PatternScan(GetModuleHandleA("client.dll"), "55 8B EC 83 E4 F8 8B 4D 08 BA ? ? ? ? E8 ? ? ? ? 85 C0 75 12"));

			if (fnAccept) {

				fnAccept("");

				//This will flash the CSGO window on the taskbar
				//so we know a game was found (you cant hear the beep sometimes cause it auto-accepts too fast)
				FLASHWINFO fi;
				fi.cbSize = sizeof(FLASHWINFO);
				fi.hwnd = InputSys::Get().GetMainWindow();
				fi.dwFlags = FLASHW_ALL | FLASHW_TIMERNOFG;
				fi.uCount = 0;
				fi.dwTimeout = 0;
				FlashWindowEx(&fi);
			}
		}

		ofunc(g_EngineSound, edx, filter, iEntIndex, iChannel, pSoundEntry, nSoundEntryHash, pSample, flVolume, nSeed, flAttenuation, iFlags, iPitch, pOrigin, pDirection, pUtlVecOrigins, bUpdatePositions, soundtime, speakerentity, unk);

	}
	//--------------------------------------------------------------------------------
	int __fastcall hkDoPostScreenEffects(void* _this, int edx, int a1)
	{
		static auto oDoPostScreenEffects = clientmode_hook.get_original<decltype(&hkDoPostScreenEffects)>(index::DoPostScreenSpaceEffects);

		BulletBeamsEvent::Get().Paint();

		if (g_LocalPlayer && g_Options.glow_enabled)
			Glow::Get().Run();

		return oDoPostScreenEffects(g_ClientMode, edx, a1);
	}
	//--------------------------------------------------------------------------------
	void __fastcall hkFrameStageNotify(void* _this, int edx, ClientFrameStage_t stage)
	{
		static auto ofunc = hlclient_hook.get_original<decltype(&hkFrameStageNotify)>(index::FrameStageNotify);
		//static auto backtrack_init = (backtrack.init(), false);

		if (g_EngineClient->IsInGame() && g_EngineClient->IsConnected()) 
			Visuals::Get().NightMode();		

		/*
Visuals::Get().ScopeLine();*/
		std::vector<const char*> smoke_materials = {
"particle/beam_smoke_01",
"particle/particle_smokegrenade",
"particle/particle_smokegrenade1",
"particle/particle_smokegrenade2",
"particle/particle_smokegrenade3",
"particle/particle_smokegrenade_sc",
"particle/smoke1/smoke1",
"particle/smoke1/smoke1_ash",
"particle/smoke1/smoke1_nearcull",
"particle/smoke1/smoke1_nearcull2",
"particle/smoke1/smoke1_snow",
"particle/smokesprites_0001",
"particle/smokestack",
"particle/vistasmokev1/vistasmokev1",
"particle/vistasmokev1/vistasmokev1_emods",
"particle/vistasmokev1/vistasmokev1_emods_impactdust",
"particle/vistasmokev1/vistasmokev1_fire",
"particle/vistasmokev1/vistasmokev1_nearcull",
"particle/vistasmokev1/vistasmokev1_nearcull_fog",
"particle/vistasmokev1/vistasmokev1_nearcull_nodepth",
"particle/vistasmokev1/vistasmokev1_smokegrenade",
"particle/vistasmokev1/vistasmokev4_emods_nocull",
"particle/vistasmokev1/vistasmokev4_nearcull",
"particle/vistasmokev1/vistasmokev4_nocull"
		};

		if (stage == FRAME_NET_UPDATE_START)
		{


			if (g_EngineClient->IsInGame() && g_EngineClient->IsConnected())
			{
				for (auto material_name : smoke_materials) {
					IMaterial* mat = g_MatSystem->FindMaterial(material_name, TEXTURE_GROUP_OTHER);
					mat->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, g_Options.misc_nosmoke ? true : false);
				}
				if (g_Options.misc_nosmoke) {
					static auto smokecount = *(DWORD*)(Utils::PatternScan(GetModuleHandleW(L"client.dll"), "55 8B EC 83 EC 08 8B 15 ? ? ? ? 0F 57 C0") + 0x8);
					//static int* smokecount = *(int**)(Utils::PatternScan("client.dll", "8B 1D ? ? ? ? 56 33 F6 57 85 DB") + 0x2);
					if (!smokecount)
						return;
					*(int*)(smokecount) = 0;
				}
			}
		}

		if (stage == FRAME_RENDER_START)
		{
			for (int i = 1; i <= 64; i++)
			{
				C_BasePlayer* Player = C_BasePlayer::GetPlayerByIndex(i);
				if (!Player || !Player->IsPlayer() || Player == g_LocalPlayer) continue;

				*(int*)((uintptr_t)Player + 0xA30) = g_GlobalVars->framecount;
				*(int*)((uintptr_t)Player + 0xA28) = 0;
			}
		

			if (g_Options.misc_thirdperson)
			{
				static bool enabledtp = false, check = false;

				if (GetKeyState(0x56) && g_LocalPlayer->IsAlive())
				{			
					if (!check)
						enabledtp = !enabledtp;
					check = true;
				}
				else
					check = false;
				if (enabledtp)
				{
					*(QAngle*)((DWORD)g_LocalPlayer.operator->() + 0x31D8) = RageAimbot::Get().RealAngle;
				}
				if (g_Input->m_fCameraInThirdPerson)
				{
					//    I::Prediction1->set_local_viewangles_rebuilt(LastAngleAAReal);
					QAngle viewangs = *(QAngle*)((DWORD)g_LocalPlayer.operator->() + 0x31D8); viewangs = RageAimbot::Get().RealAngle;
				}
				if (enabledtp && g_LocalPlayer->IsAlive())
				{
					if (!g_Input->m_fCameraInThirdPerson)
					{
						g_Input->m_fCameraInThirdPerson = true;
						Vector camForward;
					}
				}
				else
				{
					g_Input->m_fCameraInThirdPerson = false;
				}
				if (g_Input->m_fCameraInThirdPerson)
				{
					*(QAngle*)((DWORD)g_LocalPlayer.operator->() + 0x31D8) = RageAimbot::Get().RealAngle;
				}
			}

			RageAimbot::Get().LocalAnimationFix(g_LocalPlayer);




		}
		if (stage == FRAME_NET_UPDATE_POSTDATAUPDATE_END)
		{
			for (int i = 1; i <= 64; i++)
			{
				C_BasePlayer* Player = C_BasePlayer::GetPlayerByIndex(i);
				if (!Player
					|| !Player->IsAlive())
					continue;
				if (Player->IsDormant())
					continue;

				VarMapping_t* map = (VarMapping_t*)((uintptr_t)Player + 36);

				for (int i = 0; i < map->m_nInterpolatedEntries; i++)
				{
					VarMapEntry_t* e = &map->m_Entries[i];

					if (!e)
						continue;

					e->m_bNeedsToInterpolate = false;
				}
			}
		}
		if (g_Options.misc_grenadepreview && g_EngineClient->IsInGame() && g_EngineClient->IsConnected()) {
			static auto grenadepre = g_CVar->FindVar("cl_grenadepreview");
			grenadepre->SetValue(1);
		}

		if (stage == FRAME_NET_UPDATE_END)
		{
			
			if (g_Options.rage_enable)
			RageAimbot::Get().AnimationFix();
			RageAimbot::Get().StoreRecords();
		}
			

		ofunc(g_CHLClient, edx, stage);
	}
	//--------------------------------------------------------------------------------
	void __fastcall hkOverrideView(void* _this, int edx, CViewSetup* vsView)
	{
		static auto ofunc = clientmode_hook.get_original<decltype(&hkOverrideView)>(index::OverrideView);

		/*if (g_EngineClient->IsInGame() && vsView)
			Visuals::Get().ThirdPerson();*/

		ofunc(g_ClientMode, edx, vsView);
	}
	//--------------------------------------------------------------------------------
	void __fastcall hkLockCursor(void* _this)
	{
		static auto ofunc = vguisurf_hook.get_original<decltype(&hkLockCursor)>(index::LockCursor);

		if (Menu::Get().IsVisible()) {
			g_VGuiSurface->UnlockCursor();
			g_InputSystem->ResetInputState();
			return;
		}
		ofunc(g_VGuiSurface);

	}
	//--------------------------------------------------------------------------------
	void __fastcall hkDrawModelExecute(void* _this, int edx, IMatRenderContext* ctx, const DrawModelState_t& state, const ModelRenderInfo_t& pInfo, matrix3x4_t* pCustomBoneToWorld)
	{
		static auto ofunc = mdlrender_hook.get_original<decltype(&hkDrawModelExecute)>(index::DrawModelExecute);

		if (g_MdlRender->IsForcedMaterialOverride() &&
			!strstr(pInfo.pModel->szName, "arms") &&
			!strstr(pInfo.pModel->szName, "weapons/v_")) {
			return ofunc(_this, edx, ctx, state, pInfo, pCustomBoneToWorld);
		}

		Chams::Get().OnDrawModelExecute(ctx, state, pInfo, pCustomBoneToWorld);

		ofunc(_this, edx, ctx, state, pInfo, pCustomBoneToWorld);

		g_MdlRender->ForcedMaterialOverride(nullptr);
	}

	
	
	bool __fastcall hkSvCheatsGetBool(PVOID pConVar, void* edx)
	{
		static auto dwCAM_Think = Utils::PatternScan(GetModuleHandleW(L"client.dll"), "85 C0 75 30 38 86");
		static auto ofunc = sv_cheats.get_original<bool(__thiscall *)(PVOID)>(13);
		if (!ofunc)
			return false;

		if (reinterpret_cast<DWORD>(_ReturnAddress()) == reinterpret_cast<DWORD>(dwCAM_Think))
			return true;
		return ofunc(pConVar);
	}
}
