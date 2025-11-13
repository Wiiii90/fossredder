#include "managers/BookingGroupManager.h"
#include <algorithm>
#include <iostream>

void BookingGroupManager::addBookingGroup(const std::shared_ptr<BookingGroup>& group) {
    bookingGroups_.push_back(group);
}

std::vector<std::shared_ptr<BookingGroup>> BookingGroupManager::getBookingGroups() const {
    return bookingGroups_;
}

void BookingGroupManager::removeBookingGroup(const std::string& id) {
    // Stub: special command to clear all
    if (id == "ALL") { bookingGroups_.clear(); return; }
    std::cout << "[BookingGroupManager] removeBookingGroup called with id='" << id << "' - noop in stub mode" << std::endl;
}

void BookingGroupManager::updateBookingGroup(const std::shared_ptr<BookingGroup>& group) {
    if (bookingGroups_.empty()) { bookingGroups_.push_back(group); return; }
    bookingGroups_[0] = group;
}