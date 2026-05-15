/**
 * @file ui/src/state/NavigationState.cpp
 * @brief Implementation of the UI NavigationState component.
 */

#include "ui/state/navigation/NavigationState.h"

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

void NavigationState::setSectionValue(int value)
{
    setSection(static_cast<Section>(value));
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

void NavigationState::setSettingsCategoryValue(int value)
{
    setSettingsCategory(static_cast<SettingsCategory>(value));
}

}
