#include "Bounce.h"
#include "mainLoop.h"
#include "ImageLoader.h"
#include "Logging.h"

#include <cairo/cairo.h>
#include <dirent.h>
#include <cstring>
#include <chrono>
#include <random>
#include <algorithm>

// Bouncing animation globals
static double  g_bounce_x = 0.0;
static double  g_bounce_y = 0.0;
static double  g_bounce_vx = 2.0;  // pixels per frame
static double  g_bounce_vy = 2.0;
static bool    g_bounce_enabled = false;

static cairo_surface_t* g_img_surface = nullptr;
static cairo_surface_t* g_logo_surface = nullptr;

// Random image support
static std::chrono::steady_clock::time_point g_last_image_change;
static constexpr int IMAGE_CHANGE_INTERVAL_SEC = 60;  // 1 minutes

static std::vector<const char*>& get_image_paths();

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
    get_image_paths().clear();
    g_last_image_change = std::chrono::steady_clock::now();
    // Scan for images on startup
    bounce_scan_images("static/bounce");
    if (!get_image_paths().empty()) {
        bounce_load_random_image();
    }
}

void bounce_scan_images(const char* folder) {
    auto& paths = get_image_paths();
    paths.clear();
    DIR* dir = opendir(folder);
    if (!dir) {
        LOG("failed to open folder %s: %s", folder, strerror(errno));
        return;
    }

    struct dirent* entry = nullptr;
    while ((entry = readdir(dir)) != nullptr) {
        const char* name = entry->d_name;
        // Check for .jpg or .jpeg extension (case-insensitive)
        size_t len = strlen(name);
        if (len > 4 && (strcmp(name + len - 4, ".jpg") == 0 ||
                        strcmp(name + len - 4, ".JPG") == 0 ||
                        strcmp(name + len - 5, ".jpeg") == 0)) {
            // Create a copy of the filename for the path
            char full_path[512];
            snprintf(full_path, sizeof(full_path), "%s/%s", folder, name);
            paths.push_back(strdup(full_path));
            LOG("found image: %s", full_path);
        }
    }
    closedir(dir);
    LOG("scanned %zu images in %s", g_image_paths.size(), folder);
}

const std::vector<const char*>& bounce_get_images() {
    return get_image_paths();
}

cairo_surface_t* bounce_get_current_surface() {
    return g_img_surface;
}

size_t bounce_get_image_count() {
    return get_image_paths().size();
}

void bounce_clear_image_paths() {
    auto& paths = get_image_paths();
    for (const char* path : paths) {
        free((void*)path);
    }
    paths.clear();
}

void bounce_load_random_image() {
    auto& paths = get_image_paths();
    if (paths.empty()) {
        LOG("no images found, using placeholder");
        g_img_surface = image_create_placeholder(MAX_BOUNCE_WIDTH, MAX_BOUNCE_HEIGHT);
        return;
    }

    // Pick random index
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dist(0, paths.size() - 1);
    size_t idx = dist(gen);

    const char* path = paths[idx];
    LOG("loading random image %s (index %zu of %zu)", path, idx, paths.size());

    // Destroy current surface
    if (g_img_surface) {
        cairo_surface_destroy(g_img_surface);
        g_img_surface = nullptr;
    }

    // Load new image
    g_img_surface = image_load_jpeg(path, MAX_BOUNCE_WIDTH, MAX_BOUNCE_HEIGHT);
    if (g_img_surface) {
        LOG("loaded bouncing image %dx%d",
            (int)cairo_image_surface_get_width(g_img_surface),
            (int)cairo_image_surface_get_height(g_img_surface));
    } else {
        LOG("failed to load bouncing image %s, using placeholder", path);
        g_img_surface = image_create_placeholder(MAX_BOUNCE_WIDTH, MAX_BOUNCE_HEIGHT);
    }

    g_last_image_change = std::chrono::steady_clock::now();
}

bool bounce_update() {
    if (!g_bounce_enabled || !g_img_surface) { return false; }

    // Check if it's time to load a new random image (every 1 minutes)
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - g_last_image_change).count();
    if (elapsed >= IMAGE_CHANGE_INTERVAL_SEC && !get_image_paths().empty()) {
        LOG("time to pick new random image (elapsed: %d seconds)", (int)elapsed);
        bounce_load_random_image();
    }

    // image_load_jpeg already scaled to fit, so surface dims are final
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
    if (!g_bounce_enabled) { return; }
    if (!g_img_surface) {
        LOG("no bouncing image");
        return;
    }

    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
    bounce_get_position(&x, &y, &w, &h);
    LOG("bouncing at (%d,%d) size %dx%d", x, y, w, h);

    cairo_set_source_surface(g_cr, g_img_surface, x, y);
    cairo_paint(g_cr);
}

void logo_init(const char* path) {
    g_logo_surface = image_load_jpeg(path, LOGO_SIZE, LOGO_SIZE);
    if (!g_logo_surface) {
        LOG("logo_init: failed to load %s", path);
    }
}

void logo_draw() {
    if (!g_logo_surface) { return; }
    cairo_set_source_surface(g_cr, g_logo_surface, 0, 0);
    cairo_paint(g_cr);
}

void bounce_shutdown() {
    if (g_img_surface) {
        cairo_surface_destroy(g_img_surface);
        g_img_surface = nullptr;
    }
    if (g_logo_surface) {
        cairo_surface_destroy(g_logo_surface);
        g_logo_surface = nullptr;
    }
    // Free image paths
    auto& paths = get_image_paths();
    for (const char* path : paths) {
        free((void*)path);
    }
    paths.clear();
    g_bounce_enabled = false;
}

static std::vector<const char*>& get_image_paths() {
    static std::vector<const char*> paths;
    return paths;
}
