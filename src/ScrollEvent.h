#pragma once
#include <vector>
#include <string>
#include <utility>

// ============================================================================
// SCROLL EVENT SYSTEM - 100 NEWSWIRE-STYLE EVENTS
// ============================================================================
// Events are displayed 10 at a time, randomly selected from the full list.
// The system cycles through events every ~1 minute (scroll ~45s, wait ~15s).
// ============================================================================

class ScrollEvent {
public:
    ScrollEvent() = default;
    ScrollEvent(std::string text) : m_text(std::move(text)) {}

    [[nodiscard]] const std::string& text() const { return m_text; }

private:
    std::string m_text;
};

// Get the complete list of 100 scroll events
const std::vector<ScrollEvent>& get_all_scroll_events();

// Get a random selection of 10 events (without replacement)
std::vector<ScrollEvent> get_random_events(int count = 10);

// Get current events being scrolled
std::vector<ScrollEvent>& get_current_events();

// Set new random events (call this to refresh the scroll)
void set_new_events(int count = 10);
