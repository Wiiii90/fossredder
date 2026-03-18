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
