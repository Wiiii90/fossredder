/**
 * @file persistence/tests/unit/TestSqliteRepository.cpp
 * @brief Smoke tests for SQLite repository helpers.
 */

#include <gtest/gtest.h>

#include "persistence/SqliteDb.h"

namespace fossredder::persistence {

TEST(SqliteRepositoryTest, HeaderIsUsable) {
    SUCCEED();
}

} // namespace fossredder::persistence
