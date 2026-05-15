/**
 * @file ui/tests/unit/TestNavigationState.cpp
 * @brief Tests for the UI NavigationState component.
 */

#include <gtest/gtest.h>

#include "ui/state/navigation/NavigationState.h"

namespace ui {

TEST(NavigationStateTest, StoresAndExposesTheCurrentSectionAndSettingsCategory)
{
    NavigationState navigation;

    EXPECT_EQ(navigation.section(), NavigationState::Section::Import);
    EXPECT_EQ(navigation.sectionValue(), static_cast<int>(NavigationState::Section::Import));
    EXPECT_EQ(navigation.settingsCategory(), NavigationState::SettingsCategory::General);
    EXPECT_EQ(navigation.settingsCategoryValue(), static_cast<int>(NavigationState::SettingsCategory::General));

    navigation.setSection(NavigationState::Section::Booking);
    navigation.setSectionValue(static_cast<int>(NavigationState::Section::Analysis));
    navigation.setSettingsCategory(NavigationState::SettingsCategory::Export);
    navigation.setSettingsCategoryValue(static_cast<int>(NavigationState::SettingsCategory::Miscellaneous));

    EXPECT_EQ(navigation.section(), NavigationState::Section::Analysis);
    EXPECT_EQ(navigation.sectionValue(), static_cast<int>(NavigationState::Section::Analysis));
    EXPECT_EQ(navigation.settingsCategory(), NavigationState::SettingsCategory::Miscellaneous);
    EXPECT_EQ(navigation.settingsCategoryValue(), static_cast<int>(NavigationState::SettingsCategory::Miscellaneous));
}

} // namespace ui

