#pragma once

#include "core/repositories/IBookingGroupRepository.h"
#include <memory>
#include <string>

class SqliteBookingGroupRepository : public IBookingGroupRepository {
public:
    explicit SqliteBookingGroupRepository(const std::string& dbPath);
    ~SqliteBookingGroupRepository() override;

    void addBookingGroup(const std::string& statementId, const std::shared_ptr<BookingGroup>& group) override;
    std::vector<std::shared_ptr<BookingGroup>> getBookingGroupsForStatement(const std::string& statementId) const override;
    void removeBookingGroup(const std::string& id) override;
    void updateBookingGroup(const std::string& id, const std::shared_ptr<BookingGroup>& group) override;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};
