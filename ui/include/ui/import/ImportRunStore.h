/**
 * @file ui/include/ui/import/ImportRunStore.h
 * @brief Declares helpers that manage persisted import run folders in the UI layer.
 */

#pragma once

#include <QString>

namespace ui::importing {

/** @brief Describes a newly allocated import run directory and its stable ID prefix. */
struct ImportRunInfo {
    QString runRoot;
    QString runIdPrefix;
};

/** @brief Creates a unique persisted folder for a new import run. */
ImportRunInfo createImportRunInfo();
/** @brief Deletes older persisted import runs beyond the configured retention count. */
void cleanupOldImportRuns(int keepCount);

}