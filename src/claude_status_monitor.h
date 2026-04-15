#pragma once
#include <atomic>
#include <string>
#include <thread>
// Forward declaration — full cairo.h is included via mainLoop.h in translation units that need it
struct _cairo_surface;
typedef struct _cairo_surface cairo_surface_t;

// Forward declaration
class ClaudeStatusMonitor;

// External declaration of the global monitor instance
extern ClaudeStatusMonitor g_claude_monitor;

class ClaudeStatusMonitor {
public:
    static constexpr int BOX_SIZE = 256;
    static constexpr int POLL_INTERVAL_SEC = 30;

    std::atomic<bool> is_down{false};
    std::atomic<bool> running{false};
    std::atomic<bool> test_mode_{true};

    void start();
    void stop();
    bool get_is_down() const;
    std::string get_status() const;
    void render();
    void end_test_mode();

private:
    void load_faucet_image();
    void render_image(int x, int y, int w, int h);
    void fetch_status_html(std::string& response);
    void check_status();

    std::thread monitoring_thread;
    std::string last_status_;
    mutable cairo_surface_t* faucet_surf_;

public:
    ~ClaudeStatusMonitor();
};
