#pragma once

#include <QString>

namespace ui::importing {

struct ImportRunInfo {
    QString runRoot;
    QString runIdPrefix;
};

ImportRunInfo createImportRunInfo();
void cleanupOldImportRuns(int keepCount);

}