#pragma once
#include <atomic>
#include <thread>
// Forward declaration — full cairo.h is included via mainLoop.h in translation units that need it
struct _cairo_surface;
typedef struct _cairo_surface cairo_surface_t;

// External declaration of the global monitor instance
class ClaudeStatusMonitor;
extern ClaudeStatusMonitor g_claude_monitor;

class ClaudeStatusMonitor {
public:
    static constexpr int BOX_SIZE = 256;
    static constexpr int POLL_INTERVAL_SEC = 30;

    std::atomic<bool> is_down{false};
    std::atomic<bool> running{false};
    std::atomic<bool> test_mode_{true};
    std::atomic<bool> network_error_{false};

    void start();
    void stop();
    bool get_is_down() const;
    void render();

private:
    void load_faucet_image();
    void render_image(int x, int y, int w, int h);
    void fetch_status_json(std::string& response);
    void check_status();

    std::thread monitoring_thread;
    mutable cairo_surface_t* faucet_surf_;

public:
    ~ClaudeStatusMonitor();
};
