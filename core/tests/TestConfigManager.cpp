/**
 * @file core/tests/TestConfigManager.cpp
 * @brief Unit tests for ConfigManager (UNIT).
 */

#include "gtest/gtest.h"

#include "core/managers/ConfigManager.h"
#include "core/models/Config.h"

#include <memory>
#include <string>
#include <algorithm>

TEST(ConfigManagerTests, SaveLoadListDeleteDefault) {
    ConfigManager mgr;

    auto cfg1 = std::make_shared<Config>();
    cfg1->language = Language::EN;
    ASSERT_TRUE(mgr.saveConfig("one", cfg1));

    auto cfg2 = std::make_shared<Config>();
    cfg2->language = Language::DE;
    ASSERT_TRUE(mgr.saveConfig("two", cfg2));

    auto list = mgr.listConfigs();
    // list may be in any order, check contents
    EXPECT_NE(std::find(list.begin(), list.end(), "one"), list.end());
    EXPECT_NE(std::find(list.begin(), list.end(), "two"), list.end());

    auto loaded = mgr.loadConfig("one");
    ASSERT_TRUE(loaded.has_value());
    EXPECT_EQ((*loaded)->language, Language::EN);

    EXPECT_TRUE(mgr.deleteConfig("one"));
    EXPECT_FALSE(mgr.loadConfig("one").has_value());

    // default config handling
    mgr.setDefaultConfig("two");
    auto def = mgr.getDefaultConfig();
    ASSERT_TRUE(def.has_value());
    EXPECT_EQ((*def)->language, Language::DE);
    EXPECT_EQ(mgr.getDefaultConfigName(), std::string("two"));

    // set default to non-existing clears name but getDefaultConfig returns null
    mgr.setDefaultConfig("nonexistent");
    EXPECT_EQ(mgr.getDefaultConfigName(), std::string("nonexistent"));
    // load will be null because not present
    EXPECT_FALSE(mgr.getDefaultConfig().has_value());
}
