/**
 * @file core/tests/TestStorageManager.cpp
 * @brief Unit tests for StorageManager registry behavior (UNIT).
 */

#include "gtest/gtest.h"

#include "core/managers/StorageManager.h"
#include "core/managers/IStorageManager.h"
#include "core/models/AppState.h"

#include <filesystem>
#include <cstdio>

// We'll test basic registry behavior: setLatestPath and loadLatestPath via a StorageManager
// Use a temporary directory inside tmp and ensure cleanup.

TEST(StorageManagerTests, SetAndLoadLatestPathRegistry) {
    // create temp dir
    auto tmp = std::filesystem::temp_directory_path();
    auto dir = tmp / std::filesystem::path("fossredder_test_storage");
    std::filesystem::create_directories(dir);

    StorageManager sm(dir.string());

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
