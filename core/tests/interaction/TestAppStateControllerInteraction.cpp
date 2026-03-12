#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "core/controllers/AppStateController.h"
#include "mocks/MockStorageManager.h"

using ::testing::Return;
using ::testing::NiceMock;
using ::testing::_;
using ::testing::ReturnRef;

TEST(AppStateControllerInteraction, SaveFile_calls_storage_save_once) {
    auto mock = std::make_unique<NiceMock<MockStorageManager>>();
    MockStorageManager* mptr = mock.get();
    std::string currentPath = "interaction.db";

    EXPECT_CALL(*mptr, currentPath()).WillRepeatedly(ReturnRef(currentPath));
    EXPECT_CALL(*mptr, createNew(_)).Times(1);
    EXPECT_CALL(*mptr, save(_)).Times(1);

    core::controllers::AppStateController ctrl(std::move(mock));
    ctrl.newFile("interaction.db");

    ctrl.saveFile();
}
