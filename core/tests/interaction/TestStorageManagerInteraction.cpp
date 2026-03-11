#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "core/storage/StorageManager.h"
#include "mocks/MockStorageManager.h"

#include "mocks/MockActorRepository.h"
#include "mocks/MockPropertyRepository.h"
#include "mocks/MockContractRepository.h"
#include "mocks/MockStatementRepository.h"
#include "mocks/MockTransactionRepository.h"

using ::testing::Return;
using ::testing::NiceMock;
using ::testing::_;

TEST(StorageManagerInteraction, SaveAs_calls_atomic_save_when_configured) {
    StorageManager sm(std::string("/tmp"));

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
