#pragma once

#include <memory>
#include <vector>
#include <string>

class BookingGroup;

class IBookingGroupRepository {
public:
    virtual ~IBookingGroupRepository() = default;
    virtual void addBookingGroup(const std::string& statementId, const std::shared_ptr<BookingGroup>& group) = 0;
    virtual std::vector<std::shared_ptr<BookingGroup>> getBookingGroupsForStatement(const std::string& statementId) const = 0;
    virtual void removeBookingGroup(const std::string& id) = 0;
    virtual void updateBookingGroup(const std::string& id, const std::shared_ptr<BookingGroup>& group) = 0;
};
