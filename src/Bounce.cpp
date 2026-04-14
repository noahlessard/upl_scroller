#include "Bounce.h"
#include "mainLoop.h"
#include "ImageLoader.h"
#include "CairoOverlay.h"
#include "Logging.h"

#include <cairo/cairo.h>

// Bouncing animation globals
static double  g_bounce_x = 0.0;
static double  g_bounce_y = 0.0;
static double  g_bounce_vx = 2.0;  // pixels per frame
static double  g_bounce_vy = 2.0;
static bool    g_bounce_enabled = false;

static cairo_surface_t* g_img_surface = nullptr;

static void log_bounce_event(const char* edge, double x, double vx) {
    LOG("bounce: hit %s edge at x=%.0f (vx=%.1f -> %.1f)",
        edge, x, vx, -vx);
}

void bounce_init(bool enabled) {
    g_bounce_enabled = enabled;
    g_bounce_x = 0.0;
    g_bounce_y = 0.0;
    g_bounce_vx = 2.0;
    g_bounce_vy = 2.0;
    g_img_surface = nullptr;
}

void bounce_load_image(const char* path) {
    if (path && !g_img_surface) {
        g_img_surface = image_load_jpeg(path, 100, 100);
        if (g_img_surface) {
            LOG("loaded bouncing image %dx%d",
                (int)cairo_image_surface_get_width(g_img_surface),
                (int)cairo_image_surface_get_height(g_img_surface));
        } else {
            LOG("failed to load bouncing image %s, using placeholder", path);
            g_img_surface = image_create_placeholder(100, 100);
        }
    }
}

bool bounce_update() {
    if (!g_bounce_enabled || !g_img_surface) return false;

    // image_load_jpeg already scaled to fit 100x100, so surface dims are final
    int img_w = (int)cairo_image_surface_get_width(g_img_surface);
    int img_h = (int)cairo_image_surface_get_height(g_img_surface);

    g_bounce_x += g_bounce_vx;
    g_bounce_y += g_bounce_vy;

    if (g_bounce_x + img_w >= OVERLAY_W) {
        g_bounce_x = OVERLAY_W - img_w;
        g_bounce_vx = -g_bounce_vx;
        log_bounce_event("RIGHT", g_bounce_x, g_bounce_vx);
    }
    if (g_bounce_x <= 0) {
        g_bounce_x = 0;
        g_bounce_vx = -g_bounce_vx;
        log_bounce_event("LEFT", g_bounce_x, g_bounce_vx);
    }
    if (g_bounce_y + img_h >= OVERLAY_H) {
        g_bounce_y = OVERLAY_H - img_h;
        g_bounce_vy = -g_bounce_vy;
        log_bounce_event("BOTTOM", g_bounce_y, g_bounce_vy);
    }
    if (g_bounce_y <= 0) {
        g_bounce_y = 0;
        g_bounce_vy = -g_bounce_vy;
        log_bounce_event("TOP", g_bounce_y, g_bounce_vy);
    }

    return true;
}

void bounce_get_position(int* x, int* y, int* w, int* h) {
    if (!g_img_surface || !g_bounce_enabled) {
        *x = *y = 0;
        *w = *h = 0;
        return;
    }
    *x = (int)g_bounce_x;
    *y = (int)g_bounce_y;
    *w = (int)cairo_image_surface_get_width(g_img_surface);
    *h = (int)cairo_image_surface_get_height(g_img_surface);
}

void bounce_draw() {
    if (!g_bounce_enabled) return;
    if (!g_img_surface) {
        LOG("no bouncing image");
        return;
    }

    int x, y, w, h;
    bounce_get_position(&x, &y, &w, &h);
    LOG("bouncing at (%d,%d) size %dx%d", x, y, w, h);

    clear_to_transparent();
    cairo_set_source_surface(g_cr, g_img_surface, x, y);
    cairo_paint(g_cr);
}

void bounce_shutdown() {
    if (g_img_surface) {
        cairo_surface_destroy(g_img_surface);
        g_img_surface = nullptr;
    }
    g_bounce_enabled = false;
}
