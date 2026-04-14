#pragma once
#include <cairo/cairo.h>
#include <string_view>
#include <vector>
#include <string>

// ── Overlay geometry (800×600 display) ───────────────────────────────────────
constexpr int OVERLAY_W = 800;
constexpr int OVERLAY_H = 600;
constexpr int OVERLAY_X = 0;   // full width
constexpr int OVERLAY_Y = 0;   // full height

// ── Bar / border sizes ────────────────────────────────────────────────────────
constexpr int TOP_BAR_H     = 17;
constexpr int BOTTOM_BAR_H  = 23;
constexpr int SIDE_BORDER_W = 3;

// ── Font sizes ────────────────────────────────────────────────────────────────
constexpr double LABEL_FONT_SZ      = 12.0;   // "UPL TRAIN CAM" / "24 HRS A DAY"
constexpr double TICKER_FONT_SZ     = 18.0;   // scrolling ticker
constexpr double ALERT_TITLE_SZ     = 14.0;
constexpr double ALERT_BODY_SZ      = 10.0;

// ── Scroll speed & frame timing ───────────────────────────────────────────────
constexpr float SCROLL_SPEED_PX = 3.0f;  // pixels per frame
constexpr int   FRAME_MS        = 33;    // ~30 fps

// ── Shared Cairo state (defined in mainLoop.cpp) ─────────────────────────────
extern cairo_surface_t* g_surface;
extern cairo_t*         g_cr;
extern int              g_mpv_sock;

// ── API used by scroll.cpp ────────────────────────────────────────────────────
void present_overlay();
void draw_border(bool show_bottom_label);
void create_alert(std::string_view title, std::string_view body, float duration_s);
