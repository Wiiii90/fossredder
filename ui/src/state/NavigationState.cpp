/**
 * @file ui/src/state/NavigationState.cpp
 * @brief Implementation of the UI NavigationState component.
 */

#include "ui/state/NavigationState.h"

#include <QMetaObject>

namespace ui {

NavigationState::NavigationState(QObject* parent) : QObject(parent) {}

void NavigationState::setSection(Section s) {
    if (section_ == s) return;
    section_ = s;
    if (sectionChangeQueued_) return;
    sectionChangeQueued_ = true;
    QMetaObject::invokeMethod(this, [this]() {
        sectionChangeQueued_ = false;
        emit sectionChanged();
    }, Qt::QueuedConnection);
}

void NavigationState::setBookingView(BookingView v)
{
    if (bookingView_ == v) return;
    bookingView_ = v;
    if (bookingViewChangeQueued_) return;
    bookingViewChangeQueued_ = true;
    QMetaObject::invokeMethod(this, [this]() {
        bookingViewChangeQueued_ = false;
        emit bookingViewChanged();
    }, Qt::QueuedConnection);
}

void NavigationState::setSettingsCategory(SettingsCategory c)
{
    if (settingsCategory_ == c) return;
    settingsCategory_ = c;
    if (settingsCategoryChangeQueued_) return;
    settingsCategoryChangeQueued_ = true;
    QMetaObject::invokeMethod(this, [this]() {
        settingsCategoryChangeQueued_ = false;
        emit settingsCategoryChanged();
    }, Qt::QueuedConnection);
}

}

