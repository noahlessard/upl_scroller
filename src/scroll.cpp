#include "scroll.h"
#include <algorithm>
#include <cstdio>

std::pair<float, float> calc_sync_speeds(int len1, int len2, unsigned win_width, float base_speed) {
    float dist1    = len1 * BIG_FONT_SPACING + win_width;
    float dist2    = len2 * BIG_FONT_SPACING + win_width;
    float max_dist = std::max(dist1, dist2);
    return { base_speed * (max_dist / dist1), base_speed * (max_dist / dist2) };
}

// Loads a letter PNG once, blits + recolors it, returns the plane.
// Caller owns the plane; pass anchor as parent so it follows anchor moves.
static ncplane* create_letter_plane(notcurses* nc, ncplane* parent, char letter, int y, int x,
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
        .n       = child,
        .scaling = NCSCALE_STRETCH,
        .blitter = NCBLIT_2x2,
    };
    ncvisual_blit(nc, nv, &vopts);
    ncvisual_destroy(nv);

    // Remap bright pixels → fg_color, dark pixels → bg_color
    for (unsigned row = 0; row < BIG_FONT_ROW_SIZE; row++) {
        for (unsigned col = 0; col < BIG_FONT_COLS_SIZE; col++) {
            nccell c = NCCELL_TRIVIAL_INITIALIZER;
            if (ncplane_at_yx_cell(child, row, col, &c) < 0) continue;

            auto lum = [](uint32_t rgb) {
                return ((rgb >> 16) & 0xFF) + ((rgb >> 8) & 0xFF) + (rgb & 0xFF);
            };
            nccell_set_fg_rgb(&c, lum(nccell_fg_rgb(&c)) > 384 ? fg_color : bg_color);
            nccell_set_bg_rgb(&c, lum(nccell_bg_rgb(&c)) > 384 ? fg_color : bg_color);

            ncplane_putc_yx(child, row, col, &c);
            nccell_release(child, &c);
        }
    }
    return child;
}


Scroll::Scroll(std::string_view text_, int y_, float speed_, ScrollDirection dir_,
    unsigned win_width, uint32_t fg_color_, uint32_t bg_color_)
    : text(text_), y(y_), speed(speed_), dir(dir_),
      accumulator(0.f), done(false), plane(nullptr),
      fg_color(fg_color_), bg_color(bg_color_)
{
    // this needs to change depending on left or right
    scroll_x = (dir == SCROLL_RIGHT)
        ? -BIG_FONT_SPACING * (int)text.size()
        :  (int)win_width;
}

void run_scrolls(notcurses* nc, ncplane* std_plane, std::vector<Scroll>& scrolls) {
    float tick = scrolls[0].speed;
    for (auto& s : scrolls)
        tick = std::min(tick, s.speed);

    unsigned h, w;
    ncplane_dim_yx(std_plane, &h, &w);

    // Build one transparent anchor plane per scroll; letter planes are children at fixed
    // relative offsets. A single ncplane_move_yx on the anchor moves everything together.
    for (auto& s : scrolls) {
        int len = (int)s.text.size();

        struct ncplane_options anchor_opts = {
            .y    = s.y,
            .x    = s.scroll_x,
            .rows = BIG_FONT_ROW_SIZE,
            .cols = (unsigned)(len * BIG_FONT_SPACING + BIG_FONT_COLS_SIZE),
        };
        s.plane = ncplane_create(std_plane, &anchor_opts);

        nccell bc = NCCELL_TRIVIAL_INITIALIZER;
        nccell_set_fg_alpha(&bc, NCALPHA_TRANSPARENT);
        nccell_set_bg_alpha(&bc, NCALPHA_TRANSPARENT);
        ncplane_set_base_cell(s.plane, &bc);
        nccell_release(s.plane, &bc);

        s.planes.resize(len, nullptr);
        for (int i = 0; i < len; i++) {
            if (s.text[i] != ' ')
                s.planes[i] = create_letter_plane(nc, s.plane, s.text[i],
                                                  0, i * BIG_FONT_SPACING,
                                                  s.fg_color, s.bg_color);
        }
    }

    while (true) {
        for (auto& s : scrolls)
            if (!s.done)
                ncplane_move_yx(s.plane, s.y, s.scroll_x);

        notcurses_render(nc);

        bool all_done = true;
        for (auto& s : scrolls) {
            s.accumulator += tick;
            if (s.accumulator >= s.speed) {
                s.accumulator -= s.speed;

                s.scroll_x += (s.dir == SCROLL_RIGHT) ? 1 : -1;

                // Done when the LAST letter to exit has fully left the screen:
                //   SCROLL_RIGHT → leftmost letter (scroll_x) clears the right edge
                //   SCROLL_LEFT  → rightmost letter clears the left edge
                int last_x = s.scroll_x + (int)(s.text.size() - 1) * BIG_FONT_SPACING;
                s.done = (s.dir == SCROLL_RIGHT)
                    ? s.scroll_x >= (int)w
                    : last_x     <= -BIG_FONT_SPACING;
            }
            if (!s.done) all_done = false;
        }
        if (all_done) break;
        usleep(tick * 1000000);
    }

    for (auto& s : scrolls) {
        for (auto* p : s.planes)
            if (p) ncplane_destroy(p);
        s.planes.clear();
        if (s.plane) { ncplane_destroy(s.plane); s.plane = nullptr; }
    }
}
