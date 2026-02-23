#include "mainLoop.h"

std::pair<float, float> calc_sync_speeds(int len1, int len2, unsigned win_width, float base_speed) {
    float dist1 = len1 * BIG_FONT_SPACING + win_width;
    float dist2 = len2 * BIG_FONT_SPACING + win_width;
    float max_dist = std::max(dist1, dist2);
    // Shorter text gets a slower (larger) speed value
    return { base_speed * (max_dist / dist1), base_speed * (max_dist / dist2) };
}

// Creates a letter plane once; caller owns the plane and must destroy it.
ncplane* create_letter_plane(notcurses* nc, ncplane* parent, char letter, int y, int x,
    uint32_t fg_color, uint32_t bg_color) {
    char filename[256];
    snprintf(filename, sizeof(filename), "%s/letter_%c.png", FONTS_PATH, letter);
    ncvisual* nv = ncvisual_from_file(filename);
    if (!nv) return nullptr;

    struct ncplane_options popts = {
        .y = y, .x = x,
        .rows = BIG_FONT_ROW_SIZE, .cols = BIG_FONT_COLS_SIZE,
    };
    ncplane* child = ncplane_create(parent, &popts);
    if (!child) { ncvisual_destroy(nv); return nullptr; }

    struct ncvisual_options vopts = {
        .n = child,
        .scaling = NCSCALE_STRETCH,
        .blitter = NCBLIT_2x2,
    };
    ncvisual_blit(nc, nv, &vopts);
    ncvisual_destroy(nv);

    // Recolor cells after blit — remap bright pixels to fg_color, dark to bg_color
    for (unsigned row = 0; row < BIG_FONT_ROW_SIZE; row++) {
        for (unsigned col = 0; col < BIG_FONT_COLS_SIZE; col++) {
            nccell c = NCCELL_TRIVIAL_INITIALIZER;
            if (ncplane_at_yx_cell(child, row, col, &c) < 0) continue;

            uint32_t fg = nccell_fg_rgb(&c);
            uint32_t bg = nccell_bg_rgb(&c);

            auto lum = [](uint32_t rgb) {
                return ((rgb >> 16) & 0xFF) + ((rgb >> 8) & 0xFF) + (rgb & 0xFF);
            };

            nccell_set_fg_rgb(&c, lum(fg) > 384 ? fg_color : bg_color);
            nccell_set_bg_rgb(&c, lum(bg) > 384 ? fg_color : bg_color);

            ncplane_putc_yx(child, row, col, &c);
            nccell_release(child, &c);
        }
    }
    return child;
}

ScrollEntry make_scroll(std::string_view text, int y, float speed,
    ScrollDirection dir, unsigned win_width,
    uint32_t fg_color = 0xFFFFFF, uint32_t bg_color = 0x000000) {
    ScrollEntry e;
    e.text = text;
    e.y = y;
    e.speed = speed;
    e.dir = dir;
    e.accumulator = 0;
    e.done = false;
    e.fg_color = fg_color;
    e.bg_color = bg_color;

    int len = e.text.size();

    for (int i = 0; i < len; i++) {
        if (dir == SCROLL_RIGHT) {
            // Start off-screen left
            e.positions.push_back(-BIG_FONT_SPACING * (len - i));
        }
        else {
            // Start off-screen right
            e.positions.push_back((int)win_width + BIG_FONT_SPACING * i);
        }
    }
    return e;
}

void update_scroll(ScrollEntry& e, unsigned win_width) {
    if (e.done) return;

    int dir = (e.dir == SCROLL_RIGHT) ? 1 : -1;
    bool all_offscreen = true;

    for (size_t i = 0; i < e.positions.size(); i++) {
        e.positions[i] += dir;

        if (e.dir == SCROLL_RIGHT && e.positions[i] < (int)win_width)
            all_offscreen = false;
        if (e.dir == SCROLL_LEFT && e.positions[i] > -BIG_FONT_SPACING)
            all_offscreen = false;
    }
    e.done = all_offscreen;
}

void run_scrolls(notcurses* nc, ncplane* std, std::vector<ScrollEntry>& entries) {
    float tick = entries[0].speed;
    for (auto& e : entries)
        tick = std::min(tick, e.speed);

    unsigned h, w;
    ncplane_dim_yx(std, &h, &w);

    // Pre-create all letter planes once — no more per-frame file I/O or blitting
    for (auto& e : entries) {
        e.planes.resize(e.text.size(), nullptr);
        for (size_t i = 0; i < e.text.size(); i++) {
            if (e.text[i] != ' ')
                e.planes[i] = create_letter_plane(nc, std, e.text[i], e.y, e.positions[i], e.fg_color, e.bg_color);
        }
    }

    while (true) {
        // Just reposition existing planes — no create/destroy/blit
        for (auto& e : entries) {
            if (e.done) continue;
            for (size_t i = 0; i < e.text.size(); i++) {
                if (e.planes[i])
                    ncplane_move_yx(e.planes[i], e.y, e.positions[i]);
            }
        }

        notcurses_render(nc);

        bool all_done = true;
        for (auto& e : entries) {
            e.accumulator += tick;
            if (e.accumulator >= e.speed) {
                e.accumulator -= e.speed;
                update_scroll(e, w);
            }
            if (!e.done) all_done = false;
        }
        if (all_done) break;
        usleep(tick * 1000000);
    }

    // Clean up planes
    for (auto& e : entries) {
        for (auto* p : e.planes)
            if (p) ncplane_destroy(p);
        e.planes.clear();
    }
}

int main() {

    notcurses_options opts = {};
    notcurses* nc = notcurses_init(&opts, nullptr);
    ncplane* std = notcurses_stdplane(nc);

    unsigned win_height, win_width;
    ncplane_dim_yx(std, &win_height, &win_width);

    std::string_view scroll1 = {"UPL TRAIN CAM"};
    std::string_view scroll2 {"24 HRS A DAY"};

    auto [s1, s2] = calc_sync_speeds(scroll1.length(), scroll2.length(), win_width, SPEED);

    std::vector<ScrollEntry> entries;
    entries.push_back(make_scroll(scroll1, (win_height - BIG_FONT_ROW_SIZE) / 2 - 4, s1, SCROLL_RIGHT, win_width, 0x800000, 0xFFFFFF));
    entries.push_back(make_scroll(scroll2, (win_height - BIG_FONT_ROW_SIZE) / 2 + 6, s2, SCROLL_LEFT, win_width, 0x00FF00, 0x000000));

    run_scrolls(nc, std, entries);

    notcurses_stop(nc);
    return 0;
}
