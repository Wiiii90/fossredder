#pragma once
#include <memory>
#include <vector>
#include <string>

class BookingGroup;

class IBookingGroupManager {
public:
    virtual ~IBookingGroupManager() = default;
    virtual void addBookingGroup(const std::shared_ptr<BookingGroup>& group) = 0;
    virtual std::vector<std::shared_ptr<BookingGroup>> getBookingGroups() const = 0;
    virtual void removeBookingGroup(const std::string& id) = 0;
    virtual void updateBookingGroup(const std::shared_ptr<BookingGroup>& group) = 0;
};