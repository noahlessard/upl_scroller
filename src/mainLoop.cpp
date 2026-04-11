#include "mainLoop.h"
#include "scroll.h"

#include <cairo/cairo.h>
#include <cairo/cairo-ft.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <chrono>
#include <string>
#include <vector>

// ── Globals ───────────────────────────────────────────────────────────────────
cairo_surface_t* g_surface  = nullptr;
cairo_t*         g_cr       = nullptr;
int              g_mpv_sock = -1;

static FT_Library          g_ft_lib   = nullptr;
static FT_Face             g_ft_face  = nullptr;
static cairo_font_face_t*  g_font     = nullptr;
static uint8_t*            g_shm_data = nullptr;
static int                 g_shm_size = 0;

static constexpr const char* SHM_PATH = "/dev/shm/overlay.bgra";
static constexpr const char* MPV_SOCK = "/tmp/mpvsock";
static constexpr const char* LOG_PATH = "/tmp/upl_scroller.log";

// ── Logging ───────────────────────────────────────────────────────────────────
static FILE* g_log = nullptr;

static void log_open() {
    g_log = fopen(LOG_PATH, "w");
    if (g_log) setvbuf(g_log, nullptr, _IOLBF, 0);  // line-buffered
}

static void log_msg(const char* fmt, ...) {
    if (!g_log) return;
    time_t now = time(nullptr);
    struct tm tmv;
    localtime_r(&now, &tmv);
    fprintf(g_log, "[%02d:%02d:%02d] ",
            tmv.tm_hour, tmv.tm_min, tmv.tm_sec);
    va_list ap;
    va_start(ap, fmt);
    vfprintf(g_log, fmt, ap);
    va_end(ap);
    fputc('\n', g_log);
    fflush(g_log);
}

// Drain any pending bytes from mpv (responses / events) into the log.
// Socket must be O_NONBLOCK for this to be safe.
static void drain_mpv_replies() {
    if (g_mpv_sock < 0) return;
    char buf[1024];
    for (;;) {
        ssize_t n = read(g_mpv_sock, buf, sizeof(buf) - 1);
        if (n > 0) {
            buf[n] = '\0';
            // Trim trailing newline so the log stays tidy
            while (n > 0 && (buf[n-1] == '\n' || buf[n-1] == '\r')) {
                buf[--n] = '\0';
            }
            log_msg("mpv -> %s", buf);
        } else if (n == 0) {
            log_msg("mpv socket closed by peer");
            close(g_mpv_sock);
            g_mpv_sock = -1;
            return;
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) return;
            log_msg("read(mpv) error: %s", strerror(errno));
            return;
        }
    }
}

// ── MPV IPC ───────────────────────────────────────────────────────────────────
static bool connect_mpv() {
    g_mpv_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (g_mpv_sock < 0) {
        log_msg("socket() failed: %s", strerror(errno));
        return false;
    }

    struct sockaddr_un addr = {};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, MPV_SOCK, sizeof(addr.sun_path) - 1);

    // Retry for up to 15 seconds while MPV is starting
    for (int i = 0; i < 30; ++i) {
        if (connect(g_mpv_sock, (sockaddr*)&addr, sizeof(addr)) == 0) {
            // Make non-blocking so drain_mpv_replies() can poll without stalling
            int fl = fcntl(g_mpv_sock, F_GETFL, 0);
            if (fl >= 0) fcntl(g_mpv_sock, F_SETFL, fl | O_NONBLOCK);
            log_msg("connected to mpv at %s after %d attempts", MPV_SOCK, i + 1);
            return true;
        }
        if (i == 0 || i == 29) {
            log_msg("connect(%s) attempt %d failed: %s",
                    MPV_SOCK, i + 1, strerror(errno));
        }
        usleep(500000);
    }
    return false;
}

void present_overlay() {
    cairo_surface_flush(g_surface);
    if (g_mpv_sock < 0) {
        log_msg("present_overlay: no mpv socket, skipping");
        return;
    }

    int stride = OVERLAY_W * 4;
    char cmd[512];
    // mpv overlay-add syntax:
    //   overlay-add <id> <x> <y> <file> <offset> <fmt> <w> <h> <stride>
    // fmt MUST be the literal string "bgra" (only supported format).
    int n = snprintf(cmd, sizeof(cmd),
        "{\"command\":[\"overlay-add\",1,%d,%d,\"%s\",0,\"bgra\",%d,%d,%d]}\n",
        OVERLAY_X, OVERLAY_Y, SHM_PATH, OVERLAY_W, OVERLAY_H, stride);

    static bool logged_first = false;
    if (!logged_first) {
        log_msg("first overlay-add cmd: %.*s", n - 1, cmd);  // strip trailing \n
        logged_first = true;
    }

    if (write(g_mpv_sock, cmd, (size_t)n) < 0) {
        log_msg("write(mpv) failed: %s — reconnecting", strerror(errno));
        close(g_mpv_sock);
        g_mpv_sock = -1;
        connect_mpv();
        return;
    }

    drain_mpv_replies();
}

