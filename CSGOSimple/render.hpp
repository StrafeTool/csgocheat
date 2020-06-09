#pragma once
#include <string>
#include <sstream>
#include <stdint.h>
#include <d3d9.h>
#include <d3dx9.h>
#pragma comment(lib,"d3d9.lib")
#pragma comment(lib,"d3dx9.lib")

#include "singleton.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/impl/imgui_impl_dx9.h"
#include "imgui/impl/imgui_impl_win32.h"

#include "valve_sdk/misc/Color.hpp"

#include "valve_sdk/csgostructs.hpp"

extern ImFont* g_pDefaultFont;
extern ImFont* g_pSecondFont;
extern ImFont* g_namefont;

class Vector;

class Render
	: public Singleton<Render>
{
	friend class Singleton<Render>;

private:
	ImDrawList * draw_list_act;
	ImDrawList * draw_list_rendering;
	ImDrawList* draw_list;
	ImDrawData draw_data;

	ImU32 GetU32(Color _color)
	{
		return ((_color[3] & 0xff) << 24) + ((_color[2] & 0xff) << 16) + ((_color[1] & 0xff) << 8)
			+ (_color[0] & 0xff);
	}
public:
	void Initialize();
	void GetFonts();
	void ClearDrawList();
	void BeginScene();
	ImDrawList* RenderScene();

	float RenderText(const std::string& text, ImVec2 position, float size, Color color, bool center = false, bool outline = true, ImFont* pFont = g_pDefaultFont);

	void RenderCircle3D(Vector position, float points, float radius, Color color);

	void RenderImage(ImTextureID user_texture_id, const ImVec2& a, const ImVec2& b, const ImVec2& uv_a = ImVec2(0, 0), const ImVec2& uv_b = ImVec2(1, 1), ImU32 col = 0xFFFFFFFF) {
		draw_list->AddImage(user_texture_id, a, b, uv_a, uv_b, col);
	}

	/*void draw_text(int x, int y, unsigned long font, std::string string, bool text_centered, Color colour) {
		const auto converted_text = std::wstring(string.begin(), string.end());

		int width, height;
		g_VGuiSurface->GetTextSize(font, converted_text.c_str(), width, height);

		g_VGuiSurface->DrawSetTextColor(colour.r, colour.g, colour.b, colour.a);
		g_VGuiSurface->DrawSetTextFont(font);
		if (text_centered)
			g_VGuiSurface->DrawSetTextPos(x - (width / 2), y);
		else
			g_VGuiSurface->DrawSetTextPos(x, y);
		g_VGuiSurface->DrawPrintText(converted_text.c_str(), wcslen(converted_text.c_str()));
	}*/

	template <class T>
	inline void RenderBoxByType(T x1, T y1, T x2, T y2, Color color, float thickness = 1.f, int type = 0) {
		if (type == 0)
			RenderBox(x1, y1, x2, y2, color, thickness);
		else if (type == 1)
			RenderCoalBox(x1, y1, x2, y2, color);
		else if (type == 2)
			RenderBox(x1, y1, x2, y2, color, thickness, 8.f);
	}

	template <class T>
	inline void RenderBoxFilledByType(T x1, T y1, T x2, T y2, Color color, float thickness = 1.f, int type = 0) {
		if (type == 0 || type == 1)
			RenderBoxFilled(x1, y1, x2, y2, color, thickness);
		else if (type == 2)
			RenderBoxFilled(x1, y1, x2, y2, color, thickness, 8.f);
	}

	template <class T>
	inline void RenderCoalBox(T x1, T y1, T x2, T y2, Color color, float th = 1.f) {
		int w = x2 - x1;
		int h = y2 - y1;

		int iw = w / 4;
		int ih = h / 4;
		// top
		RenderLine(x1, y1, x1 + iw, y1, color, th);					// left
		RenderLine(x1 + w - iw, y1, x1 + w, y1, color, th);			// right
		RenderLine(x1, y1, x1, y1 + ih, color, th);					// top left
		RenderLine(x1 + w - 1, y1, x1 + w - 1, y1 + ih, color, th);	// top right
																	// bottom
		RenderLine(x1, y1 + h, x1 + iw, y1 + h, color, th);			// left
		RenderLine(x1 + w - iw, y1 + h, x1 + w, y1 + h, color, th);	// right
		RenderLine(x1, y1 + h - ih, x1, y1 + h, color, th);			// bottom left
		RenderLine(x1 + w - 1, y1 + h - ih, x1 + w - 1, y1 + h, color, th);	// bottom right
	}

	template <class T>
	inline void RenderBox(T x1, T y1, T x2, T y2, Color color, float thickness = 1.f, float rounding = 0.f) {
		draw_list->AddRect(ImVec2(x1, y1), ImVec2(x2, y2), GetU32(color), rounding, 15, thickness);
	}


	inline void RenderBox(RECT r, Color color,float thickness = 1.f, float rounding = 0.f) {
		RenderBox(r.left, r.top, r.right, r.bottom, color, thickness, rounding);
	}
	template <class T>
	inline void RenderBoxFilled(T x1, T y1, T x2, T y2, Color color, float thickness = 1.f, float rounding = 0.f) {
		draw_list->AddRectFilled(ImVec2(x1, y1), ImVec2(x2, y2), GetU32(color), rounding, 15);
	}
	template <class T>
	inline void RenderLine(T x1, T y1, T x2, T y2, Color color, float thickness = 1.f) {
		draw_list->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), GetU32(color), thickness);
	}
	void box(const float x1, const float y1, const float x2, const float y2, const Color clr, const float size) const
	{
		Render::Get().RenderLine(x1, y1, x2, y1, clr, size);
		Render::Get().RenderLine(x1, y2, x2, y2, clr, size);
		Render::Get().RenderLine(x1, y1, x1, y2, clr, size);
		Render::Get().RenderLine(x2, y1, x2, y2, clr, size);
	}

	template <class T>
	inline float RenderText(const std::string& text, T x, T y, float size, Color clr, bool center = false, bool outline = true, ImFont* pFont = g_pDefaultFont) {
		return RenderText(text, ImVec2(x, y), size, clr, center, outline, pFont);
	}
	template <class T>
	inline void RenderCircle(T x, T y, float radius, int points, Color color, float thickness = 1.f) {
		draw_list->AddCircle(ImVec2(x, y), radius, GetU32(color), points, thickness);
	}
	template <class T>
	inline void RenderCircleFilled(T x, T y, float radius, int points, Color color) {
		draw_list->AddCircleFilled(ImVec2(x, y), radius, GetU32(color), points);
	}
};