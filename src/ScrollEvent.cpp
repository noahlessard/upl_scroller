#include "ScrollEvent.h"
#include <random>
#include <algorithm>
#include <chrono>
#include <unordered_set>

// ============================================================================
// 100 NEWSWIRE-STYLE SCROLL EVENTS
// ============================================================================
const std::vector<ScrollEvent>& get_all_scroll_events() {
    // Use static local to avoid non-POD static warning
    static const std::vector<ScrollEvent> all_events = {
        // negativity ones
        {"Chaos cult defends actions: 'we don't chaos kill that many people'"},
        {"#BRINGOUTTHECORPSE"},
        {"NO MORE PLEASE"},
        {"Fun Fisher Fact: It's easier to imagine the end of the world than an end to capitalism."},
        {"Claude, make it stop"},
        {"Senate talks disintegrate as both parties are beset with plagues"},
        {"Startup raise 40 M+ seed round, offering to build merciful deaths for us all"},
        {"Breaking: 300 miles of Connecticut just fell into the ocean"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        
        // positivity ones
        {"Breaking: Beautiful rainbow seen over local town, residents beaming with joy"},
        {"WAGMI: we are all going to make it"},
        {"We believe in you!"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        
        // old upl references
        {"psilord: Who put the first IC's into the ceiling?"},
        {"Try running sendmail with the -CHECKYOURCRONTAB flag..."},
        {"In the early days, when the shades were still open, you still couldn't see in as the majority of the window was covered with color printouts of MicroCosm images."},
        {"Junkies@upl.cs.wisc.edu [meaning people who leave 'junk' computer in upl] ...but I think everyone knew what it would devolve to from day one. (kilroy)"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        
        // new upl references
        {"As Bad atime as Any: Every Read a Write"},
        {"Please consult the chart: good thing down, bad thing up"},
        {"Deluzian son or Hegelian daughter?"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        
        // TECHNOLOGY & INNOVATION
        {"without veneration, holy things swiftly rot to nothing"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        
        // ENTERTAINMENT & MEDIA
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        
        // TRANSPORTATION & TRAVEL
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        
        // HEALTH & WELLNESS
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        
        // FINANCE & TAXES
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        
        // EDUCATION & LEARNING
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        
        // SPORTS & ATHLETICS
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        
        // ENVIRONMENT & SUSTAINABILITY
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        
        // GENERAL & MISCELLANEOUS
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"},
        {"this is a placeholder news scroll"}
    };
    return all_events;
}

// ============================================================================
// IMPLEMENTATION
// ============================================================================

std::vector<ScrollEvent> get_random_events(int count) {
    if (count <= 0) { count = 10; }
    const auto& all_events = get_all_scroll_events();
    if (count > (int)all_events.size()) { count = (int)all_events.size(); }
    
    std::random_device rd;
    std::mt19937 gen(rd());
    
    // Create index array
    std::vector<size_t> indices(all_events.size());
    for (size_t i = 0; i < all_events.size(); ++i) {
        indices[i] = i;
    }
    
    // Shuffle indices
    std::shuffle(indices.begin(), indices.end(), gen);
    
    // Select first 'count' indices
    std::vector<ScrollEvent> result;
    result.reserve(count);
    for (int i = 0; i < count; ++i) {
        result.emplace_back(all_events[indices[i]]);
    }
    
    return result;
}

std::vector<ScrollEvent>& get_current_events() {
    // Use static local to avoid non-POD static warning
    static std::vector<ScrollEvent> current_events;
    return current_events;
}

void set_new_events(int count) {
    get_current_events() = get_random_events(count);
}
