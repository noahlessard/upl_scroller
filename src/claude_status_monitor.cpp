#include "claude_status_monitor.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <string>
#include <curl/curl.h>
#include "FontLoader.h"
#include "ImageLoader.h"
#include "Logging.h"
#include "mainLoop.h"

void ClaudeStatusMonitor::start() {
    if (running.load()) return;
    running.store(true);
    monitoring_thread = std::thread([this]() {
        while (running.load()) {
            check_status();
            std::this_thread::sleep_for(std::chrono::seconds(POLL_INTERVAL_SEC));
        }
    });
}

void ClaudeStatusMonitor::stop() {
    running.store(false);
    if (monitoring_thread.joinable()) {
        monitoring_thread.join();
    }
}

void ClaudeStatusMonitor::end_test_mode() {
    test_mode_.store(false);
}

bool ClaudeStatusMonitor::get_is_down() const {
    return is_down.load();
}

std::string ClaudeStatusMonitor::get_status() const {
    return last_status_;
}

void ClaudeStatusMonitor::render() {
    if (network_error_.load()) {
        font_set_size(ALERT_BODY_SZ);
        cairo_set_source_rgba(g_cr, 1.0, 0.0, 0.0, 1.0);
        cairo_move_to(g_cr, OVERLAY_X + OVERLAY_W - 100, OVERLAY_Y + 10);
        cairo_show_text(g_cr, "OFFLINE");
        return;
    }

    // Show alert in test mode (first run) OR if status is actually down
    if (!test_mode_.load() && !is_down.load()) return;

    // Draw red background box (top-right corner)
    cairo_set_source_rgba(g_cr, 1.0, 0.0, 0.0, 1.0);
    cairo_rectangle(g_cr, OVERLAY_X + OVERLAY_W - BOX_SIZE, OVERLAY_Y, BOX_SIZE, BOX_SIZE);
    cairo_fill(g_cr);

    // Draw "Claude is down" text at top center
    font_set_size(ALERT_TITLE_SZ);
    cairo_set_source_rgba(g_cr, 1.0, 1.0, 1.0, 1.0);
    const std::string message = "Claude is down";
    cairo_text_extents_t te;
    cairo_text_extents(g_cr, message.c_str(), &te);
    double text_width = te.x_advance;
    cairo_move_to(g_cr, (OVERLAY_X + OVERLAY_W - BOX_SIZE + (BOX_SIZE - text_width) / 2.0),
                         OVERLAY_Y + 35);
    cairo_show_text(g_cr, message.c_str());
    cairo_fill(g_cr);

    // Load and render faucet.jpg at bottom of box
    render_image(OVERLAY_X + OVERLAY_W - BOX_SIZE + 10, OVERLAY_Y + BOX_SIZE - 160, 136, 128);
}

ClaudeStatusMonitor::~ClaudeStatusMonitor() {
    stop();
    if (faucet_surf_) {
        cairo_surface_destroy(faucet_surf_);
    }
}

static size_t write_callback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    size_t total_size = size * nmemb;
    userp->append((char*)contents, total_size);
    return total_size;
}

void ClaudeStatusMonitor::load_faucet_image() {
    if (faucet_surf_) {
        cairo_surface_destroy(faucet_surf_);
    }
    faucet_surf_ = nullptr;
    std::string paths[] = {
        "static/faucet.jpg",
        "faucet.jpg",
        "./faucet.jpg",
        "../faucet.jpg"
    };
    for (const auto& p : paths) {
        faucet_surf_ = image_load_jpeg(p.c_str(), 256, 128);
        if (faucet_surf_) {
            LOG("Loaded faucet.jpg from %s", p.c_str());
            break;
        }
    }
    if (!faucet_surf_) {
        LOG("Warning: faucet.jpg not found");
    }
}

void ClaudeStatusMonitor::render_image(int x, int y, int w, int h) {
    if (!faucet_surf_) {
        load_faucet_image();
        if (!faucet_surf_) return;
    }

    cairo_surface_t* scaled = cairo_surface_create_similar_image(
        g_surface, CAIRO_FORMAT_ARGB32, w, h);
    cairo_t* cr = cairo_create(scaled);

    cairo_set_source_surface(cr, faucet_surf_, 0, 0);
    cairo_scale(cr, (double)w / cairo_image_surface_get_width(faucet_surf_),
                      (double)h / cairo_image_surface_get_height(faucet_surf_));
    cairo_paint(cr);

    cairo_destroy(cr);

    // Draw scaled image at position
    cairo_set_source_surface(g_cr, scaled, x, y);
    cairo_paint(g_cr);

    cairo_surface_destroy(scaled);
}

void ClaudeStatusMonitor::fetch_status_html(std::string& response) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        LOG("Failed to init curl");
        response = "";
        return;
    }

    // Use the official Statuspage JSON API instead of scraping HTML
    curl_easy_setopt(curl, CURLOPT_URL, "https://status.claude.com/api/v2/status.json");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        LOG("curl_easy_perform failed: %s", curl_easy_strerror(res));
        response = "";
    }

    curl_easy_cleanup(curl);
}

// Extract the value of a JSON string field: "key":"value"
static std::string extract_json_string(const std::string& json, const std::string& key) {
    std::string search = "\"" + key + "\":\"";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return "";
    pos += search.size();
    size_t end = json.find('"', pos);
    if (end == std::string::npos) return "";
    return json.substr(pos, end - pos);
}

void ClaudeStatusMonitor::check_status() {
    std::string json;
    fetch_status_html(json);

    if (json.empty()) {
        LOG("Claude status: fetch failed, keeping previous status");
        network_error_.store(true);
        return;
    }
    network_error_.store(false);
    test_mode_.store(false);

    // Response: {"status":{"indicator":"none"|"minor"|"major"|"critical","description":"..."}}
    std::string indicator = extract_json_string(json, "indicator");
    std::string description = extract_json_string(json, "description");

    LOG("Claude status API: indicator=%s description=%s", indicator.c_str(), description.c_str());

    if (indicator == "none") {
        last_status_ = "OK";
        is_down.store(false);
        LOG("Claude status: OK");
    } else {
        last_status_ = indicator.empty() ? "unknown" : indicator;
        is_down.store(true);
        LOG("Claude status: %s - DOWN (%s)", last_status_.c_str(), description.c_str());
    }
}
