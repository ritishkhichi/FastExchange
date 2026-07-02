#pragma once

#include "event/event.hpp"
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

namespace fastexchange {

class EventJournal {
public:
    EventJournal() = default;

    void open(const std::string& path, uint64_t seed, uint64_t config_hash);
    void append(const Event& event);
    void close();

    std::vector<Event> read_all() const;
    static std::vector<Event> read_from_file(const std::string& path);

    bool verify_sha256() const;
    static bool verify_file(const std::string& path);
    static std::string compute_file_hash(const std::string& path);
    static std::string export_to_json(const std::string& path, const std::string& out_path);

    uint64_t event_count() const { return event_count_; }
    uint64_t seed() const { return seed_; }
    const std::string& path() const { return path_; }

private:
    std::string path_;
    std::fstream file_;
    uint64_t seed_{0};
    uint64_t config_hash_{0};
    uint64_t event_count_{0};
    std::vector<uint8_t> hash_buffer_;
    bool open_{false};

    void write_header();
    void write_record(const Event& event);
    std::vector<uint8_t> build_hash_input() const;
};

}  // namespace fastexchange
