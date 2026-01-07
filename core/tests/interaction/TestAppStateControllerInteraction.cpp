#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "core/controllers/AppStateController.h"
#include "core/models/Actor.h"
#include "mocks/MockStorageManager.h"

using ::testing::Return;
using ::testing::NiceMock;
using ::testing::_;

TEST(AppStateControllerInteraction, SaveFile_calls_storage_save_once) {
    auto mock = std::make_unique<NiceMock<MockStorageManager>>();
    MockStorageManager* mptr = mock.get();

    EXPECT_CALL(*mptr, save(_)).Times(1);

    AppStateController ctrl(std::move(mock));

    auto a = std::make_shared<Actor>();
    a->id = "id1";
    a->name = "Name";
    ctrl.mutableState().actors.push_back(a);

    ctrl.saveFile();
}
