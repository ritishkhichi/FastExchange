#pragma once

#include <string>

namespace fastexchange {

class ReplayEngine {
public:
    struct ReplayResult {
        bool hash_match{false};
        std::string original_hash;
        std::string replay_hash;
        uint64_t events_replayed{0};
    };

    static ReplayResult verify(const std::string& journal_path,
                               const std::string& default_config = "config/default.yaml");
    static void replay(const std::string& journal_path, double speed_multiplier = 1.0);
};

}  // namespace fastexchange
