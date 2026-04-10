#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "core/storage/StorageManager.h"

#include <memory>

using ::testing::Return;
using ::testing::NiceMock;
using ::testing::_;
using core::domain::AppState;
using core::domain::DeletionImpact;
using core::storage::StorageManager;

namespace {

class TrackingRegistry final : public core::storage::IRegistry {
public:
    std::optional<std::string> getLatest() const override { return latest; }
    void setLatest(const std::string& path) override { latest = path; }

    std::optional<std::string> latest;
};

} // namespace

TEST(StorageManagerInteraction, SaveAs_calls_atomic_save_when_configured) {
    StorageManager sm;

    bool called = false;
    sm.setAtomicStoreSave([&](const std::string& path, const AppState& state){
        called = true;
        DeletionImpact imp;
        return imp;
    });

    AppState s;
    sm.saveAs("path.db", s);
    EXPECT_TRUE(called);
}

TEST(StorageManagerInteraction, LoadFromUsesAtomicLoadAndRemembersLatestPath)
{
    auto registry = std::make_shared<TrackingRegistry>();
    StorageManager sm(registry);

    bool called = false;
    sm.setAtomicStoreLoad([&](const std::string& path) {
        called = true;
        EXPECT_EQ(path, "workspace.db");
        AppState state;
        return state;
    });

    const auto loaded = sm.loadFrom("workspace.db");
    EXPECT_TRUE(called);
    EXPECT_TRUE(loaded.empty());
    ASSERT_TRUE(registry->latest.has_value());
    EXPECT_EQ(*registry->latest, "workspace.db");
}

TEST(StorageManagerInteraction, CreateNewUsesRepoFactoryWhenNoAtomicLoadConfigured)
{
    StorageManager sm;

    bool repoFactoryCalled = false;
    sm.setRepoFactory([&](const std::string& path) {
        repoFactoryCalled = true;
        EXPECT_EQ(path, "new-workspace.db");
        return core::storage::RepositoryBundle{};
    });

    sm.createNew("new-workspace.db");
    EXPECT_TRUE(repoFactoryCalled);
}

TEST(StorageManagerInteraction, SaveAsPublishesDeletionImpactFromAtomicSave)
{
    StorageManager sm;

    bool callbackCalled = false;
    sm.setDeletionImpactCallback([&](const DeletionImpact& impact) {
        callbackCalled = true;
        EXPECT_EQ(impact.deletedActorIds, (std::vector<std::string>{"actor-1"}));
    });

    sm.setAtomicStoreSave([](const std::string& path, const AppState&) {
        EXPECT_EQ(path, "impact.db");
        DeletionImpact impact;
        impact.deletedActorIds = {"actor-1"};
        return impact;
    });

    sm.saveAs("impact.db", AppState{});
    EXPECT_TRUE(callbackCalled);
}
