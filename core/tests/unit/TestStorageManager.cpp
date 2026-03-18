/**
 * @file core/tests/TestStorageManager.cpp
 * @brief Unit tests for StorageManager registry behavior (UNIT).
 */

#include "gtest/gtest.h"

#include "core/storage/StorageManager.h"
#include "core/storage/IStorageManager.h"
#include "core/models/AppState.h"

namespace {

class FakeRegistry final : public core::storage::IRegistry {
public:
    std::optional<std::string> getLatest() const override { return latest_; }
    void setLatest(const std::string& path) override { latest_ = path; }

private:
    std::optional<std::string> latest_;
};

} // namespace

TEST(StorageManagerTests, LoadFromTracksLatestPathInRegistry) {
    auto registry = std::make_shared<FakeRegistry>();

    core::storage::StorageManager sm(registry);

    auto maybe = sm.loadLatestPath();
    EXPECT_FALSE(maybe.has_value());

    EXPECT_THROW(sm.loadFrom("/tmp/mydb.db"), std::runtime_error);

    auto maybe2 = sm.loadLatestPath();
    ASSERT_TRUE(maybe2.has_value());
    EXPECT_EQ(*maybe2, std::string("/tmp/mydb.db"));
}
