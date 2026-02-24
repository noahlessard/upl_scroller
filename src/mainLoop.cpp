#include "scroll.h"
#include "mainLoop.h"
#include <thread>
#include <chrono>
#include <cstdlib>

static void playMp3(const char* path) {
    system("amixer set PCM 60%");
    system(("cvlc --play-and-exit " + std::string(path) + " 2>/dev/null").c_str());
    system("amixer set PCM 10%");
}

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

static std::vector<std::string> wrapText(std::string_view text, unsigned max_width) {
    std::vector<std::string> lines;
    size_t pos = 0;
    while (pos < text.length()) {
        size_t end = pos + max_width;
        if (end >= text.length()) {
            lines.push_back(std::string(text.substr(pos)));
            break;
        }
        size_t space_pos = text.rfind(' ', end);
        if (space_pos == std::string::npos || space_pos <= pos)
            space_pos = end;
        lines.push_back(std::string(text.substr(pos, space_pos - pos)));
        pos = text.find_first_not_of(' ', space_pos);
        if (pos == std::string::npos) break;
    }
    return lines;
}

static void createAlertWindow(std::string_view titleText, std::string_view bodyText, float duration) {
    constexpr unsigned MAX_CONTENT_WIDTH = 40;
    constexpr unsigned BOX_PADDING = 4;

    auto wrapped_lines = wrapText(bodyText, MAX_CONTENT_WIDTH);
    unsigned box_width  = MAX_CONTENT_WIDTH + BOX_PADDING;
    unsigned box_height = wrapped_lines.size() + 4;

    struct ncplane_options opts = {
        .y    = (int)(win_height - box_height) / 2,
        .x    = (int)(win_width  - box_width)  / 2,
        .rows = (int)box_height,
        .cols = (int)box_width,
    };
    ncplane* alert_plane = ncplane_create(std_plane, &opts);

    ncplane_set_fg_rgb8(alert_plane, 255, 255, 255);
    ncplane_set_bg_rgb8(alert_plane, 200, 0, 0);

    nccell base_cell = NCCELL_TRIVIAL_INITIALIZER;
    nccell_set_bg_rgb8(&base_cell, 200, 0, 0);
    ncplane_set_base_cell(alert_plane, &base_cell);

    uint64_t chan = 0;
    ncchannels_set_fg_rgb8(&chan, 255, 255, 255);
    ncchannels_set_bg_rgb8(&chan, 200, 0, 0);
    ncplane_rounded_box(alert_plane, 0, chan, box_height - 1, box_width - 1, 0);

    ncplane_putstr_yx(alert_plane, 1, 2, titleText.data());
    for (size_t i = 0; i < wrapped_lines.size(); ++i)
        ncplane_putstr_yx(alert_plane, 2 + (int)i, 2, wrapped_lines[i].c_str());

    notcurses_render(nc);
    std::this_thread::sleep_for(std::chrono::milliseconds((int)(duration * 1000)));
    ncplane_destroy(alert_plane);
    notcurses_render(nc);
}

int main() {
    setlocale(LC_ALL, "");
    notcurses_options opts = {};
    nc = notcurses_init(&opts, nullptr);
    std_plane = notcurses_stdplane(nc);

    ncplane_dim_yx(std_plane, &win_height, &win_width);

    returnToNormalBoarder(true);

    std::this_thread::sleep_for(std::chrono::seconds(2));
    //run_reel("  UPL TRAIN CAM  *  24 HRS A DAY   ", 0.1);

    createAlertWindow("摧毁我肥兔的生活", "mister beast better get me the new 6 7 matcha labubu clairo vinyl dubai chocoalte trump announced dead at 79 sports car", 3.0f);  // Shows for 3 seconds
    playMp3("bing.mp3");


    while (true){
        run_reel("UPL TRAIN CAM * 24 HOURS A DAY * NON STOP * SERIOUSLY... *", 0.15);
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    
    
    notcurses_stop(nc);
    return 0;
}
