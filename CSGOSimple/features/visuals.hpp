#pragma once

#include "../singleton.hpp"

#include "../render.hpp"
#include "../helpers/math.hpp"
#include "../valve_sdk/csgostructs.hpp"


extern unsigned long esp_font;


class Renders
	: public Singleton<Renders>
{
	friend class Singleton<Renders>;
private:

public:
	vgui::HFont VerdanaBold12;
	vgui::HFont icon;
	void CreateFonts();
	void Text(int X, int Y, const char* Text, vgui::HFont Font, Color DrawColor, bool Center);
	void TextSize(int& Width, int& Height, const char* Text, vgui::HFont Font);
	void FilledRectange(int X1, int Y1, int X2, int Y2, Color DrawColor);
	void OutlinedRectange(int X1, int Y1, int X2, int Y2, Color DrawColor);
	void Line(int X1, int Y1, int X2, int Y2, Color DrawColor);
};


class Visuals : public Singleton<Visuals>
{
	friend class Singleton<Visuals>;

	CRITICAL_SECTION cs;

	Visuals();
	~Visuals();
public:
	class Player
	{
	public:
		struct
		{
			C_BasePlayer* pl;
			bool          is_enemy;
			bool          is_visible;
			Color         clr;
			Vector        head_pos;
			Vector        feet_pos;
			RECT          bbox;
			float percent = 1.f;
		} ctx;

		bool Begin(C_BasePlayer * pl);
		void RenderBox();
		void RenderName();
		void RenderWeaponName();
		void RenderHealth();
		void RenderArmour();
		void renderammo();
		void RenderSnapline();
	};
	void NightMode();
	void ScopeLine();
	void Spectators();
	void CrosshairRecoil();
	void RenderCrosshair();
	void RenderWeapon(C_BaseCombatWeapon* ent);
	void RenderDefuseKit(C_BaseEntity* ent);
	void RenderPlantedC4(C_BaseEntity* ent);
	void RenderItemEsp(C_BaseEntity* ent);
	void ThirdPerson();
	//void backtrackline(C_BaseEntity* ent);
	
public:
	void AddToDrawList();
	void DrawString(unsigned long font, int x, int y, Color color, unsigned long alignment, const char* msg, ...);
	bool CreateFonts();
	void Render();
};
