#include "scroll.h"
#include "mainLoop.h"
#include <thread>
#include <chrono>

ncplane* std_plane;
unsigned win_height, win_width;
notcurses* nc;

static void run_reel(std::string_view text, float speed) {
    returnToNormalBoarder(false);
    struct ncplane_options opts = {
        .y    = (int)win_height - 1,
        .x    = (int)win_width,
        .rows = 1,
        .cols = (unsigned)text.size(),
    };
    ncplane* rplane = ncplane_create(std_plane, &opts);
    ncplane_set_fg_rgb8(rplane, 0, 0, 0);
    ncplane_set_bg_rgb8(rplane, 0, 255, 0);
    ncplane_putstr(rplane, text.data());
    
    int x = (int)win_width;
    int len = (int)text.size();
    
    // Scroll once
    while (x + len > 0) {
        ncplane_move_yx(rplane, (int)win_height - 1, x);
        notcurses_render(nc);
        --x;
        std::this_thread::sleep_for(std::chrono::milliseconds((int)(speed * 1000)));
    }
    
    ncplane_destroy(rplane);
    returnToNormalBoarder(true);

}

static void returnToNormalBoarder(bool textAtBottom) {
    ncplane_erase(std_plane);

    uint64_t chan = 0;
    ncchannels_set_fg_rgb8(&chan, 0, 0, 0);
    ncchannels_set_bg_rgb8(&chan, 0, 255, 0);
    ncplane_rounded_box(std_plane, 0, chan, win_height - 1, win_width - 1, 0);

    // Text at top and bottom — green background, black foreground
    ncplane_set_fg_rgb8(std_plane, 0, 0, 0);
    ncplane_set_bg_rgb8(std_plane, 0, 255, 0);
    ncplane_putstr_yx(std_plane, 0, 2, " UPL TRAIN CAM ");
    if (textAtBottom)
        ncplane_putstr_yx(std_plane, (int)win_height - 1, 2, " 24 HRS A DAY ");

    notcurses_render(nc);

}

int main() {
    setlocale(LC_ALL, "");
    notcurses_options opts = {};
    nc = notcurses_init(&opts, nullptr);
    std_plane = notcurses_stdplane(nc);

    ncplane_dim_yx(std_plane, &win_height, &win_width);

    returnToNormalBoarder(true);

    std::this_thread::sleep_for(std::chrono::seconds(5));
    //run_reel("  UPL TRAIN CAM  *  24 HRS A DAY   ", 0.1);

    run_reel(" mister beast better get me the new 6 7 matcha labubu clairo vinyl dubai chocoalte trump announced dead at 79 sports car", 0.15);
    
    
    notcurses_stop(nc);
    return 0;
}