// ── Drawing helpers ───────────────────────────────────────────────────────────
static void clear_to_transparent() {
    cairo_save(g_cr);
    cairo_set_operator(g_cr, CAIRO_OPERATOR_CLEAR);
    cairo_paint(g_cr);
    cairo_restore(g_cr);
}

static void set_green() { cairo_set_source_rgba(g_cr, 0, 1, 0, 1); }
static void set_black() { cairo_set_source_rgba(g_cr, 0, 0, 0, 1); }
static void set_white() { cairo_set_source_rgba(g_cr, 1, 1, 1, 1); }

// ── Border ────────────────────────────────────────────────────────────────────
void draw_border(bool show_bottom_label) {
    clear_to_transparent();

    set_green();

    // Top bar
    cairo_rectangle(g_cr, 0, 0, OVERLAY_W, TOP_BAR_H);
    cairo_fill(g_cr);

    // Bottom bar
    cairo_rectangle(g_cr, 0, OVERLAY_H - BOTTOM_BAR_H, OVERLAY_W, BOTTOM_BAR_H);
    cairo_fill(g_cr);

    // Left border strip
    cairo_rectangle(g_cr, 0, TOP_BAR_H, SIDE_BORDER_W,
                    OVERLAY_H - TOP_BAR_H - BOTTOM_BAR_H);
    cairo_fill(g_cr);

    // Right border strip
    cairo_rectangle(g_cr, OVERLAY_W - SIDE_BORDER_W, TOP_BAR_H,
                    SIDE_BORDER_W, OVERLAY_H - TOP_BAR_H - BOTTOM_BAR_H);
    cairo_fill(g_cr);

    // Top label
    cairo_set_font_size(g_cr, LABEL_FONT_SZ);
    set_black();
    cairo_move_to(g_cr, 10, TOP_BAR_H - 8);
    cairo_show_text(g_cr, " UPL TRAIN CAM ");

    // Bottom label (when no ticker is active)
    if (show_bottom_label) {
        cairo_move_to(g_cr, 10, OVERLAY_H - 12);
        cairo_show_text(g_cr, " 24 HRS A DAY ");
    }
}

// ── Alert window ──────────────────────────────────────────────────────────────
static std::vector<std::string> wrap_pixels(std::string_view text, double max_w) {
    std::vector<std::string> lines;
    std::string current;

    auto measure = [&](const std::string& s) -> double {
        cairo_text_extents_t e;
        cairo_text_extents(g_cr, s.c_str(), &e);
        return e.width;
    };

    size_t pos = 0;
    while (pos < text.size()) {
        size_t sp = text.find(' ', pos);
        if (sp == std::string_view::npos) sp = text.size();
        std::string word(text.substr(pos, sp - pos));
        std::string trial = current.empty() ? word : current + " " + word;
        if (!current.empty() && measure(trial) > max_w) {
            lines.push_back(current);
            current = word;
        } else {
            current = trial;
        }
        pos = sp + 1;
    }
    if (!current.empty()) lines.push_back(current);
    return lines;
}

void create_alert(std::string_view title, std::string_view body, float duration_s) {
    draw_border(true);

    cairo_set_font_size(g_cr, ALERT_BODY_SZ);
    double max_content_w = 380.0;
    auto lines = wrap_pixels(body, max_content_w);

    double line_h = ALERT_BODY_SZ + 6;
    double box_w  = max_content_w + 40;
    double box_h  = ALERT_TITLE_SZ + (double)lines.size() * line_h + 30;
    double bx     = (OVERLAY_W - box_w) / 2.0;
    double by     = (OVERLAY_H - box_h) / 2.0;

    // Red fill
    cairo_set_source_rgba(g_cr, 0.78, 0, 0, 0.97);
    cairo_rectangle(g_cr, bx, by, box_w, box_h);
    cairo_fill(g_cr);

    // White border
    set_white();
    cairo_set_line_width(g_cr, 2);
    cairo_rectangle(g_cr, bx, by, box_w, box_h);
    cairo_stroke(g_cr);

    // Title
    cairo_set_font_size(g_cr, ALERT_TITLE_SZ);
    set_white();
    cairo_move_to(g_cr, bx + 10, by + ALERT_TITLE_SZ + 6);
    cairo_show_text(g_cr, title.data());

    // Body
    cairo_set_font_size(g_cr, ALERT_BODY_SZ);
    for (size_t i = 0; i < lines.size(); ++i) {
        cairo_move_to(g_cr, bx + 10,
                      by + ALERT_TITLE_SZ + 6 + (double)(i + 1) * line_h + 8);
        cairo_show_text(g_cr, lines[i].c_str());
    }

    present_overlay();
    std::this_thread::sleep_for(std::chrono::milliseconds((int)(duration_s * 1000.0f)));
}

