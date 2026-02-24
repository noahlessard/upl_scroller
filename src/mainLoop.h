#pragma once
#include <string_view>
#include <vector>
#include <string>
constexpr unsigned BIG_FONT_ROW_SIZE  = 20;
constexpr unsigned BIG_FONT_COLS_SIZE = 22;
constexpr int      BIG_FONT_SPACING   = 10;
constexpr double   SPEED              = 0.1;

static void run_reel(std::string_view text, float speed);
static void returnToNormalBoarder(bool textAtBottom);
static std::vector<std::string> wrapText(std::string_view text, unsigned max_width);
static void createAlertWindow(std::string_view titleText, std::string_view bodyText, float duration);