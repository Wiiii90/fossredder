/**
 * @file core/tests/TestStorageManager.cpp
 * @brief Unit tests for StorageManager registry behavior (UNIT).
 */

#include "gtest/gtest.h"

#include "core/storage/StorageManager.h"
#include "core/storage/IStorageManager.h"
#include "core/models/AppState.h"

#include <chrono>
#include <filesystem>
#include <cstdio>

namespace {

class FakeRegistry final : public core::storage::IRegistry {
public:
    std::optional<std::string> getLatest() const override { return latest_; }
    void setLatest(const std::string& path) override { latest_ = path; }

private:
    std::optional<std::string> latest_;
};

} // namespace

// We'll test basic registry behavior: setLatestPath and loadLatestPath via a StorageManager
// Use a temporary directory inside tmp and ensure cleanup.

TEST(StorageManagerTests, SetAndLoadLatestPathRegistry) {
    // create temp dir
    auto tmp = std::filesystem::temp_directory_path();
    auto dir = tmp / std::filesystem::path(
        "fossredder_test_storage_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::filesystem::create_directories(dir);

    core::storage::StorageManager sm(dir.string());

    // initially no latest path
    auto maybe = sm.loadLatestPath();
    EXPECT_FALSE(maybe.has_value());

    sm.setLatestPath("/tmp/mydb.db");
    auto maybe2 = sm.loadLatestPath();
    ASSERT_TRUE(maybe2.has_value());
    EXPECT_EQ(*maybe2, std::string("/tmp/mydb.db"));

    // cleanup
    try { std::filesystem::remove_all(dir); } catch (...) {}
}
