#include "eventlog/journal.hpp"
#include "event/event_codec.hpp"
#include "eventlog/sha256.hpp"
#include <cstring>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <stdexcept>

namespace fastexchange {

namespace {

void append_u16(std::vector<uint8_t>& buf, uint16_t v) {
    buf.push_back(static_cast<uint8_t>(v & 0xFF));
    buf.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
}

void append_u32(std::vector<uint8_t>& buf, uint32_t v) {
    for (int i = 0; i < 4; ++i) {
        buf.push_back(static_cast<uint8_t>((v >> (i * 8)) & 0xFF));
    }
}

void append_u64(std::vector<uint8_t>& buf, uint64_t v) {
    for (int i = 0; i < 8; ++i) {
        buf.push_back(static_cast<uint8_t>((v >> (i * 8)) & 0xFF));
    }
}

std::vector<uint8_t> read_file_bytes(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) throw std::runtime_error("cannot open file: " + path);
    return std::vector<uint8_t>((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
}

}  // namespace

void EventJournal::open(const std::string& path, uint64_t seed, uint64_t config_hash) {
    if (open_) close();
    std::filesystem::create_directories(std::filesystem::path(path).parent_path());
    path_ = path;
    seed_ = seed;
    config_hash_ = config_hash;
    event_count_ = 0;
    hash_buffer_.clear();

    file_.open(path, std::ios::binary | std::ios::out | std::ios::trunc);
    if (!file_) throw std::runtime_error("cannot create journal: " + path);

    write_header();
    open_ = true;
}

void EventJournal::write_header() {
    FileHeader hdr{};
    std::memcpy(hdr.magic, JOURNAL_MAGIC, 8);
    hdr.version = JOURNAL_VERSION;
    hdr.seed = seed_;
    hdr.config_hash = config_hash_;
    file_.write(reinterpret_cast<const char*>(&hdr), sizeof(hdr));

    hash_buffer_.resize(sizeof(FileHeader));
    std::memcpy(hash_buffer_.data(), &hdr, sizeof(FileHeader));
}

void EventJournal::write_record(const Event& event) {
    auto payload = EventCodec::encode_payload(event);
    EventRecordHeader rh{};
    rh.type = static_cast<uint16_t>(event.type);
    rh.timestamp = event.timestamp;
    rh.payload_len = static_cast<uint32_t>(payload.size());

    file_.write(reinterpret_cast<const char*>(&rh), sizeof(rh));
    if (!payload.empty()) {
        file_.write(reinterpret_cast<const char*>(payload.data()),
                    static_cast<std::streamsize>(payload.size()));
    }

    append_u16(hash_buffer_, rh.type);
    append_u64(hash_buffer_, rh.timestamp);
    append_u32(hash_buffer_, rh.payload_len);
    hash_buffer_.insert(hash_buffer_.end(), payload.begin(), payload.end());
    ++event_count_;
}

void EventJournal::append(const Event& event) {
    if (!open_) throw std::runtime_error("journal not open");
    write_record(event);
}

void EventJournal::close() {
    if (!open_) return;

    auto hash = sha256(hash_buffer_);
    FileFooter footer{};
    footer.event_count = event_count_;
    std::memcpy(footer.sha256, hash.data(), 32);

    file_.write(reinterpret_cast<const char*>(&footer), sizeof(footer));
    file_.close();
    open_ = false;
}

std::vector<uint8_t> EventJournal::build_hash_input() const {
    auto bytes = read_file_bytes(path_);
    if (bytes.size() < sizeof(FileHeader) + sizeof(FileFooter)) {
        throw std::runtime_error("journal too small");
    }
    size_t body_end = bytes.size() - sizeof(FileFooter);
    return std::vector<uint8_t>(bytes.begin(), bytes.begin() + static_cast<ptrdiff_t>(body_end));
}

bool EventJournal::verify_sha256() const {
    return verify_file(path_);
}

bool EventJournal::verify_file(const std::string& path) {
    auto bytes = read_file_bytes(path);
    if (bytes.size() < sizeof(FileHeader) + sizeof(FileFooter)) return false;

    FileFooter footer;
    std::memcpy(&footer, bytes.data() + bytes.size() - sizeof(FileFooter), sizeof(FileFooter));

    std::vector<uint8_t> body(bytes.begin(),
                               bytes.begin() + static_cast<ptrdiff_t>(bytes.size() - sizeof(FileFooter)));
    auto computed = sha256(body);

    return std::memcmp(computed.data(), footer.sha256, 32) == 0;
}

std::string EventJournal::compute_file_hash(const std::string& path) {
    auto body = read_file_bytes(path);
    if (body.size() < sizeof(FileHeader) + sizeof(FileFooter)) {
        throw std::runtime_error("invalid journal file");
    }
    body.resize(body.size() - sizeof(FileFooter));
    return sha256_hex(body);
}

std::vector<Event> EventJournal::read_from_file(const std::string& path) {
    auto bytes = read_file_bytes(path);
    if (bytes.size() < sizeof(FileHeader) + sizeof(FileFooter)) {
        throw std::runtime_error("invalid journal");
    }

    size_t offset = sizeof(FileHeader);
    size_t end = bytes.size() - sizeof(FileFooter);
    std::vector<Event> events;

    while (offset + sizeof(EventRecordHeader) <= end) {
        EventRecordHeader rh;
        std::memcpy(&rh, bytes.data() + offset, sizeof(rh));
        offset += sizeof(rh);

        if (offset + rh.payload_len > end) break;

        std::vector<uint8_t> payload(bytes.begin() + static_cast<ptrdiff_t>(offset),
                                     bytes.begin() + static_cast<ptrdiff_t>(offset + rh.payload_len));
        offset += rh.payload_len;

        events.push_back(
            EventCodec::decode_payload(static_cast<EventType>(rh.type), rh.timestamp, payload));
    }
    return events;
}

std::vector<Event> EventJournal::read_all() const {
    return read_from_file(path_);
}

std::string EventJournal::export_to_json(const std::string& path, const std::string& out_path) {
    auto events = read_from_file(path);
    auto bytes = read_file_bytes(path);

    FileHeader hdr;
    std::memcpy(&hdr, bytes.data(), sizeof(hdr));

    FileFooter footer;
    std::memcpy(&footer, bytes.data() + bytes.size() - sizeof(FileFooter), sizeof(FileFooter));

    nlohmann::json j;
    j["header"]["version"] = hdr.version;
    j["header"]["seed"] = hdr.seed;
    j["header"]["config_hash"] = hdr.config_hash;
    j["footer"]["event_count"] = footer.event_count;
    j["footer"]["sha256"] = compute_file_hash(path);

    nlohmann::json arr = nlohmann::json::array();
    for (const auto& e : events) {
        arr.push_back(nlohmann::json::parse(EventCodec::export_event_json(e)));
    }
    j["events"] = arr;

    std::ofstream out(out_path);
    out << j.dump(2);
    return out_path;
}

}  // namespace fastexchange
