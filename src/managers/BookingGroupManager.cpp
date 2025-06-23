#include "managers/BookingGroupManager.h"
#include <algorithm>

void BookingGroupManager::addBookingGroup(const std::shared_ptr<BookingGroup>& group) {
    bookingGroups_.push_back(group);
}

std::vector<std::shared_ptr<BookingGroup>> BookingGroupManager::getBookingGroups() const {
    return bookingGroups_;
}

void BookingGroupManager::removeBookingGroup(const std::string& id) {
    bookingGroups_.erase(
        std::remove_if(bookingGroups_.begin(), bookingGroups_.end(),
            [&](const std::shared_ptr<BookingGroup>& g) { return g && g->getBookingDate() == id; }),
        bookingGroups_.end());
}

void BookingGroupManager::updateBookingGroup(const std::shared_ptr<BookingGroup>& group) {
    for (auto& g : bookingGroups_) {
        if (g && g->getBookingDate() == group->getBookingDate()) {
            g = group;
            break;
        }
    }
}