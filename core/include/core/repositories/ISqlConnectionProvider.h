#pragma once

#include <sqlite3.h>

class ISqlConnectionProvider {
public:
    virtual ~ISqlConnectionProvider() = default;
    virtual sqlite3* sqliteHandle() const noexcept = 0;
};
