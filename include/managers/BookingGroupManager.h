#pragma once
#include "managers/IBookingGroupManager.h"

class BookingGroupManager : public IBookingGroupManager {
public:
    void addBookingGroup(const std::shared_ptr<BookingGroup>& group) override;
    std::vector<std::shared_ptr<BookingGroup>> getBookingGroups() const override;
    void removeBookingGroup(const std::string& id) override;
    void updateBookingGroup(const std::shared_ptr<BookingGroup>& group) override;
private:
    std::vector<std::shared_ptr<BookingGroup>> groups_;
};