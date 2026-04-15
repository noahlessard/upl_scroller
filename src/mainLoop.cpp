#include "mainLoop.h"
#include "Logging.h"
#include "MpvIpc.h"
#include "CairoOverlay.h"
#include "Bounce.h"
#include "FontLoader.h"
#include "scroll.h"

#include <cairo/cairo.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <cstdint>
#include <cstdio>
#include <thread>
#include <chrono>

// ── Globals ───────────────────────────────────────────────────────────────────
cairo_surface_t* g_surface  = nullptr;
cairo_t*         g_cr       = nullptr;
int              g_mpv_sock = -1;
static uint8_t*  g_shm_data = nullptr;
static int       g_shm_size = 0;

static constexpr const char* SHM_PATH = "/dev/shm/overlay.bgra";

// ── SHM/Cairo init ────────────────────────────────────────────────────────────
static bool init_shm_cairo() {
    int stride = OVERLAY_W * 4;
    int shm_size = stride * OVERLAY_H;

    LOG("shm: opening %s (size=%d)", SHM_PATH, shm_size);
    int fd = open(SHM_PATH, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        LOG("shm: open failed: %s", strerror(errno));
        return false;
    }
    if (ftruncate(fd, shm_size) < 0) {
        LOG("shm: ftruncate failed: %s", strerror(errno));
        close(fd);
        return false;
    }

    g_shm_data = (uint8_t*)mmap(nullptr, shm_size,
                                PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    if (g_shm_data == MAP_FAILED) {
        LOG("shm: mmap failed: %s", strerror(errno));
        return false;
    }
    g_shm_size = shm_size;
    LOG("shm: mmap OK at %p (%d bytes)", (void*)g_shm_data, g_shm_size);

    // CAIRO_FORMAT_ARGB32 = premul ARGB in native 32-bit order
    // On little-endian (Pi) bytes in memory are: B G R A  →  exactly MPV's BGRA
    g_surface = cairo_image_surface_create_for_data(
        g_shm_data, CAIRO_FORMAT_ARGB32, OVERLAY_W, OVERLAY_H, stride);
    g_cr = cairo_create(g_surface);

    cairo_status_t ss = cairo_surface_status(g_surface);
    cairo_status_t cs = cairo_status(g_cr);
    LOG("cairo surface status: %s", cairo_status_to_string(ss));
    LOG("cairo context status: %s", cairo_status_to_string(cs));
    if (ss != CAIRO_STATUS_SUCCESS || cs != CAIRO_STATUS_SUCCESS) return false;

    return true;
}

// ── Main ──────────────────────────────────────────────────────────────────────
int main() {
    // ── Initialize subsystems ─────────────────────────────────────────────────
#if ENABLE_LOGGING
    logging_init();
    LOG("upl_scroller starting (overlay %dx%d at %d,%d, shm=%s)",
            OVERLAY_W, OVERLAY_H, OVERLAY_X, OVERLAY_Y, SHM_PATH);
#endif

    // SHM + Cairo
    if (!init_shm_cairo()) {
        LOG("FATAL: init_shm_cairo failed");
        fprintf(stderr, "Cairo init failed\n"); return 1;
    }
    LOG("init_shm_cairo OK (%d bytes mapped)", g_shm_size);

    // Font
    font_init("pix.ttf");
    LOG("font_init OK");

    // Scroll
    scroll_init();
    LOG("scroll_init OK");

    // MPV connection
    if (!mpv_connect()) {
        LOG("FATAL: mpv_connect failed - is mpv running with "
                "--input-ipc-server=/tmp/mpvsock ?");
        fprintf(stderr, "MPV connect failed\n"); return 1;
    }

    // Query mpv properties
    LOG("querying mpv properties...");
    mpv_query_props();

    // ── Start bouncing animation (scans /static and loads random image) ─────────
    LOG("starting bouncing animation");
    bounce_init(true);

    // ── Test phase: display initial image for 5 seconds ───────────────────────
    LOG("TEST: displaying random image at top-left corner");
    cairo_surface_t* img = bounce_get_current_surface();
    if (img) {
        int w = (int)cairo_image_surface_get_width(img);
        int h = (int)cairo_image_surface_get_height(img);
        LOG("drawing image %dx%d at (0,0)", w, h);

        cairo_save(g_cr);
        cairo_set_source_surface(g_cr, img, 0, 0);
        cairo_paint(g_cr);
        cairo_restore(g_cr);

        cairo_surface_flush(g_surface);
        mpv_present_overlay();

        LOG("holding for 5 seconds...");
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    // Drain mpv responses
    drain_mpv_replies();

    // ── Second query ───────────────────────────────────────────────────────────
    LOG("re-querying mpv properties (VO should be running)...");
    mpv_query_props();

    // Main loop: keep refreshing so mpv doesn't drop overlay
    // Frame rate: 100ms (~10fps) with bouncing animation and scrolling text
    while (true) {
        // Update animation states
        bounce_update();
        scroll_update();

        // Draw bouncing image (in front)
        bounce_draw();

        // Draw scrolling text (behind image, at bottom)
        scroll_draw();

        mpv_present_overlay();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // ── Cleanup (unreachable in normal operation) ──────────────────────────────
    scroll_shutdown();
    bounce_shutdown();
    font_shutdown();
    mpv_shutdown();
    cairo_destroy(g_cr);
    cairo_surface_destroy(g_surface);
    munmap(g_shm_data, g_shm_size);
    logging_shutdown();
    return 0;
}
