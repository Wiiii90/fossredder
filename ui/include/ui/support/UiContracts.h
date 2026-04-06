/**
 * @file ui/include/ui/support/UiContracts.h
 * @brief Declares shared UI-facing contract values and enums used across modules.
 */

#pragma once

#include <QObject>
#include <QString>

namespace ui::support::contracts {

Q_NAMESPACE

namespace operations {

/** @brief Operation key emitted for create-new-file actions. */
inline const auto kNewFile = QStringLiteral("newFile");

/** @brief Operation key emitted for open-file actions. */
inline const auto kOpenFile = QStringLiteral("openFile");

/** @brief Operation key emitted for save-file actions. */
inline const auto kSaveFile = QStringLiteral("saveFile");

/** @brief Operation key emitted for save-as actions. */
inline const auto kSaveFileAs = QStringLiteral("saveFileAs");

} // namespace operations

/** @brief Supported export formats exposed to QML. */
enum class ExportFormat : int {
    Csv = 0,
    Xlsx = 1
};

Q_ENUM_NS(ExportFormat)

} // namespace ui::support::contracts
