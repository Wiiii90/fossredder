#pragma once

/**
 * @file persistence/include/persistence/ISqlConnectionProvider.h
 * @brief SQLite connection handle provider used by persistence adapters.
 *
 * This header owns the dependency on <sqlite3.h> and is intended for use by
 * persistence-layer implementations only. Core code should not include this
 * header to avoid leaking sqlite types into the domain.
 */

#include <sqlite3.h>

class ISqlConnectionProvider {
public:
    virtual ~ISqlConnectionProvider() = default;
    virtual sqlite3* sqliteHandle() const noexcept = 0;
};
