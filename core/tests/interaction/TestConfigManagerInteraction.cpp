#include "gtest/gtest.h"

#include "core/managers/ConfigManager.h"
#include "core/models/Config.h"

#include <memory>
#include <string>

using namespace std;

TEST(ConfigManagerInteraction, SaveLoadListDeleteDefault) {
    ConfigManager mgr;

    auto cfg1 = make_shared<Config>();
    cfg1->language = Language::EN;
    EXPECT_TRUE(mgr.saveConfig("one", cfg1));

    auto cfg2 = make_shared<Config>();
    cfg2->language = Language::DE;
    EXPECT_TRUE(mgr.saveConfig("two", cfg2));

    auto list = mgr.listConfigs();
    EXPECT_NE(find(list.begin(), list.end(), string("one")), list.end());
    EXPECT_NE(find(list.begin(), list.end(), string("two")), list.end());

    auto loaded = mgr.loadConfig("one");
    ASSERT_TRUE(loaded.has_value());
    EXPECT_EQ((*loaded)->language, Language::EN);

    EXPECT_TRUE(mgr.deleteConfig("one"));
    EXPECT_FALSE(mgr.loadConfig("one").has_value());

    mgr.setDefaultConfig("two");
    auto def = mgr.getDefaultConfig();
    ASSERT_TRUE(def.has_value());
    EXPECT_EQ((*def)->language, Language::DE);
    EXPECT_EQ(mgr.getDefaultConfigName(), string("two"));

    mgr.setDefaultConfig("nonexistent");
    EXPECT_EQ(mgr.getDefaultConfigName(), string("nonexistent"));
    EXPECT_FALSE(mgr.getDefaultConfig().has_value());
}
