#include "event/event.hpp"
#include "eventlog/journal.hpp"
#include <filesystem>
#include <gtest/gtest.h>

TEST(JournalTest, AppendAndVerify) {
    std::filesystem::create_directories("test_output");
    std::string path = "test_output/test_journal.bin";

    fastexchange::EventJournal journal;
    journal.open(path, 42, 12345);

    for (int i = 0; i < 100; ++i) {
        fastexchange::Event e;
        e.type = fastexchange::EventType::ScenarioStarted;
        e.timestamp = static_cast<fastexchange::Timestamp>(i);
        e.payload = fastexchange::ScenarioPayload{"test", 42};
        journal.append(e);
    }
    journal.close();

    EXPECT_TRUE(fastexchange::EventJournal::verify_file(path));

    auto events = fastexchange::EventJournal::read_from_file(path);
    EXPECT_EQ(events.size(), 100);

    std::string json_path = "test_output/test_journal.json";
    fastexchange::EventJournal::export_to_json(path, json_path);
    EXPECT_TRUE(std::filesystem::exists(json_path));
}

TEST(JournalTest, DeterministicHash) {
    std::filesystem::create_directories("test_output");
    auto write_journal = [](const std::string& path) {
        fastexchange::EventJournal journal;
        journal.open(path, 99, 555);
        for (int i = 0; i < 50; ++i) {
            fastexchange::Event e;
            e.type = fastexchange::EventType::OrderRejected;
            e.timestamp = static_cast<fastexchange::Timestamp>(i * 100);
            e.payload = fastexchange::RejectPayload{static_cast<fastexchange::OrderId>(i), "test"};
            journal.append(e);
        }
        journal.close();
    };

    std::string path1 = "test_output/hash1.bin";
    std::string path2 = "test_output/hash2.bin";
    write_journal(path1);
    write_journal(path2);

    EXPECT_EQ(fastexchange::EventJournal::compute_file_hash(path1),
              fastexchange::EventJournal::compute_file_hash(path2));
}
