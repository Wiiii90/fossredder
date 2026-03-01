#include "ui/state/NavigationState.h"

namespace ui {

NavigationState::NavigationState(QObject* parent) : QObject(parent) {}

void NavigationState::setSection(Section s) {
    if (section_ == s) return;
    section_ = s;
    emit sectionChanged();
}

void NavigationState::setBookingView(BookingView v)
{
    if (bookingView_ == v) return;
    bookingView_ = v;
    emit bookingViewChanged();
}

void NavigationState::setSettingsCategory(SettingsCategory c)
{
    if (settingsCategory_ == c) return;
    settingsCategory_ = c;
    emit settingsCategoryChanged();
}

}

