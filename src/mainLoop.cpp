#include "scroll.h"
#include "mainLoop.h"

struct Reel {
    ncplane* plane;
    int      x;
    int      len;
};

static void run_reel(notcurses* nc, Reel& r, int y, int w, float speed) {
    while (true) {
        ncplane_move_yx(r.plane, y, r.x);
        notcurses_render(nc);
        if (--r.x + r.len <= 0) r.x = w;
        usleep((useconds_t)(speed * 1000000));
    }
}

int main() {
    setlocale(LC_ALL, "");
    notcurses_options opts = {};
    notcurses* nc           = notcurses_init(&opts, nullptr);
    ncplane*   std_plane    = notcurses_stdplane(nc);

    unsigned win_height, win_width;
    ncplane_dim_yx(std_plane, &win_height, &win_width);

    std::string_view t1 = "UPL TRAIN CAM";
    std::string_view t2 = "24 HRS A DAY";
    auto [s1, s2] = calc_sync_speeds(t1.length(), t2.length(), win_width, SPEED);

    int y1 = (int)(win_height - BIG_FONT_ROW_SIZE) / 2 - 4;
    int y2 = (int)(win_height - BIG_FONT_ROW_SIZE) / 2 + 6;

    std::vector<Scroll> scrolls = {
        { t1, y1, s1, SCROLL_RIGHT, win_width, 0x800000, 0xFFFFFF },
        { t2, y2, s2, SCROLL_LEFT,  win_width, 0x00FF00, 0x000000 },
    };

    // Draw a box around the entire screen
    uint64_t chan = 0;
    ncchannels_set_fg_rgb8(&chan, 0, 0, 0);
    ncchannels_set_bg_rgb8(&chan, 0, 255, 0);
    ncplane_rounded_box(std_plane, 0, chan, win_height - 1, win_width - 1, 0);

    // Text at top and bottom — green background, black foreground
    ncplane_set_fg_rgb8(std_plane, 0, 0, 0);
    ncplane_set_bg_rgb8(std_plane, 0, 255, 0);
    ncplane_putstr_yx(std_plane, 0, 2, " UPL TRAIN CAM ");
    ncplane_putstr_yx(std_plane, (int)win_height - 1, 2, " 24 HRS A DAY ");

    ncplane_putstr_yx(std_plane, (int)win_height - 10, 10, " 24 HRS A DAY ");

    std::string_view reel_text = "  UPL TRAIN CAM  *  24 HRS A DAY  *  LIVE BROADCAST  *  ";
    struct ncplane_options rpopts = {
        .y    = (int)win_height - 1,
        .x    = (int)win_width,
        .rows = 1,
        .cols = (unsigned)reel_text.size(),
    };
    ncplane* rplane = ncplane_create(std_plane, &rpopts);
    ncplane_set_fg_rgb8(rplane, 0, 0, 0);
    ncplane_set_bg_rgb8(rplane, 0, 255, 0);
    ncplane_putstr(rplane, reel_text.data());
    Reel reel = { rplane, (int)win_width, (int)reel_text.size() };

    notcurses_render(nc);
    //run_scrolls(nc, std_plane, scrolls);
    run_reel(nc, reel, (int)win_height - 1, (int)win_width, SPEED);
    notcurses_stop(nc);
    return 0;
}
