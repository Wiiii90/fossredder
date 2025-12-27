#include "ui/state/UiNavigation.h"

UiNavigation::UiNavigation(QObject* parent) : QObject(parent) {}

void UiNavigation::setSection(Section s) {
    if (section_ == s) return;
    section_ = s;
    emit sectionChanged();
}
