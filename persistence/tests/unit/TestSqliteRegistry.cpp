/**
 * @file persistence/tests/unit/TestSqliteRegistry.cpp
 * @brief Unit-style tests for the SQLite-backed latest-path registry.
 */

#include "gtest/gtest.h"

#include "core/storage/IRegistry.h"
#include "persistence/Factory.h"

#include <chrono>
#include <filesystem>
#include <string>

namespace {

class TempDatabaseFile {
public:
    TempDatabaseFile()
    {
        const auto uniqueId = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
        path_ = std::filesystem::temp_directory_path() /
                ("fossredder-registry-tests-" + uniqueId + ".db");
    }

    ~TempDatabaseFile()
    {
        std::error_code error;
        std::filesystem::remove(path_, error);
    }

    const std::filesystem::path& path() const noexcept { return path_; }

private:
    std::filesystem::path path_;
};

} // namespace

TEST(SqliteRegistryTests, SetLatestPersistsAcrossInstances)
{
    TempDatabaseFile tempDb;

    {
        auto registry = createSqliteRegistry(tempDb.path().string());
        ASSERT_NE(registry, nullptr);
        EXPECT_FALSE(registry->getLatest().has_value());
        registry->setLatest("/tmp/latest.db");
    }

    auto registry = createSqliteRegistry(tempDb.path().string());
    ASSERT_NE(registry, nullptr);

    const auto latest = registry->getLatest();
    ASSERT_TRUE(latest.has_value());
    EXPECT_EQ(*latest, std::string("/tmp/latest.db"));
}
