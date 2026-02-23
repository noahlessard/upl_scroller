#pragma once
#include <notcurses/notcurses.h>
#include "mainLoop.h"
#include <vector>
#include <string>
#include <string_view>
#include <utility>

enum ScrollDirection { SCROLL_LEFT, SCROLL_RIGHT };

struct Scroll {
    std::string           text;
    int                   y;
    float                 speed;
    ScrollDirection       dir;
    int                   scroll_x;     // x of anchor plane (= left edge of letter[0])
    float                 accumulator;
    bool                  done;
    ncplane*              plane;        // transparent anchor; letter planes are children
    std::vector<ncplane*> planes;
    uint32_t              fg_color;
    uint32_t              bg_color;

    Scroll(std::string_view text, int y, float speed, ScrollDirection dir,
           unsigned win_width, uint32_t fg_color = 0xFFFFFF, uint32_t bg_color = 0x000000);
};

std::pair<float, float> calc_sync_speeds(int len1, int len2, unsigned win_width, float base_speed);
void run_scrolls(notcurses* nc, ncplane* std_plane, std::vector<Scroll>& scrolls);
