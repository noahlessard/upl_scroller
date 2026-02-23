#include "scroll.h"
#include "mainLoop.h"
static std::pair<float, float> calc_sync_speeds(int len1, int len2, unsigned win_width, float base_speed) {
    float dist1    = len1 * BIG_FONT_SPACING + win_width;
    float dist2    = len2 * BIG_FONT_SPACING + win_width;
    float max_dist = std::max(dist1, dist2);
    return { base_speed * (max_dist / dist1), base_speed * (max_dist / dist2) };
}

int main() {
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

    run_scrolls(nc, std_plane, scrolls);
    notcurses_stop(nc);
    return 0;
}
