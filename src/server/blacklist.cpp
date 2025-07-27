#include "pgw/blacklist.hpp"
#include <spdlog/spdlog.h>

namespace pgw {

Blacklist::Blacklist(const std::vector<std::string>& imsi_list) {
    blocked_.reserve(imsi_list.size());
    for (const auto& imsi : imsi_list) {
        blocked_.insert(imsi);
    }
    spdlog::info("Loaded blacklist: {} entries", blocked_.size());
}

bool Blacklist::is_blocked(const std::string& imsi) const noexcept {
    // В чёрном списке? — логируем на уровне debug, чтобы потом не шуметь в инфо
    bool blocked = (blocked_.find(imsi) != blocked_.end());
    if (blocked) {
        spdlog::debug("IMSI {} is blocked", imsi);
    }
    return blocked;
}

} // namespace pgw
