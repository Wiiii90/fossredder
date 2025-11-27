#pragma once

#include <QStandardItemModel>

namespace ui {

class FakeDataGenerator {
public:
    static void populate_timeline_model(QStandardItemModel* model,
                                        int year_start = 2021,
                                        int year_count = 3,
                                        int statements_per_year = 3);
};

}
