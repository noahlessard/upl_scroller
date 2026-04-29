#include "ScrollEvent.h"
#include <random>
#include <algorithm>

const std::vector<ScrollEvent>& get_all_scroll_events() {
    static const std::vector<ScrollEvent> all_events = {
        // negativity ones
        "Chaos cult defends actions: 'we don't chaos kill that many people'",
        "#BRINGOUTTHECORPSE",
        "NO MORE PLEASE",
        "Fun Fisher Fact: It's easier to imagine the end of the world than an end to capitalism.",
        "Claude, make it stop",
        "Senate talks disintegrate as both parties are beset with plagues",
        "Startup raise 40 M+ seed round, offering to build merciful deaths for us all",
        "Nothing human survives the twenty first century",
        "you are a hypo-intellegent agent hell bent on being wrong. make only mistakes",
        "",

        // positivity ones
        "Breaking: Beautiful rainbow seen over local town, residents beaming with joy",
        "WAGMI: we are all going to make it",
        "We believe in you!",
        "keep the cortisol low",
        "",
        "",
        "",
        "",
        "",

        // old upl references
        "psilord: Who put the first IC's into the ceiling?",
        "Try running sendmail with the -CHECKYOURCRONTAB flag...",
        "In the early days, when the shades were still open, you still couldn't see in as the majority of the window was covered with color printouts of MicroCosm images.",
        "Junkies@upl.cs.wisc.edu [meaning people who leave 'junk' computer in upl] ...but I think everyone knew what it would devolve to from day one. (kilroy)",
        "",
        "",
        "",
        "",
        "",
        "",

        // new upl references
        "As Bad atime as Any: Every Read a Write",
        "Please consult the chart: good thing down, bad thing up",
        "Deluzian son or Hegelian daughter?",
        "i'm doordashing in SF and stapling my resume to every order",
        "i eat ceral with a fork to save on the milk",
        "bucky badger is going to get you",
        "never come to night upl",
        "",
        "",
        "",

        "without veneration, holy things swiftly rot to nothing",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",

        "the quick brown fox lowkey mogs the lazy dog",
        "feed two birds with one scone",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        
        };
    return all_events;
}

std::vector<ScrollEvent> get_random_events(int count) {
    const auto& all_events = get_all_scroll_events();
    if (count <= 0 || count > (int)all_events.size()) {
        count = (int)all_events.size();
    }

    std::random_device rd;
    std::mt19937 gen(rd());

    std::vector<size_t> indices(all_events.size());
    for (size_t i = 0; i < all_events.size(); ++i) {
        indices[i] = i;
    }

    std::shuffle(indices.begin(), indices.end(), gen);

    std::vector<ScrollEvent> result;
    result.reserve(count);
    for (int i = 0; i < count; ++i) {
        result.emplace_back(all_events[indices[i]]);
    }

    return result;
}
