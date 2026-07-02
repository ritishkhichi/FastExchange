#pragma once

#include "event/event.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace fastexchange {

constexpr char JOURNAL_MAGIC[8] = {'F', 'X', 'E', 'V', 'E', 'N', 'T', '\0'};
constexpr uint32_t JOURNAL_VERSION = 1;

#pragma pack(push, 1)
struct FileHeader {
    char magic[8];
    uint32_t version;
    uint64_t seed;
    uint64_t config_hash;
    uint8_t reserved[32];
};

struct EventRecordHeader {
    uint16_t type;
    uint64_t timestamp;
    uint32_t payload_len;
};

struct FileFooter {
    uint64_t event_count;
    uint8_t sha256[32];
};
#pragma pack(pop)

class EventCodec {
public:
    static std::vector<uint8_t> encode_payload(const Event& event);
    static Event decode_payload(EventType type, Timestamp ts, const std::vector<uint8_t>& data);
    static std::string export_event_json(const Event& event);
};

}  // namespace fastexchange
