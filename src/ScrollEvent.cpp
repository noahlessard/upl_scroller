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
        // MARKETING & BUSINESS
        {"🔥 FLASH SALE: 50% OFF SELECTED ITEMS - TODAY ONLY!"},
        {"💰 INVESTMENT ALERT: Tech stocks surge to record highs"},
        {"📈 MARKET UPDATE: Dow Jones gains 200 points in early trading"},
        {"🎯 NEW PRODUCT LAUNCH: Revolutionary device breaks pre-order records"},
        {"🏆 COMPANY NEWS: Local business wins national innovation award"},
        {"📊 STOCK UPDATE: Oil prices stabilize after volatile week"},
        {"🚀 STARTUP SPOTLIGHT: Homegrown company raises $10M in funding"},
        {"💼 JOB FAIR: 50+ local companies hiring this weekend"},
        {"🎉 LOCALENTERTAINMENT: Music festival announces headline acts"},
        {"🏪 RETAIL UPDATE: New shopping district opens downtown"},
        
        // WEATHER & TRAFFIC
        {"⛈️ WEATHER ALERT: Severe storms expected Tuesday afternoon"},
        {"🚦 TRAFFIC UPDATE: I-95 northbound reopening after accident"},
        {"❄️ SEASONAL: First snowfall of the season recorded in northern counties"},
        {"🌊 COASTAL ADVISORY: Beach erosion warning issued for south shore"},
        {"🚧 ROAD WORK: Main Street closed for repaving through Friday"},
        {"🌡️ TEMPERATURE ALERT: Heat wave continues through weekend"},
        {"🚌 TRANSIT UPDATE: Bus routes 12 and 45 detoured due to parade"},
        {"⚠️ AIR QUALITY: Poor air quality warning for sensitive groups"},
        {"🚗 PARKING ALERT: Downtown garage full during evening events"},
        {"🌨️ SNOW REMOVAL: Plow crews working through the night"},
        
        // COMMUNITY & LOCAL
        {"🎪 COMMUNITY EVENT: Free concert in Central Park tonight 7PM"},
        {"🍽️ RESTAURANT REVIEW: New farm-to-table spot impresses critics"},
        {"🎨 ART EXHIBITION: Local artists showcase work at City Gallery"},
        {"📚 LIBRARY NEWS: Free literacy program starts Monday"},
        {"🏃 FITNESS CLASS: Free outdoor yoga in the park every Sunday"},
        {"🍕 FOOD TRUCK: New Mexican cuisine arriving at downtown plaza"},
        {"🎭 THEATER: Community theater presents Shakespeare in the Park"},
        {"🌳 PARK CLEANUP: Volunteer event this Saturday 9AM-1PM"},
        {"🏀 SPORTS: Local team advances to championship finals"},
        {"🎉 HOLIDAY: Bank offices closed for observed holiday"},
        
        // CRIME & SAFETY
        {"⚠️ SAFETY ALERT: Beware of package theft in residential areas"},
        {"🚓 POLICE UPDATE: Suspect arrested in downtown incident"},
        {"🔒 SECURITY TIP: Update passwords to protect your accounts"},
        {"🚨 EMERGENCY: Flash flood warning in effect until 6PM"},
        {"🚦 CROSSING SAFETY: New crosswalk installation in progress"},
        {"🏥 HOSPITAL NOTICE: Blood drive needs type O donors"},
        {"⚡ POWER ALERT: Outage affecting 5000 residents"},
        {"🚒 FIRE DEPT: Fire prevention inspection week starts Monday"},
        {"🚓 NEIGHBORHOOD WATCH: Meeting this Thursday at 7PM"},
        {"🏠 HOME SAFETY: Free lock assessment program available"},
        
        // TECHNOLOGY & INNOVATION
        {"💻 TECH NEWS: Major software update released with security patches"},
        {"📱 APP REVIEW: New fitness app tops download charts"},
        {"🌐 INTERNET UPDATE: Fiber expansion reaching new neighborhoods"},
        {"🤖 AI NEWS: Local company develops innovative AI solution"},
        {"🔋 BATTERY ALERT: Recall announced for popular device battery"},
        {"📡 WIRELESS UPDATE: 5G coverage expands to suburban areas"},
        {"💾 DATA SECURITY: New identity theft protection service launched"},
        {"🎮 GAMING: Local esports team announces roster changes"},
        {"📷 CAMERA UPDATE: New smartphone camera tech impresses reviewers"},
        {"🖥️ REMOTE WORK: Company announces permanent hybrid policy"},
        
        // ENTERTAINMENT & MEDIA
        {"🎬 MOVIE NEWS: Local theater hosting film premiere event"},
        {"📺 TV UPDATE: Season finale draws record viewership numbers"},
        {"🎵 MUSIC: Local band signs with major record label"},
        {"🎤 CONCERT: Annual music festival announces complete lineup"},
        {"📰 OPINION: Editorial on community development priorities"},
        {"🎬 FILM FESTIVAL: Submission deadline extended to next month"},
        {"📻 RADIO: Local station adds new late-night program"},
        {"🎸 LIVE MUSIC: Free concert series kicks off this weekend"},
        {"🎬 STREAMING: New series features local actors"},
        {"🎭 PERFORMANCE: Ballet company announces season schedule"},
        
        // TRANSPORTATION & TRAVEL
        {"🚄 TRAIN UPDATE: New schedule takes effect next Monday"},
        {"✈️ AIRPORT: Construction causing terminal delays"},
        {"🚲 BIKE SHARE: New stations opening at transit hubs"},
        {"🚢 FERRY SERVICE: Summer schedule announced"},
        {"🚗 CARPOOL: Highway carpool lane hours extended"},
        {"🚌 RIDE SHARE: New pickup zones established downtown"},
        {"🚆 COMMUTER: New train cars added to rush hour service"},
        {"🅿️ PARKING: Rate increase effective next month"},
        {"🚕 TRANSPORTATION: New app integrates all transit options"},
        {"🚌 SCHOOL BUS: Route changes affecting elementary zones"},
        
        // HEALTH & WELLNESS
        {"🏥 HEALTH ALERT: Flu shots available at community clinics"},
        {"💊 PHARMACY: Prescription refill program now online"},
        {"🧘 MENTAL HEALTH: Free counseling workshops this month"},
        {"🏃 RUNNERS: 5K charity run registration now open"},
        {"🥗 NUTRITION: Summer eating guide available online"},
        {"🛌 SLEEP: New clinic offers sleep disorder screening"},
        {"🏊 SWIMMING: Pool season opening schedule released"},
        {"🍏 WELLNESS: Free health screenings at workplace"},
        {"💪 YOGA: Community classes offered at reduced rates"},
        {"👶 PARENTING: Free newborn care classes starting next week"},
        
        // FINANCE & TAXES
        {"💰 TAX UPDATE: Filing deadline extended for affected taxpayers"},
        {"🏦 BANKING: New fees effective for certain account types"},
        {"📉 INVESTMENT: Analyst predicts market volatility ahead"},
        {"💳 CREDIT: New consumer protection laws take effect"},
        {"🏘️ HOUSING: Mortgage rates show slight decline"},
        {"💵 PAYROLL: Wage garnishment rules change next quarter"},
        {"📊 BUDGET: City council approves fiscal year budget"},
        {"🎯 SAVINGS: High-yield account rates at record levels"},
        {"🏛️ INSURANCE: New flood insurance requirements announced"},
        {"💼 INVOICE: Payment terms updated for vendors"},
        
        // EDUCATION & LEARNING
        {"🎓 GRADUATION: Class of 2026 ceremony schedules announced"},
        {"📝 EXAMINATION: State testing dates confirmed for students"},
        {"👨‍🏫 TEACHER: Professional development day rescheduled"},
        {"🎓 SCHOLARSHIP: Deadline approaching for local awards"},
        {"📚 SCHOOL SUPPLIES: Free program available for students"},
        {"🧪 SCIENCE: Student competition winners announced"},
        {"🎨 ARTS: Student art exhibition opens this weekend"},
        {"🏫 SCHOOL BOUNDARY: Redistricting map released"},
        {"📖 LITERACY: Summer reading challenge begins"},
        {"👩‍💼 CAREER: College application workshops available"},
        
        // SPORTS & ATHLETICS
        {"🏈 FOOTBALL: Regional playoffs schedule released"},
        {"🏀 BASKETBALL: All-star game rosters announced"},
        {"⚾ BASEBALL: Season opener draws record crowd"},
        {"🏈 SOCCER: Youth league registration now open"},
        {"🎾 TENNIS: Annual tournament accepts entries"},
        {"🏊 SWIMMING: Meet results posted online"},
        {"🚴 CYCLING: Road race route preview available"},
        {"🏋️ WEIGHTLIFTING: Local record broken at competition"},
        {"🤸 GYMNASTICS: Championship team announces roster"},
        {"🥊 COMBAT: Self-defense classes for youth starting"},
        
        // ENVIRONMENT & SUSTAINABILITY
        {"🌱 RECYCLING: New program launches this month"},
        {"🌲 TREE PLANTING: Community day scheduled for spring"},
        {"♻️ COMPOST: Drop-off locations expanded"},
        {"💧 WATER SAVING: Conservation measures take effect"},
        {"🐾 ANIMAL SHELTER: Adoption event this weekend"},
        {"🦅 WILDLIFE: Nature trail renovation underway"},
        {"🌞 SOLAR: Rebate program extended for residents"},
        {"🚴 ECO-TRANSPORT: Bike lane expansion announced"},
        {"🍃 ORGANIC: Farmers market features local growers"},
        {"🏞️ PARK: New playground equipment installed"},
        
        // GENERAL & MISCELLANEOUS
        {"🎉 CELEBRATION: Anniversary event draws thousands"},
        {"🎁 GIVEAWAY: Local business celebrates with prizes"},
        {"📞 SERVICE UPDATE: Call center hours adjusted"},
        {"🗳️ ELECTION: Voter registration deadline approaching"},
        {"📋 FEE: Permit processing time increased"},
        {"🚧 UTILITY: Scheduled maintenance notice for residents"},
        {"🏛️ GOVERNMENT: City council meeting agenda posted"},
        {"📊 SURVEY: Community feedback sought on development"},
        {"📅 CALENDAR: Monthly events guide now available"},
        {"🎊 OPEN HOUSE: Local attraction offers free admission"}
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
