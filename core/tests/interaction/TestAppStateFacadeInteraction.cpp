#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "core/application/AppStateFacade.h"
#include "mocks/MockStorageManager.h"

using ::testing::_;
using ::testing::NiceMock;
using ::testing::ReturnRef;

TEST(AppStateFacadeInteraction, SaveFileCallsStorageSaveOnce) {
    auto mock = std::make_unique<NiceMock<MockStorageManager>>();
    MockStorageManager* storage = mock.get();
    std::string currentPath = "interaction.db";

    EXPECT_CALL(*storage, currentPath()).WillRepeatedly(ReturnRef(currentPath));
    EXPECT_CALL(*storage, createNew(_)).Times(1);
    EXPECT_CALL(*storage, save(_)).Times(1);

    core::application::AppStateFacade facade(std::move(mock));
    facade.newFile("interaction.db");

    facade.saveFile();
}

TEST(AppStateFacadeInteraction, AddActorCommitsThroughStorageSaveWhenWorkspaceIsOpen)
{
    auto mock = std::make_unique<NiceMock<MockStorageManager>>();
    MockStorageManager* storage = mock.get();
    std::string currentPath = "interaction.db";

    EXPECT_CALL(*storage, currentPath()).WillRepeatedly(ReturnRef(currentPath));
    EXPECT_CALL(*storage, createNew(_)).Times(1);
    EXPECT_CALL(*storage, save(_)).Times(1);

    core::application::AppStateFacade facade(std::move(mock));
    facade.newFile("interaction.db");

    const auto id = facade.addActor("Alice", {});
    EXPECT_FALSE(id.empty());
}

TEST(AppStateFacadeInteraction, OpenLatestLoadsFromLatestPath)
{
    auto mock = std::make_unique<NiceMock<MockStorageManager>>();
    MockStorageManager* storage = mock.get();

    EXPECT_CALL(*storage, loadLatestPath()).WillOnce(::testing::Return(std::optional<std::string>{"latest.db"}));
    EXPECT_CALL(*storage, loadFrom("latest.db")).Times(1).WillOnce(::testing::Return(core::domain::AppState{}));

    core::application::AppStateFacade facade(std::move(mock));
    facade.openLatest();
}

TEST(AppStateFacadeInteraction, OpenFileDelegatesToStorageLoad)
{
    auto mock = std::make_unique<NiceMock<MockStorageManager>>();
    MockStorageManager* storage = mock.get();

    EXPECT_CALL(*storage, loadFrom("workspace.db")).Times(1).WillOnce(::testing::Return(core::domain::AppState{}));

    core::application::AppStateFacade facade(std::move(mock));
    facade.openFile("workspace.db");
}