// ── Audio ─────────────────────────────────────────────────────────────────────
static void play_mp3(const char* path) {
    system("amixer set PCM 60% 2>&1 >/dev/null");
    system(("cvlc --play-and-exit " + std::string(path) + " 2>/dev/null").c_str());
    system("amixer set PCM 10% 2>&1 >/dev/null");
}

// ── Init ──────────────────────────────────────────────────────────────────────
static bool init_shm_cairo() {
    int stride = OVERLAY_W * 4;
    g_shm_size = stride * OVERLAY_H;

    int fd = open(SHM_PATH, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) { perror("open shm"); return false; }
    if (ftruncate(fd, g_shm_size) < 0) { perror("ftruncate"); close(fd); return false; }

    g_shm_data = (uint8_t*)mmap(nullptr, g_shm_size,
                                PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    if (g_shm_data == MAP_FAILED) { perror("mmap"); return false; }

    // CAIRO_FORMAT_ARGB32 = premul ARGB in native 32-bit order
    // On little-endian (Pi) bytes in memory are: B G R A  →  exactly MPV's BGRA
    g_surface = cairo_image_surface_create_for_data(
        g_shm_data, CAIRO_FORMAT_ARGB32, OVERLAY_W, OVERLAY_H, stride);
    g_cr = cairo_create(g_surface);
    return true;
}

static bool init_font() {
    if (FT_Init_FreeType(&g_ft_lib))                    return false;
    if (FT_New_Face(g_ft_lib, FONT_TTF, 0, &g_ft_face)) return false;

    g_font = cairo_ft_font_face_create_for_ft_face(g_ft_face, 0);
    cairo_set_font_face(g_cr, g_font);

    // Pixel fonts look best without anti-aliasing
    cairo_font_options_t* opts = cairo_font_options_create();
    cairo_font_options_set_antialias(opts, CAIRO_ANTIALIAS_NONE);
    cairo_font_options_set_hint_style(opts, CAIRO_HINT_STYLE_FULL);
    cairo_set_font_options(g_cr, opts);
    cairo_font_options_destroy(opts);

    cairo_set_font_size(g_cr, LABEL_FONT_SZ);
    return true;
}

// ── Main ──────────────────────────────────────────────────────────────────────
int main() {
    log_open();
    log_msg("upl_scroller starting (overlay %dx%d at %d,%d, shm=%s, sock=%s)",
            OVERLAY_W, OVERLAY_H, OVERLAY_X, OVERLAY_Y, SHM_PATH, MPV_SOCK);

    if (!init_shm_cairo()) {
        log_msg("FATAL: init_shm_cairo failed");
        fprintf(stderr, "Cairo init failed\n"); return 1;
    }
    log_msg("init_shm_cairo OK (%d bytes mapped at %s)", g_shm_size, SHM_PATH);

    if (!init_font()) {
        log_msg("FATAL: init_font failed (FONT_TTF=%s)", FONT_TTF);
        fprintf(stderr, "Font init failed\n");  return 1;
    }
    log_msg("init_font OK (%s)", FONT_TTF);

    if (!connect_mpv()) {
        log_msg("FATAL: connect_mpv failed — is mpv running with "
                "--input-ipc-server=%s ? check /tmp/mpv.log", MPV_SOCK);
        fprintf(stderr, "MPV connect failed\n"); return 1;
    }

    draw_border(true);
    present_overlay();

    std::this_thread::sleep_for(std::chrono::seconds(2));

    create_alert("摧毁我肥兔的生活",
        "mister beast better get me the new 6 7 matcha labubu clairo vinyl "
        "dubai chocolate trump announced dead at 79 sports car",
        3.0f);

    std::thread audio(play_mp3, "bing.mp3");
    audio.join();

    while (true) {
        std::vector<ScrollText> scrolls = {
            ScrollText(
                "UPL TRAIN CAM  *  24 HOURS A DAY  *  NON STOP  *  SERIOUSLY...  *",
                OVERLAY_H - 12,   // baseline: near bottom of the bottom bar
                SCROLL_SPEED_PX,
                Dir::Left)
        };
        run_scrolls(scrolls);
        draw_border(true);
        present_overlay();
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    // Cleanup (unreachable in normal operation)
    cairo_font_face_destroy(g_font);
    cairo_destroy(g_cr);
    cairo_surface_destroy(g_surface);
    munmap(g_shm_data, g_shm_size);
    FT_Done_Face(g_ft_face);
    FT_Done_FreeType(g_ft_lib);
    close(g_mpv_sock);
    return 0;
}
